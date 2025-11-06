/* Start Header ************************************************************************/
/*!
\file       GraphicsSystem.cpp
\author     Guan Shao Jun [85%]
\par        shaojun.g@digipen.edu
\date       October 02 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "GraphicsSystem.h"
#include "Animator.hpp"

const char *configFile = "./Logs/config.txt";
const double two_pi = 6.28318530717958647692528676655900576;

GraphicsSystem::GraphicsSystem()
    : window(nullptr), vao(0), vbo(0), Wwidth(800), Wheight(600), tex(0), vp(), batchVAO(0), batchVBO(0), batchEBO(0), textVAO(0), textVBO(0),
      frameBuffer(0), frameBufferTexture(0), depthRenderBuffer(0),
      frameBufferWidth(800), frameBufferHeight(600), editorMode(true)
{
    // Initialize GLFW library (required before creating a window/context).
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return;
    }

    // Create a resizable window with fixed default size (800x600 for now).
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(Wwidth, Wheight, "Game Engine", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return;
    }

    // Store pointer to this GraphicsSystem instance in the GLFW window,
    // so we can retrieve it inside callbacks.
    glfwSetWindowUserPointer(window, this);

    // Set callback for window resizing (updates viewport + stored dimensions).
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *win, int width, int height)
                                   {
            glViewport(0, 0, width, height);

            // Update stored window width/height in GraphicsSystem.
            auto* gs = static_cast<GraphicsSystem*>(glfwGetWindowUserPointer(win));
            if (gs) {
                gs->Wwidth = width;
                gs->Wheight = height;
            } });

    // Make OpenGL context current for this window.
    glfwMakeContextCurrent(window);

    // Query the framebuffer size (accounts for HiDPI displays).
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);

    // Store the actual framebuffer size.
    Wwidth = fbWidth;
    Wheight = fbHeight;
}

GraphicsSystem::GraphicsSystem(GLint w, GLint h)
    : window(nullptr), vao(0), vbo(0), Wwidth(w), Wheight(h), vp(), batchVAO(0), batchVBO(0), batchEBO(0), textVAO(0), textVBO(0),
      frameBuffer(0), frameBufferTexture(0), depthRenderBuffer(0),
      frameBufferWidth(800), frameBufferHeight(600), editorMode(true)
{
    // Same as default constructor, but allows user to specify window size.

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(Wwidth, Wheight, "Game Engine", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return;
    }

    // Store pointer to this GraphicsSystem instance in the GLFW window,
    // so we can retrieve it inside callbacks.
    glfwSetWindowUserPointer(window, this);

    // Set callback for window resizing (updates viewport + stored dimensions).
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *win, int width, int height)
                                   {
            glViewport(0, 0, width, height);

            // Update stored window width/height in GraphicsSystem.
            auto* gs = static_cast<GraphicsSystem*>(glfwGetWindowUserPointer(win));
            if (gs) {
                gs->Wwidth = width;
                gs->Wheight = height;
            } });

    // Make OpenGL context current for this window.
    glfwMakeContextCurrent(window);

    // Query the framebuffer size (accounts for HiDPI displays).
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);

    // Store the actual framebuffer size.
    Wwidth = fbWidth;
    Wheight = fbHeight;
}

GraphicsSystem::~GraphicsSystem()
{
    DeleteFrameBuffer();

    // Clean up resources on destruction.
    if (window)
    {
        glfwDestroyWindow(window); // Destroy GLFW window
        glfwTerminate();           // Terminate GLFW (frees global resources)
    }
}

void GraphicsSystem::Initialize()
{
    // Initialize GLEW (required to access modern OpenGL functions).
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW\n";
        return;
    }

    // Enable blending (allows transparency in textures and colors).
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Compile and link the default shader program for rendering.
    // Requires "vertices.vert" (vertex shader) and "fragment.frag" (fragment shader).
    shader.CompileLinkValidate({{GL_VERTEX_SHADER, "./Shaders/vertices.vert"},
                                {GL_FRAGMENT_SHADER, "./Shaders/fragment.frag"}});

    // Define vertex structure for a textured quad.
    struct Vertex
    {
        vec3 pos; // Position (x, y, z)
        vec2 tex; // Texture coordinates (u, v)
    };

    // Quad geometry (two triangles forming a square).
    // Coordinates are in NDC space, centered at origin.
    Vertex vertices[] = {
        {{-0.5f, -0.5f, 0.f}, {0.f, 0.f}}, // bottom-left
        {{0.5f, -0.5f, 0.f}, {1.f, 0.f}},  // bottom-right
        {{-0.5f, 0.5f, 0.f}, {0.f, 1.f}},  // top-left
        {{0.5f, 0.5f, 0.f}, {1.f, 1.f}}    // top-right
    };

    // === VAO (Vertex Array Object) + VBO (Vertex Buffer Object) setup ===

    // Create a VAO to store the state of vertex attribute bindings.
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create a VBO and upload vertex data into GPU memory.
    glCreateBuffers(1, &vbo);
    glNamedBufferStorage(vbo, sizeof(vertices), vertices, 0);

    // Bind the VBO to the VAO at binding point 0.
    // Each vertex is sizeof(Vertex) apart.
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));

    // --- Attribute 0: Position (vec3) ---
    glEnableVertexArrayAttrib(vao, 0); // enable attribute location 0
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
    glVertexArrayAttribBinding(vao, 0, 0); // bind attribute 0 to binding point 0

    // --- Attribute 1: Texture coordinates (vec2) ---
    glEnableVertexArrayAttrib(vao, 1); // enable attribute location 1
    glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, tex));
    glVertexArrayAttribBinding(vao, 1, 0); // bind attribute 1 to binding point 0

    // Initialise batch rendering buffers
    glCreateVertexArrays(1, &batchVAO);
    glCreateBuffers(1, &batchVBO);
    glCreateBuffers(1, &batchEBO);

    // Allocate large buffers for batch rendering
    glNamedBufferStorage(batchVBO, MAX_BATCH_VERTICES * sizeof(BatchVertex), nullptr, GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferStorage(batchEBO, MAX_BATCH_INDICES * sizeof(GLuint), nullptr, GL_DYNAMIC_STORAGE_BIT);

    // bind buffers to VAO
    glVertexArrayVertexBuffer(batchVAO, 0, batchVBO, 0, sizeof(BatchVertex));
    glVertexArrayElementBuffer(batchVAO, batchEBO);

    // Attribute 0: Position (vec3)
    glEnableVertexArrayAttrib(batchVAO, 0);
    glVertexArrayAttribFormat(batchVAO, 0, 3, GL_FLOAT, GL_FALSE, offsetof(BatchVertex, pos));
    glVertexArrayAttribBinding(batchVAO, 0, 0);

    // Attribute 1: Texture Coords (vec2)
    glEnableVertexArrayAttrib(batchVAO, 1);
    glVertexArrayAttribFormat(batchVAO, 1, 2, GL_FLOAT, GL_FALSE, offsetof(BatchVertex, tex));
    glVertexArrayAttribBinding(batchVAO, 1, 0);

    // Attribute 2: Color (vec4)
    glEnableVertexArrayAttrib(batchVAO, 2);
    glVertexArrayAttribFormat(batchVAO, 2, 4, GL_FLOAT, GL_FALSE, offsetof(BatchVertex, color));
    glVertexArrayAttribBinding(batchVAO, 2, 0);

    // Attribute 3: UV Scale (vec2)
    glEnableVertexArrayAttrib(batchVAO, 3);
    glVertexArrayAttribFormat(batchVAO, 3, 2, GL_FLOAT, GL_FALSE, offsetof(BatchVertex, uvScale));
    glVertexArrayAttribBinding(batchVAO, 3, 0);

    // Attribute 4: UV Offset (vec2)
    glEnableVertexArrayAttrib(batchVAO, 4);
    glVertexArrayAttribFormat(batchVAO, 4, 2, GL_FLOAT, GL_FALSE, offsetof(BatchVertex, uvOffset));
    glVertexArrayAttribBinding(batchVAO, 4, 0);

    // Attribute 5: Transform (vec3)
    glEnableVertexArrayAttrib(batchVAO, 5);
    glVertexArrayAttribFormat(batchVAO, 5, 3, GL_FLOAT, GL_FALSE, offsetof(BatchVertex, transform));
    glVertexArrayAttribBinding(batchVAO, 5, 0);

    // Attribute 6: Scale (vec2)
    glEnableVertexArrayAttrib(batchVAO, 6);
    glVertexArrayAttribFormat(batchVAO, 6, 2, GL_FLOAT, GL_FALSE, offsetof(BatchVertex, scale));
    glVertexArrayAttribBinding(batchVAO, 6, 0);

    // Text Shader Initialize
    textShader.CompileLinkValidate({{GL_VERTEX_SHADER, "./Shaders/text.vert"},
                                    {GL_FRAGMENT_SHADER, "./Shaders/text.frag"}});

    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);

    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    CreateFrameBuffer(800, 600);
}

void GraphicsSystem::Update(float)
{
    // std::cout << "Debug Mode: " << (debugMode ? "ON" : "OFF") << std::endl;

    if (!window)
        return; // Skip if no valid window.

    if (editorMode)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glViewport(0, 0, frameBufferWidth, frameBufferHeight);
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, Wwidth, Wheight);
    }

    // === Clear Screen ===
    glClearColor(0.6f, 0.5f, 0.45f, 0.0f);              // Background color (RGBA)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color buffer
    glLineWidth(2.f);                                   // Set line thickness
    glPointSize(50.f);                                  // Set point size (for debug points)

    // Use main shader program
    shader.Use();

    //// Orthographic projection: origin at bottom-left, size = window width/height
    //  Use editor camera for level editing, otherwise use default game camera
        float renderWidth = editorMode ? (float)(frameBufferWidth) : float(Wwidth);
        float renderHeight = editorMode ? (float)(frameBufferHeight) : float(Wheight);
    if (levelEditor && (levelEditor->GetEditorMode() == EditorMode::Stop))
    {
        // EDITOR MODE: Use free editor camera for level editing
        // This allows designers to navigate and edit the level
        vp = levelEditor->GetEditorViewProjection((float)renderWidth, (float)renderHeight);
    }
    else
    {
        vp = CameraManager::Instance().GetViewProjectMatrix(renderWidth, renderHeight);
        vec2 camPos = CameraManager::Instance().GetCameraPosition();
    }
    // Accumulate all geometry into batches
    batches.clear();

    auto &gameObjects = GameObjectFactory::Instance().GetAllGameObjects();
    for (auto &obj : gameObjects)
    {
        if (!obj->IsActive())
        {
            continue;
        }

        auto *transform = obj->GetComponent<TransformComponent>();
        if (!transform)
        {
            continue;
        }

        auto *rend = obj->GetComponent<RenderComponent>();

        if (!rend)
        {
            continue;
        }
        // Only batch Square and Circle for now
        if (rend->GetShapeType() != RenderComponent::ShapeType::Circle && rend->GetShapeType() != RenderComponent::ShapeType::Square)
        {
            continue;
        }

        // transform data
        float x, y, sx, sy;
        transform->GetPosition(x, y);
        transform->GetScale(sx, sy);
        float rot = transform->GetRotation();

        // Render data
        GLuint texID = rend->GetTextureID();
        vec4 color = vec4(rend->GetColor());
        bool isCircle = (rend->GetShapeType() == RenderComponent::ShapeType::Circle);
        int layer = rend->GetLayer();

        // UV data
        vec2 uvScale = vec2(1.0f);
        vec2 uvOffset = vec2(0.0f);
        if (auto *animator = obj->GetComponent<Spritesheet>())
        {
            uvScale = animator->GetUVScale();
            uvOffset = animator->GetUVOffset();
        }

        RenderBatch *targetBatch = nullptr;
        for (auto &batch : batches)
        {
            if (batch.textureID == texID && batch.isCircle == isCircle && batch.layer == layer)
            {
                targetBatch = &batch;
                break;
            }
        }

        if (!targetBatch)
        {
            batches.emplace_back(texID, isCircle, layer);
            targetBatch = &batches.back();

            targetBatch->vertices.reserve(20000);
            targetBatch->indices.reserve(30000);

            // std::cout << "Created batch - TexID: " << texID
            //     << ", Layer: " << layer
            //     << ", IsCircle: " << isCircle << std::endl;
        }

        //-------------------- TBR -------------------------------
        // mat4 model = mat4::translate(vec3(x, y, 0.0f));
        // model = model * mat4::rotate(rot, vec3(0, 0, 1));
        // model = model * mat4::scale(vec3(sx, sy, 1.0f));
        //--------------------------------------------------------

        vec3 localVerts[4] = {
            {-0.5f, -0.5f, 0.f},
            {0.5f, -0.5f, 0.f},
            {-0.5f, 0.5f, 0.f},
            {0.5f, 0.5f, 0.f}};

        vec2 texCoords[4] = {
            {0.f, 0.f},
            {1.f, 0.f},
            {0.f, 1.f},
            {1.f, 1.f}};

        GLuint baseIndex = (GLuint)targetBatch->vertices.size();

        // add 4 vertices to batch
        for (int i = 0; i < 4; ++i)
        {
            // vec3 worldPos = model * vec4(localVerts[i], 1.0f);

            BatchVertex vert;
            vert.pos = localVerts[i];
            vert.tex = texCoords[i];
            vert.color = color;
            vert.uvScale = uvScale;
            vert.uvOffset = uvOffset;
            vert.transform = vec3(x, y, rot);
            vert.scale = vec2(sx, sy);

            targetBatch->vertices.push_back(vert);
        }

        // add 6 indices for 2 triangles (quad)
        targetBatch->indices.push_back(baseIndex + 0);
        targetBatch->indices.push_back(baseIndex + 1);
        targetBatch->indices.push_back(baseIndex + 2);
        targetBatch->indices.push_back(baseIndex + 2);
        targetBatch->indices.push_back(baseIndex + 1);
        targetBatch->indices.push_back(baseIndex + 3);
    }

    // auto afterAccumulate = std::chrono::high_resolution_clock::now();

    std::sort(batches.begin(), batches.end(), [](const RenderBatch &a, const RenderBatch &b)
              { return a.layer < b.layer; });
    // auto afterSort = std::chrono::high_resolution_clock::now();

    // std::cout << "\n=== Batch Order After Sorting ===" << std::endl;
    // for (size_t i = 0; i < batches.size(); ++i) {
    //     std::cout << "Batch " << i << ": Layer " << batches[i].layer
    //         << ", TexID " << batches[i].textureID
    //         << ", Vertices " << batches[i].vertices.size() << std::endl;
    // }
    // Draw all batches
    DrawBatches();
    // auto afterDraw = std::chrono::high_resolution_clock::now();

    // Print timing (only occasionally, not every frame)
    // static int frameCount = 0;
    // if (++frameCount % 60 == 0) {
    //    auto accumulateMs = std::chrono::duration<double, std::milli>(afterAccumulate - start).count();
    //    auto sortMs = std::chrono::duration<double, std::milli>(afterSort - afterAccumulate).count();
    //    auto drawMs = std::chrono::duration<double, std::milli>(afterDraw - afterSort).count();

    //    std::cout << "Accumulate: " << accumulateMs << "ms, "
    //        << "Sort: " << sortMs << "ms, "
    //        << "Draw: " << drawMs << "ms" << std::endl;
    //}

    // Draw Non-batched shapes
    for (auto &obj : gameObjects)
    {
        if (!obj->IsActive())
        {
            continue;
        }

        auto *rend = obj->GetComponent<RenderComponent>();
        if (!rend)
        {
            continue;
        }

        // Draw Lines and Points (not batched)
        switch (rend->GetShapeType())
        {
        case RenderComponent::ShapeType::Line:
        {
            auto line = rend->GetLine();
            DrawLine(line.start, line.end, line.color, vp);
            break;
        }
        case RenderComponent::ShapeType::Point:
        {
            auto point = rend->GetPoint();
            DrawPoint(point.pos, point.color, vp);
            break;
        }
        default:
            break;
        }

        // Draw debug outlines for ALL shapes (moved outside switch)
        if (debugMode)
        {
            // std::cout << "went here" << std::endl;
            auto *transform = obj->GetComponent<TransformComponent>();
            if (transform)
            {
                float x, y, sx, sy;
                transform->GetPosition(x, y);
                transform->GetScale(sx, sy);

                if (rend->GetShapeType() == RenderComponent::ShapeType::Circle)
                {
                    DrawCircleOutline(x, y, static_cast<float>(sx * 0.5), static_cast<int>(sy), {0, 1, 0, 1}, vp);
                }
                else if (rend->GetShapeType() == RenderComponent::ShapeType::Square)
                {
                    DrawRectOutline(x, y, sx, sy, {0, 1, 0, 1}, vp);
                }
            }
        }

        // Text rendering
        if (auto *textComp = obj->GetComponent<TextComponent>())
        {
            if (textComp->IsEnabled())
            {
                if (auto *transform = obj->GetComponent<TransformComponent>())
                {
                    float x, y;
                    transform->GetPosition(x, y);
                    RenderText(textComp->GetCharacters(), textComp->GetText(), x, y,
                               textComp->GetScale(), textComp->GetColor(), textComp->isScreenSpace());
                }
            }
        }
    }

    if (debugMode)
    {
        auto &debugShapes = CORE->GetDebugSystem()->GetShapes();
        for (auto &shape : debugShapes)
        {
            switch (shape.type)
            {
            case DebugSystem::DebugShape::LINE:
            {
                DrawLine(shape.p1, shape.p2, shape.color, vp);
                break;
            }
            case DebugSystem::DebugShape::POINT:
            {
                DrawPoint(shape.p1, shape.color, vp);
                break;
            }
            case DebugSystem::DebugShape::RECT:
            {
                DrawRectOutline(shape.p1.x, shape.p1.y, shape.p2.x, shape.p2.y, shape.color, vp);
                break;
            }
            case DebugSystem::DebugShape::CIRCLE:
            {
                DrawCircleOutline(shape.p1.x, shape.p1.y, shape.radius, shape.segments, shape.color, vp);
                break;
            }
            }
        }
    }

    if (editorMode)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

// Draw a line between two points in world space
void GraphicsSystem::DrawLine(const vec2 &start, const vec2 &end,
                              const vec4 &color, const mat4 &viewProjection)
{
    // Tell shader this is a line draw
    shader.SetUniform("uIsLine", 1);
    shader.SetUniform("uColor", color);
    shader.SetUniform("uHasTexture", 0);
    shader.SetUniform("uIsCircle", 0);
    shader.SetUniform("uUseBatching", 0);

    // Define 2 vertices for the line
    vec3 verts[2] = {
        {start.x, start.y, 0.0f},
        {end.x, end.y, 0.0f}};

    // Create temporary VAO/VBO for this line
    GLuint tempVBO, tempVAO;
    glGenVertexArrays(1, &tempVAO);
    glGenBuffers(1, &tempVBO);

    glBindVertexArray(tempVAO);
    glBindBuffer(GL_ARRAY_BUFFER, tempVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);

    // Attribute 0 = vertex position (vec3 in shader)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);

    // Send MVP matrix
    shader.SetUniform("uMVP", viewProjection);

    // Draw the line
    glDrawArrays(GL_LINES, 0, 2);

    // Cleanup temp objects
    glDeleteBuffers(1, &tempVBO);
    glDeleteVertexArrays(1, &tempVAO);
}

// Draw a single point in world space
void GraphicsSystem::DrawPoint(const vec2 &ppos,
                               const vec4 &color, const mat4 &viewProjection)
{
    // Tell shader this is NOT a line
    shader.SetUniform("uIsLine", 0);
    shader.SetUniform("uIsCircle", 0); // keep consistent with line draw
    shader.SetUniform("uColor", color);
    shader.SetUniform("uHasTexture", 0);

    // Position of the point (use vec3 to match shader input = vec3)
    vec3 pos = {ppos.x, ppos.y, 0.0f};

    // Create temporary VAO/VBO for this point
    GLuint tempVBO, tempVAO;
    glGenVertexArrays(1, &tempVAO);
    glGenBuffers(1, &tempVBO);

    glBindVertexArray(tempVAO);
    glBindBuffer(GL_ARRAY_BUFFER, tempVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pos), &pos, GL_DYNAMIC_DRAW);

    // Attribute 0 = vertex position (vec3 in shader)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);

    // Send MVP matrix
    shader.SetUniform("uMVP", viewProjection);

    // Make point visible (default = 1px, often too small)
    glPointSize(5.0f);

    // Draw the point
    glDrawArrays(GL_POINTS, 0, 1);

    // Cleanup temp objects
    glDeleteBuffers(1, &tempVBO);
    glDeleteVertexArrays(1, &tempVAO);
}

// Draw an axis-aligned rectangle outline centered at (x, y)
// w = width, h = height
void GraphicsSystem::DrawRectOutline(float x, float y, float w, float h,
                                     const vec4 &color, const mat4 &viewProjection)
{
    // Half extents (distance from center to edge)
    float halfW = w / 2;
    float halfH = h / 2;

    // Compute 4 corner points of the rectangle
    vec2 p1(x - halfW, y - halfH); // bottom-left
    vec2 p2(x + halfW, y - halfH); // bottom-right
    vec2 p3(x + halfW, y + halfH); // top-right
    vec2 p4(x - halfW, y + halfH); // top-left

    // Connect corners with lines (in order)
    DrawLine(p1, p2, color, viewProjection);
    DrawLine(p2, p3, color, viewProjection);
    DrawLine(p3, p4, color, viewProjection);
    DrawLine(p4, p1, color, viewProjection);
}

// Draw a circle outline centered at (cx, cy)
// r = radius, segments = how many line segments to approximate the circle
void GraphicsSystem::DrawCircleOutline(float cx, float cy, float r, int segments,
                                       const vec4 &color, const mat4 &viewProjection)
{
    // Clamp to a minimum number of segments (otherwise circle looks too blocky)
    if (segments < 8)
        segments = 8;

    // Angle step per segment (full 360� / segments)
    float step = static_cast<float>(two_pi) / segments;

    // Generate connected line segments around the circle
    for (int i = 0; i < segments; i++)
    {
        float theta1 = step * i;
        float theta2 = step * (i + 1);

        // Two consecutive points on the circle�s circumference
        vec2 p1 = {cx + r * cos(theta1), cy + r * sin(theta1)};
        vec2 p2 = {cx + r * cos(theta2), cy + r * sin(theta2)};

        // Draw line between them
        DrawLine(p1, p2, color, viewProjection);
    }
}

void GraphicsSystem::DrawBatches()
{
    if (batches.empty())
    {
        return;
    }

    shader.Use();
    shader.SetUniform("uUseBatching", 1);
    shader.SetUniform("uIsLine", 0);

    glBindVertexArray(batchVAO);

    for (auto &batch : batches)
    {
        if (batch.vertices.empty())
        {
            continue;
        }

        // === ADD THIS SAFETY CHECK ===
        if (batch.vertices.size() > MAX_BATCH_VERTICES)
        {
            std::cerr << "ERROR: Batch exceeds vertex buffer size! "
                      << batch.vertices.size() << " > " << MAX_BATCH_VERTICES << std::endl;
            continue;
        }
        if (batch.indices.size() > MAX_BATCH_INDICES)
        {
            std::cerr << "ERROR: Batch exceeds index buffer size! "
                      << batch.indices.size() << " > " << MAX_BATCH_INDICES << std::endl;
            continue;
        }
        // === END SAFETY CHECK ===

        // Upload vertex data
        glNamedBufferSubData(batchVBO, 0, batch.vertices.size() * sizeof(BatchVertex), batch.vertices.data());

        // Upload index data
        glNamedBufferSubData(batchEBO, 0, batch.indices.size() * sizeof(GLuint), batch.indices.data());

        // Set Uniform
        shader.SetUniform("uMVP", vp);
        shader.SetUniform("uIsCircle", batch.isCircle ? 1 : 0);

        // Bind Texture
        if (batch.textureID > 0)
        {
            shader.SetUniform("uHasTexture", 1);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, batch.textureID);
            shader.SetUniform("uTexture", 0);
        }
        else
        {
            shader.SetUniform("uHasTexture", 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        glDrawElements(GL_TRIANGLES, (GLsizei)batch.indices.size(), GL_UNSIGNED_INT, 0);
    }
    shader.SetUniform("uUseBatching", 0);
}

void GraphicsSystem::RenderText(const std::map<char, Character> &characters, const std::string &text, float x, float y, float scale, const vec4 &color, bool screenSpace)
{
    textShader.Use();
    textShader.SetUniform("textColor", color);

    mat4 projection;
    float renderWidth = editorMode ? (float)(frameBufferWidth) : float(Wwidth);
    float renderHeight = editorMode ? (float)(frameBufferHeight) : float(Wheight);
    if (screenSpace)
    {
        projection = mat4::ortho(0.0f, (float)renderWidth, 0.0f, (float)renderHeight);
    }
    else
    {
        projection = vp;
    }
    textShader.SetUniform("projection", projection);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);

    for (char c : text)
    {
        auto it = characters.find(c);
        if (it == characters.end())
        {
            continue;
        }

        const Character &ch = it->second;

        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;
        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        float vertices[6][4] = {
            {xpos, ypos + h, 0.f, 0.f},
            {xpos, ypos, 0.f, 1.f},
            {xpos + w, ypos, 1.f, 1.f},

            {xpos, ypos + h, 0.f, 0.f},
            {xpos + w, ypos, 1.f, 1.f},
            {xpos + w, ypos + h, 1.f, 0.f}};

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GraphicsSystem::CreateFrameBuffer(int width, int height)
{
    frameBufferWidth = width;
    frameBufferHeight = height;

    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    glGenTextures(1, &frameBufferTexture);
    glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBufferTexture, 0);

    glGenRenderbuffers(1, &depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "ERROR: Framebuffer is not complete! \n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::cout << "framebuffer created " << width << "x" << height << std::endl;
}

void GraphicsSystem::DeleteFrameBuffer()
{
    if (frameBuffer)
    {
        glDeleteFramebuffers(1, &frameBuffer);
        glDeleteTextures(1, &frameBufferTexture);
        glDeleteRenderbuffers(1, &depthRenderBuffer);
        frameBuffer = 0;
        frameBufferTexture = 0;
        depthRenderBuffer = 0;
    }
}

void GraphicsSystem::ResizeFrameBuffer(int width, int height)
{
    if (width <= 0 || height <= 0)
    {
        return;
    }
    DeleteFrameBuffer();
    CreateFrameBuffer(width, height);
}

// apologies for messy code, used 2 float refs instead of ImVec2 because i didnt wanna include imgui.h here and slow compile time
void GraphicsSystem::RenderGameViewportImGui(float& viewportSizeX, float& viewportSizeY)
{
    //if (!editorMode)
    //{
    //    return;
    //}

    if (viewportSizeX < 100.0f)
        viewportSizeX = 800.0f;
    if (viewportSizeY < 100.0f)
        viewportSizeY = 600.0f;

    if ((int)viewportSizeX != frameBufferWidth || (int)viewportSizeY != frameBufferHeight)
    {
        if (viewportSizeX > 0 && viewportSizeY > 0)
        {
            ResizeFrameBuffer((int)viewportSizeX, (int)viewportSizeY);
        }
    }
}

std::string GraphicsSystem::GetName()
{
    return "Graphics System";
}

// Load a 2D texture from a file using stb_image
// Returns the OpenGL texture ID (GLuint), or 0 if loading fails
GLuint GraphicsSystem::LoadTexture(const char *filepath)
{
    // Tell stb_image to flip images vertically (so origin is bottom-left, like OpenGL expects)
    stbi_set_flip_vertically_on_load(true);

    int width, height, channels;
    // Force load with 4 channels (RGBA) regardless of source image format
    unsigned char *data = stbi_load(filepath, &width, &height, &channels, 4);
    if (!data)
    {
        std::cerr << "Failed to load texture: " << filepath << std::endl;
        return 0; // return 0 to signal failure
    }

    // Generate a texture object and bind it
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Upload pixel data to GPU (level 0, no border, RGBA format)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, data);

    // Generate mipmaps for scaling
    glGenerateMipmap(GL_TEXTURE_2D);

    // Set texture wrapping mode (repeat when UV > 1.0 or < 0.0)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // X axis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Y axis

    // Set filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // smooth with mipmaps
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);               // smooth (no mipmaps for magnification)

    // Free image memory (data is now on GPU)
    stbi_image_free(data);

    // Return texture ID for later use
    return texture;
}

std::unique_ptr<IComponent> RenderComponent::Clone(GameObject *NewClone) const
{

    // 1. Determine which constructor to use based on the original component's state.
    std::unique_ptr<RenderComponent> clonedComponent;

    // Check if the component uses a texture (textureID is non-zero)
    if (this->textureID != 0)
    {
        // Use the constructor for textured objects:
        // RenderComponent(owner, GLuint texID)
        clonedComponent = std::make_unique<RenderComponent>(NewClone, this->textureID);
    }
    else
    {
        // Use the constructor for shape/color:
        // RenderComponent(owner, ShapeType type, const vec4& color)
        clonedComponent = std::make_unique<RenderComponent>(NewClone, this->shape, this->color);
    }

    // 2. Clone specialized data for Line or Point types.
    if (this->shape == RenderComponent::ShapeType::Line)
    {
        // Copy the line data using the public setter
        clonedComponent->SetLine(this->line.start, this->line.end);
    }
    else if (this->shape == RenderComponent::ShapeType::Point)
    {
        // Copy the point data using the public setter
        clonedComponent->SetPoint(this->point.pos, this->point.size);
    }

    // 3. Clone the enabled state.
    clonedComponent->SetEnabled(this->IsEnabled());

    clonedComponent->SetLayer(this->layer);

    return clonedComponent;
}