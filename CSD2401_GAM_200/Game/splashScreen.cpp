/* Start Header **************************************************************/
/*!
\file   SplashScreenSystem.cpp
\brief  See SplashScreenSystem.h
*/
/* End Header ****************************************************************/
#include "SplashScreen.h"
#include "GraphicsSystem.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>

// ---------------------------------------------------------------------------
// Minimal shaders: textured fullscreen quad, alpha-faded against black BG
// ---------------------------------------------------------------------------
static const char* kSplashVert = R"(
#version 450 core
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;
out vec2 v_uv;
void main()
{
    v_uv        = a_uv;
    gl_Position = vec4(a_pos, 0.0, 1.0);
}
)";

static const char* kSplashFrag = R"(
#version 450 core
in  vec2      v_uv;
out vec4      fragColor;
uniform sampler2D u_tex;
uniform float     u_alpha;
void main()
{
    vec4 c    = texture(u_tex, v_uv);
    fragColor = vec4(c.rgb, c.a * u_alpha);
}
)";

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static GLuint CompileShader(GLenum type, const char* src)
{
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    GLint ok = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[512];
        glGetShaderInfoLog(id, sizeof(log), nullptr, log);
        std::cerr << "[Splash] Shader compile error: " << log << "\n";
    }
    return id;
}

static GLuint LinkProgram(GLuint vert, GLuint frag)
{
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);

    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[512];
        glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
        std::cerr << "[Splash] Shader link error: " << log << "\n";
    }
    return prog;
}

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------
SplashScreenSystem::SplashScreenSystem(GraphicsSystem* gfx,
    const std::string& schoolLogoPath,
    const std::string& gameLogoPath,
    float logoHoldSecs,
    float fadeSecs)
    : m_gfx(gfx)
    , m_schoolPath(schoolLogoPath)
    , m_gamePath(gameLogoPath)
    , m_holdSecs(logoHoldSecs)
    , m_fadeSecs(fadeSecs)
{
}

SplashScreenSystem::~SplashScreenSystem()
{
    if (m_quadVAO) { glDeleteVertexArrays(1, &m_quadVAO);  m_quadVAO = 0; }
    if (m_quadVBO) { glDeleteBuffers(1, &m_quadVBO);       m_quadVBO = 0; }
    if (m_schoolTex) { glDeleteTextures(1, &m_schoolTex);    m_schoolTex = 0; }
    if (m_gameTex) { glDeleteTextures(1, &m_gameTex);      m_gameTex = 0; }
    if (m_progHandle) { glDeleteProgram(m_progHandle);        m_progHandle = 0; }
}

// ---------------------------------------------------------------------------
// Initialize
// ---------------------------------------------------------------------------
void SplashScreenSystem::Initialize()
{
    // Load school logo and capture its dimensions
    stbi_set_flip_vertically_on_load(true);
    {
        int channels;
        unsigned char* data = stbi_load(m_schoolPath.c_str(), &m_schoolW, &m_schoolH, &channels, 4);
        if (data)
        {
            glGenTextures(1, &m_schoolTex);
            glBindTexture(GL_TEXTURE_2D, m_schoolTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_schoolW, m_schoolH, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);
            stbi_image_free(data);
        }
        else
            std::cerr << "[Splash] WARNING: could not load school logo: " << m_schoolPath << "\n";
    }

    // Load game logo and capture its dimensions
    {
        int channels;
        unsigned char* data = stbi_load(m_gamePath.c_str(), &m_gameW, &m_gameH, &channels, 4);
        if (data)
        {
            glGenTextures(1, &m_gameTex);
            glBindTexture(GL_TEXTURE_2D, m_gameTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_gameW, m_gameH, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);
            stbi_image_free(data);
        }
        else
            std::cerr << "[Splash] WARNING: could not load game logo: " << m_gamePath << "\n";
    }

    BuildShader();
    // quads are built per-draw now (see DrawLogo), no static BuildQuad needed
    BuildQuad(1.f, 1.f); // dummy call just to allocate the VAO/VBO
}

// ---------------------------------------------------------------------------
// Run  (blocking mini-loop)
// ---------------------------------------------------------------------------
void SplashScreenSystem::Run()
{
    if (!m_gfx) return;

    GLFWwindow* win = m_gfx->getWindow();
    m_prevTime = glfwGetTime();
    m_state = State::FadeInSchool;
    m_stateTime = 0.f;

    while (m_state != State::Done && !glfwWindowShouldClose(win))
    {
        // --- delta time ---
        double now = glfwGetTime();
        float  dt = static_cast<float>(now - m_prevTime);
        m_prevTime = now;

        // --- query actual framebuffer size (correct in both windowed and fullscreen) ---
        int fbW, fbH;
        glfwGetFramebufferSize(win, &fbW, &fbH);

        // --- resolve current logo and alpha ---
        GLuint currentTex = 0;
        float  alpha = 1.f;
        int    imgW = 1, imgH = 1;

        switch (m_state)
        {
        case State::FadeInSchool:
            currentTex = m_schoolTex;  imgW = m_schoolW;  imgH = m_schoolH;
            alpha = std::min(m_stateTime / m_fadeSecs, 1.f);
            break;
        case State::HoldSchool:
            currentTex = m_schoolTex;  imgW = m_schoolW;  imgH = m_schoolH;
            alpha = 1.f;
            break;
        case State::FadeOutSchool:
            currentTex = m_schoolTex;  imgW = m_schoolW;  imgH = m_schoolH;
            alpha = 1.f - std::min(m_stateTime / m_fadeSecs, 1.f);
            break;
        case State::FadeInGame:
            currentTex = m_gameTex;  imgW = m_gameW;  imgH = m_gameH;
            alpha = std::min(m_stateTime / m_fadeSecs, 1.f);
            break;
        case State::HoldGame:
            currentTex = m_gameTex;  imgW = m_gameW;  imgH = m_gameH;
            alpha = 1.f;
            break;
        case State::FadeOutGame:
            currentTex = m_gameTex;  imgW = m_gameW;  imgH = m_gameH;
            alpha = 1.f - std::min(m_stateTime / m_fadeSecs, 1.f);
            break;
        default:
            break;
        }

        // --- render ---
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, fbW, fbH);          // use actual framebuffer size
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (currentTex)
            DrawLogo(currentTex, alpha, imgW, imgH, fbW, fbH);  // pass fb dims

        glfwSwapBuffers(win);
        glfwPollEvents();

        AdvanceState(dt);
    }

    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glDisable(GL_DEPTH_TEST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

// ---------------------------------------------------------------------------
// AdvanceState
// ---------------------------------------------------------------------------
void SplashScreenSystem::AdvanceState(float dt)
{
    m_stateTime += dt;

    auto next = [&](State s) { m_state = s; m_stateTime = 0.f; };

    switch (m_state)
    {
    case State::FadeInSchool:
        if (m_stateTime >= m_fadeSecs) next(State::HoldSchool);
        break;
    case State::HoldSchool:
        if (m_stateTime >= m_holdSecs) next(State::FadeOutSchool);
        break;
    case State::FadeOutSchool:
        if (m_stateTime >= m_fadeSecs) next(State::FadeInGame);
        break;
    case State::FadeInGame:
        if (m_stateTime >= m_fadeSecs) next(State::HoldGame);
        break;
    case State::HoldGame:
        if (m_stateTime >= m_holdSecs) next(State::FadeOutGame);
        break;
    case State::FadeOutGame:
        if (m_stateTime >= m_fadeSecs) next(State::Done);
        break;
    default:
        break;
    }
}

// ---------------------------------------------------------------------------
// DrawLogo
// ---------------------------------------------------------------------------
void SplashScreenSystem::DrawLogo(GLuint tex, float alpha, int imgW, int imgH, int fbW, int fbH)
{
    if (!m_progHandle || !m_quadVAO) return;

    const float coverage = 0.4f;

    // Use the passed-in framebuffer dimensions — correct in fullscreen
    float screenAspect = (float)fbW / (float)fbH;
    float logoAspect = (float)imgW / (float)imgH;

    float ndcH = coverage;
    float ndcW = ndcH * logoAspect / screenAspect;

    if (ndcW > coverage)
    {
        ndcW = coverage;
        ndcH = ndcW * screenAspect / logoAspect;
    }

    BuildQuad(ndcW, ndcH);

    glUseProgram(m_progHandle);
    glUniform1f(glGetUniformLocation(m_progHandle, "u_alpha"), alpha);
    glUniform1i(glGetUniformLocation(m_progHandle, "u_tex"), 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glUseProgram(0);
}

// ---------------------------------------------------------------------------
// BuildQuad  (NDC fullscreen, two triangles)
// No manual V-flip — GraphicsSystem::LoadTexture sets
// stbi_set_flip_vertically_on_load(true) so UVs are standard 0=bottom, 1=top
// ---------------------------------------------------------------------------
void SplashScreenSystem::BuildQuad(float halfW, float halfH)
{
    // If VAO/VBO don't exist yet, create them once
    if (!m_quadVAO)
    {
        glGenVertexArrays(1, &m_quadVAO);
        glGenBuffers(1, &m_quadVBO);
    }

    float verts[] = {
        -halfW, -halfH,   0.f, 0.f,
         halfW, -halfH,   1.f, 0.f,
         halfW,  halfH,   1.f, 1.f,

        -halfW, -halfH,   0.f, 0.f,
         halfW,  halfH,   1.f, 1.f,
        -halfW,  halfH,   0.f, 1.f,
    };

    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
        4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}

// ---------------------------------------------------------------------------
// BuildShader
// ---------------------------------------------------------------------------
void SplashScreenSystem::BuildShader()
{
    GLuint vert = CompileShader(GL_VERTEX_SHADER, kSplashVert);
    GLuint frag = CompileShader(GL_FRAGMENT_SHADER, kSplashFrag);
    m_progHandle = LinkProgram(vert, frag);

    // Stage objects are linked into the program, safe to delete them now
    glDeleteShader(vert);
    glDeleteShader(frag);
}