#include "ScreenTransitionSystem.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <vector>
#include <cstdio>
#include <cmath>

// 
//  GLSL Sources
// 

// Fullscreen quad (NDC, no transform needed)
static const char* QUAD_VERT = R"(
#version 450 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
out vec2 vUV;
void main() {
    vUV = aUV;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

// Iris wipe: draws a black fullscreen quad with a smooth circular cut-out.
// u_Radius: 0.0 = fully closed (all black), 1.41 = fully open (nothing drawn)
static const char* IRIS_FRAG = R"(
#version 450 core
in vec2 vUV;
out vec4 FragColor;

uniform float u_Radius;
uniform vec2  u_Resolution;
uniform vec2  u_Center;

void main() {
    vec2 aspect = vec2(u_Resolution.x / u_Resolution.y, 1.0);
    vec2 uv     = vUV - u_Center;
    float dist  = length(uv * aspect);

    // alpha=1 (black) OUTSIDE the circle, alpha=0 (transparent) INSIDE
    float edge  = 0.015;
    float alpha = smoothstep(u_Radius - edge, u_Radius + edge, dist);

    FragColor = vec4(0.0, 0.0, 0.0, alpha);
}
)";

// Simple textured quad for the GIF frames
static const char* GIF_FRAG = R"(
#version 450 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D u_Tex;
void main() {
    FragColor = texture(u_Tex, vUV);
}
)";

// 
//  Helper: compile + link a shader program
// 
static unsigned int CompileProgram(const char* vert, const char* frag) {
    auto compile = [](GLenum type, const char* src) -> unsigned int {
        unsigned int s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        return s;
        };
    unsigned int vs = compile(GL_VERTEX_SHADER, vert);
    unsigned int fs = compile(GL_FRAGMENT_SHADER, frag);
    unsigned int p = glCreateProgram();
    glAttachShader(p, vs); glAttachShader(p, fs);
    glLinkProgram(p);
    glDeleteShader(vs); glDeleteShader(fs);
    return p;
}

// 
//  ISystem lifecycle
// 

// Initialize() is called by CoreEngine::InitSystems() after the GL context
// is ready, so it's safe to compile shaders and create GPU objects here.
void ScreenTransitionSystem::Initialize() {
    InitGL();
}

ScreenTransitionSystem::~ScreenTransitionSystem() {
    for (auto& f : m_GifFrames)
        glDeleteTextures(1, &f.textureID);
    glDeleteVertexArrays(1, &m_QuadVAO);
    glDeleteBuffers(1, &m_QuadVBO);
    glDeleteProgram(m_IrisShader);
    glDeleteProgram(m_GifShader);
}

// 
//  InitGL  build the fullscreen quad + shaders once
// 
void ScreenTransitionSystem::InitGL() {
    float verts[] = {
        -1.f, -1.f,   0.f, 0.f,
         1.f, -1.f,   1.f, 0.f,
         1.f,  1.f,   1.f, 1.f,
        -1.f, -1.f,   0.f, 0.f,
         1.f,  1.f,   1.f, 1.f,
        -1.f,  1.f,   0.f, 1.f,
    };

    glCreateVertexArrays(1, &m_QuadVAO);
    glCreateBuffers(1, &m_QuadVBO);

    glNamedBufferStorage(m_QuadVBO, sizeof(verts), verts, 0);

    glVertexArrayVertexBuffer(m_QuadVAO, 0, m_QuadVBO, 0, 4 * sizeof(float));

    // Attribute 0: position (vec2)
    glEnableVertexArrayAttrib(m_QuadVAO, 0);
    glVertexArrayAttribFormat(m_QuadVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(m_QuadVAO, 0, 0);

    // Attribute 1: UV (vec2)
    glEnableVertexArrayAttrib(m_QuadVAO, 1);
    glVertexArrayAttribFormat(m_QuadVAO, 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float));
    glVertexArrayAttribBinding(m_QuadVAO, 1, 0);

    // Iris shader
    m_IrisShader = CompileProgram(QUAD_VERT, IRIS_FRAG);
    m_uRadius = glGetUniformLocation(m_IrisShader, "u_Radius");
    m_uResolution = glGetUniformLocation(m_IrisShader, "u_Resolution");
    m_uCenter = glGetUniformLocation(m_IrisShader, "u_Center");

    // GIF shader
    m_GifShader = CompileProgram(QUAD_VERT, GIF_FRAG);
    m_uGifTex = glGetUniformLocation(m_GifShader, "u_Tex");
}

// 
//  LoadGif  decode every frame of a .gif via stb_image
// 
void ScreenTransitionSystem::LoadGif(const std::string& path) {
    for (auto& f : m_GifFrames)
        glDeleteTextures(1, &f.textureID);
    m_GifFrames.clear();
    m_CurrentFrame = 0;
    m_FrameTimer = 0.0f;

    FILE* fp = fopen(path.c_str(), "rb");
    if (!fp) {
        std::cout << "[ScreenTransition] Failed to open: " << path << std::endl;
        return;
    }

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    rewind(fp);

    std::cout << "[ScreenTransition] Opened GIF: " << path << " size: " << fileSize << " bytes" << std::endl;

    std::vector<unsigned char> buffer(fileSize);
    if (fread(buffer.data(), 1, fileSize, fp) != static_cast<size_t>(fileSize)) {
        std::cout << "[ScreenTransition] Failed to read GIF into buffer" << std::endl;
        fclose(fp);
        return;
    }
    fclose(fp);

    stbi_set_flip_vertically_on_load(false);

    int width, height, frameCount;
    int* delays = nullptr;
    unsigned char* data = stbi_load_gif_from_memory(
        buffer.data(), static_cast<int>(fileSize),
        &delays, &width, &height, &frameCount, nullptr, 4
    );

    stbi_set_flip_vertically_on_load(true);

    if (!data || frameCount <= 0) {
        std::cout << "[ScreenTransition] stbi error: " << stbi_failure_reason() << std::endl;
        return;
    }

    std::cout << "[ScreenTransition] Loaded " << frameCount << " frames (" << width << "x" << height << ")" << std::endl;

    int frameSize = width * height * 4;
    int rowSize = width * 4;
    std::vector<unsigned char> flipped(frameSize);

    for (int i = 0; i < frameCount; ++i) {
        GifFrame gf;
        gf.delay = (delays ? delays[i] : 10) * 0.01f;
        gf.delay = (gf.delay < 0.05f) ? 0.1f : gf.delay; // clamp bad values
        // If delay is unreasonably long, override it
        if (gf.delay > 0.5f) gf.delay = 0.1f;

        // Flip vertically - stb returns top-down, OpenGL expects bottom-up
        unsigned char* src = data + i * frameSize;
        for (int row = 0; row < height; ++row) {
            memcpy(
                flipped.data() + row * rowSize,
                src + (height - 1 - row) * rowSize,
                rowSize
            );
        }

        glGenTextures(1, &gf.textureID);
        glBindTexture(GL_TEXTURE_2D, gf.textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, flipped.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        m_GifFrames.push_back(gf);
    }

    stbi_image_free(data);
    if (delays) free(delays);
}

// 
//  Public API
// 
void ScreenTransitionSystem::StartTransition(const std::string& gifPath,
    std::function<void()> onMidpoint) {
    if (m_State != TransitionState::Idle) return;  // already running

    LoadGif(gifPath);
    m_OnMidpoint = onMidpoint;
    m_MidpointFired = false;
    m_Timer = 0.0f;
    m_State = TransitionState::IrisClose;
}

// 
//  Update
// 
void ScreenTransitionSystem::Update(float deltaTime) {
    if (m_State == TransitionState::Idle) return;

    m_Timer += deltaTime;

    switch (m_State) {

    case TransitionState::IrisClose:
        if (m_Timer >= IRIS_CLOSE_DUR) {
            // Fully black  fire scene load callback
            if (!m_MidpointFired) {
                m_MidpointFired = true;
                if (m_OnMidpoint) m_OnMidpoint();
            }
            m_Timer = 0.0f;
            m_State = TransitionState::LoadScreen;
        }
        break;

    case TransitionState::LoadScreen:
        AdvanceGif(deltaTime);
        std::cout << "[ScreenTransition] LoadScreen timer: " << m_Timer << "/" << LOAD_MIN_DUR << std::endl;
        if (m_Timer >= LOAD_MIN_DUR) {
            // You can add an "isLoaded" flag check here if loading is async
            m_Timer = 0.0f;
            m_State = TransitionState::IrisOpen;
        }
        break;

    case TransitionState::IrisOpen:
        std::cout << "[ScreenTransition] IrisOpen timer: " << m_Timer << "/" << IRIS_OPEN_DUR << std::endl;
        if (m_Timer >= IRIS_OPEN_DUR) {
            m_Timer = 0.0f;
            m_State = TransitionState::Idle;
        }
        break;

    default: break;
    }
}

void ScreenTransitionSystem::AdvanceGif(float deltaTime) {
    if (m_GifFrames.empty()) return;
    std::cout << "[GIF] dt=" << deltaTime << " frame=" << m_CurrentFrame
        << " timer=" << m_FrameTimer << " delay=" << m_GifFrames[m_CurrentFrame].delay << std::endl;
    m_FrameTimer += deltaTime;
    float frameDur = m_GifFrames[m_CurrentFrame].delay;
    if (m_FrameTimer >= frameDur) {
        m_FrameTimer -= frameDur;
        m_CurrentFrame = (m_CurrentFrame + 1) % (int)m_GifFrames.size();
    }
}

// 
//  Render  call this LAST in your render loop
// 
void ScreenTransitionSystem::Render() {
    if (m_State == TransitionState::Idle) return;

    // Need blending for the smooth iris edge
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Get current viewport size
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    float vpW = (float)vp[2], vpH = (float)vp[3];

    // Max radius to cover entire screen (corner distance from center in aspect space)
    float maxR = sqrtf((vpW / vpH) * (vpW / vpH) * 0.25f + 0.25f) + 0.05f;

    switch (m_State) {

    case TransitionState::IrisClose: {
        // radius goes from maxR  0  (easeInQuart = snappy at the end)
        float t = m_Timer / IRIS_CLOSE_DUR;
        float radius = maxR * (1.0f - EaseInQuart(t));
        RenderIris(radius);
        break;
    }

    case TransitionState::LoadScreen: {
        // Solid black background + GIF on top
        RenderIris(0.0f);   // radius 0 = fully black quad
        RenderGif();
        break;
    }

    case TransitionState::IrisOpen: {
        float t = m_Timer / IRIS_OPEN_DUR;
        float radius = maxR * EaseOutQuart(t);
        RenderIris(radius);
        break;
    }

    default: break;
    }

    glDisable(GL_BLEND);
    glUseProgram(0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ScreenTransitionSystem::RenderIris(float radius) {
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);

    glUseProgram(m_IrisShader);
    glUniform1f(m_uRadius, radius);
    glUniform2f(m_uResolution, (float)vp[2], (float)vp[3]);
    glUniform2f(m_uCenter, 0.5f, 0.5f);
    glBindVertexArray(m_QuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void ScreenTransitionSystem::RenderGif() {
    if (m_GifFrames.empty()) return;
    glUseProgram(m_GifShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_GifFrames[m_CurrentFrame].textureID);
    glUniform1i(m_uGifTex, 0);
    glBindVertexArray(m_QuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void ScreenTransitionSystem::StartLoadScreenOnly(const std::string& gifPath,
    std::function<void()> onMidpoint) {
    if (m_State != TransitionState::Idle) return;

    LoadGif(gifPath);
    m_OnMidpoint = onMidpoint;
    m_MidpointFired = true; // already fired — skip IrisClose midpoint logic
    m_Timer = 0.0f;
    m_State = TransitionState::LoadScreen; // jump straight to load screen

    // Fire callback immediately
    if (m_OnMidpoint) m_OnMidpoint();
}