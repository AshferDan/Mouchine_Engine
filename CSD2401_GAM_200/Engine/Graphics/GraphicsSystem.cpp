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

const char *configFile = "../Logs/config.txt";

GraphicsSystem::GraphicsSystem()
    : window(nullptr), vao(0), vbo(0), Wwidth(800), Wheight(600), tex(0), vp(0)
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
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int width, int height)
        {
            glViewport(0, 0, width, height);

            // Update stored window width/height in GraphicsSystem.
            auto* gs = static_cast<GraphicsSystem*>(glfwGetWindowUserPointer(win));
            if (gs) {
                gs->Wwidth = width;
                gs->Wheight = height;
            }
        });

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

GraphicsSystem::GraphicsSystem(GLint w, GLint h) : window(nullptr), vao(0), vbo(0), Wwidth(w), Wheight(h), vp(0)
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
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int width, int height)
        {
            glViewport(0, 0, width, height);

            // Update stored window width/height in GraphicsSystem.
            auto* gs = static_cast<GraphicsSystem*>(glfwGetWindowUserPointer(win));
            if (gs) {
                gs->Wwidth = width;
                gs->Wheight = height;
            }
        });

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
    shader.CompileLinkValidate({
        {GL_VERTEX_SHADER, "../Engine/Graphics/vertices.vert"},
        {GL_FRAGMENT_SHADER, "../Engine/Graphics/fragment.frag"}
        });

    // Define vertex structure for a textured quad.
    struct Vertex
    {
        glm::vec3 pos;  // Position (x, y, z)
        glm::vec2 tex;  // Texture coordinates (u, v)
    };

    // Quad geometry (two triangles forming a square).
    // Coordinates are in NDC space, centered at origin.
    Vertex vertices[] = {
        {{-0.5f, -0.5f, 0.f}, {0.f, 0.f}}, // bottom-left
        {{ 0.5f, -0.5f, 0.f}, {1.f, 0.f}}, // bottom-right
        {{-0.5f,  0.5f, 0.f}, {0.f, 1.f}}, // top-left
        {{ 0.5f,  0.5f, 0.f}, {1.f, 1.f}}  // top-right
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
}

void GraphicsSystem::Update(float)
{
    if (!window)
        return; // Skip if no valid window.

    // === Clear Screen ===
    glClearColor(0.6f, 0.5f, 0.45f, 0.0f); // Background color (RGBA)
    glClear(GL_COLOR_BUFFER_BIT);          // Clear color buffer
    glLineWidth(2.f);                      // Set line thickness
    glPointSize(50.f);                     // Set point size (for debug points)

    // Use main shader program
    shader.Use();

    // Orthographic projection: origin at bottom-left, size = window width/height
    glm::mat4 projection = glm::ortho(0.0f, (float)Wwidth, 0.0f, (float)Wheight);

    // Default camera at (0,0)
    glm::vec3 camPos(0.0f);

    // Center camera on the "Hero" object if it exists
    GameObject* player = GameObjectFactory::Instance().FindGameObject("Hero");
    if (player)
    {
        auto* transform = player->GetComponent<TransformComponent>();
        if (transform)
        {
            float px{ 0 }, py{ 0 };
            transform->GetPosition(px, py);
            camPos.x = px - Wwidth / 2.0f;  // shift camera so Hero is centered
            camPos.y = py - Wheight / 2.0f;
        }
    }

    // Camera transform: translate world relative to camPos
    glm::mat4 view = glm::translate(glm::mat4(1.0f), -camPos);
    vp = projection * view; // Final View-Projection matrix

    // === Render All Active GameObjects ===
    auto& gameObjects = GameObjectFactory::Instance().GetAllGameObjects();
    //int count = 0;
    for (auto& obj : gameObjects)
    {
        if (!obj->IsActive())
            continue;

        auto* transform = obj->GetComponent<TransformComponent>();
        if (!transform)
            continue;

        if (auto* rend = obj->GetComponent<RenderComponent>())
        {
            switch (rend->GetShapeType())
            {
                // === Draw Sprite / Shape (Square or Circle) ===
            case RenderComponent::ShapeType::Square:
            case RenderComponent::ShapeType::Circle:
            {
                float x, y, sx, sy;
                transform->GetPosition(x, y);
                transform->GetScale(sx, sy);
                float rot = transform->GetRotation();

                // Debug mode: overlay custom debug shapes
                // Draw dubug mode shapes
                if (debugMode)
                {
                    auto& debugShapes = CORE->GetDebugSystem()->GetShapes();
                    for (auto& shape : debugShapes)
                    {
                        switch (shape.type)
                        {
                        case DebugSystem::DebugShape::LINE:
                            DrawLine(shape.p1, shape.p2, shape.color, vp);
                            break;
                        case DebugSystem::DebugShape::RECT:
                            DrawRectOutline(shape.p1.x, shape.p1.y, shape.p2.x, shape.p2.y, shape.color, vp);
                            break;
                        case DebugSystem::DebugShape::CIRCLE:
                            DrawCircleOutline(shape.p1.x, shape.p1.y, shape.radius, shape.segments, shape.color, vp);
                            break;
                        case DebugSystem::DebugShape::POINT:
                            DrawPoint(shape.p1, shape.color, vp);
                        }
                    }
                }

                // === Pass uniforms to shader ===
                shader.SetUniform("uIsLine", 0);  // Sprite, not line
                shader.SetUniform("uColor", glm::vec4(rend->GetColor())); // Tint color
                shader.SetUniform("uTiling", glm::vec2(1.0f));            // Default tiling
                shader.SetUniform("uIsCircle",
                    rend->GetShapeType() == RenderComponent::ShapeType::Circle ? 1 : 0);

                glm::vec2 uvScale = glm::vec2(1.0f);
                glm::vec2 uvOffset = glm::vec2(0.0f);

                // Check if object has an animator (spritesheet)
                if (auto* animator = obj->GetComponent<Spritesheet>())
                {
                    uvScale = animator->GetUVScale();   // Scale (1/Cols, 1/Rows)
                    uvOffset = animator->GetUVOffset(); // Offset for current frame
                }

                shader.SetUniform("uTiling", uvScale);
                shader.SetUniform("uTexOffset", uvOffset);

                // === Bind texture if available ===
                if (rend->GetTextureID() > 0)
                {
                    shader.SetUniform("uHasTexture", 1);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, rend->GetTextureID());
                    shader.SetUniform("uTexture", 0);
                }
                else
                {
                    shader.SetUniform("uHasTexture", 0);
                    glBindTexture(GL_TEXTURE_2D, 0);
                };

                // === Model transform (position, rotation, scale) ===
                glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
                model = glm::rotate(model, rot, glm::vec3(0, 0, 1));
                model = glm::scale(model, glm::vec3(sx, sy, 1.0f));

                // Send final MVP matrix to shader
                shader.SetUniform("uMVP", vp * model);

                    glBindVertexArray(vao);
                    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                    if (debugMode) {
                        if (rend->GetShapeType() == RenderComponent::ShapeType::Circle) {
                            DrawCircleOutline(x, y, static_cast<float>(sx*0.5), static_cast<int>(sy), { 0,1,0,1 }, vp);
                        }
                        else {
                            DrawRectOutline(x, y, sx, sy, { 0,1,0,1 }, vp);
                        }
                    }
                break;
            }

            // === Draw Line ===
            case RenderComponent::ShapeType::Line:
            {
                auto line = rend->GetLine();
                DrawLine(line.start, line.end, line.color, vp);
                break;
            }

            // === Draw Point ===
            case RenderComponent::ShapeType::Point:
            {
                auto point = rend->GetPoint();
                DrawPoint(point.pos, point.color, vp);
                break;
            }
            }
        }
    }
}

// Draw a line between two points in world space
void GraphicsSystem::DrawLine(const glm::vec2& start, const glm::vec2& end,
    const glm::vec4& color, const glm::mat4& viewProjection)
{
    // Tell shader this is a line draw
    shader.SetUniform("uIsLine", 1);
    shader.SetUniform("uColor", color);
    shader.SetUniform("uHasTexture", 0);
    shader.SetUniform("uIsCircle", 0);

    // Define 2 vertices for the line
    glm::vec3 verts[2] = {
        {start.x, start.y, 0.0f},
        {end.x,   end.y,   0.0f} };

    // Create temporary VAO/VBO for this line
    GLuint tempVBO, tempVAO;
    glGenVertexArrays(1, &tempVAO);
    glGenBuffers(1, &tempVBO);

    glBindVertexArray(tempVAO);
    glBindBuffer(GL_ARRAY_BUFFER, tempVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);

    // Attribute 0 = vertex position (vec3 in shader)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    // Send MVP matrix
    shader.SetUniform("uMVP", viewProjection);

    // Draw the line
    glDrawArrays(GL_LINES, 0, 2);

    // Cleanup temp objects
    glDeleteBuffers(1, &tempVBO);
    glDeleteVertexArrays(1, &tempVAO);
}

// Draw a single point in world space
void GraphicsSystem::DrawPoint(const glm::vec2& ppos,
    const glm::vec4& color, const glm::mat4& viewProjection)
{
    // Tell shader this is NOT a line
    shader.SetUniform("uIsLine", 0);
    shader.SetUniform("uIsCircle", 0);   // keep consistent with line draw
    shader.SetUniform("uColor", color);
    shader.SetUniform("uHasTexture", 0);

    // Position of the point (use vec3 to match shader input = vec3)
    glm::vec3 pos = { ppos.x, ppos.y, 0.0f };

    // Create temporary VAO/VBO for this point
    GLuint tempVBO, tempVAO;
    glGenVertexArrays(1, &tempVAO);
    glGenBuffers(1, &tempVBO);

    glBindVertexArray(tempVAO);
    glBindBuffer(GL_ARRAY_BUFFER, tempVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pos), &pos, GL_DYNAMIC_DRAW);

    // Attribute 0 = vertex position (vec3 in shader)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

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
    const glm::vec4& color, const glm::mat4& viewProjection)
{
    // Half extents (distance from center to edge)
    float halfW = w / 2;
    float halfH = h / 2;

    // Compute 4 corner points of the rectangle
    glm::vec2 p1(x - halfW, y - halfH); // bottom-left
    glm::vec2 p2(x + halfW, y - halfH); // bottom-right
    glm::vec2 p3(x + halfW, y + halfH); // top-right
    glm::vec2 p4(x - halfW, y + halfH); // top-left

    // Connect corners with lines (in order)
    DrawLine(p1, p2, color, viewProjection);
    DrawLine(p2, p3, color, viewProjection);
    DrawLine(p3, p4, color, viewProjection);
    DrawLine(p4, p1, color, viewProjection);
}

// Draw a circle outline centered at (cx, cy)
// r = radius, segments = how many line segments to approximate the circle
void GraphicsSystem::DrawCircleOutline(float cx, float cy, float r, int segments,
    const glm::vec4& color, const glm::mat4& viewProjection)
{
    // Clamp to a minimum number of segments (otherwise circle looks too blocky)
    if (segments < 8)
        segments = 8;

    // Angle step per segment (full 360° / segments)
    float step = glm::two_pi<float>() / segments;

    // Generate connected line segments around the circle
    for (int i = 0; i < segments; i++)
    {
        float theta1 = step * i;
        float theta2 = step * (i + 1);

        // Two consecutive points on the circle’s circumference
        glm::vec2 p1 = { cx + r * cos(theta1), cy + r * sin(theta1) };
        glm::vec2 p2 = { cx + r * cos(theta2), cy + r * sin(theta2) };

        // Draw line between them
        DrawLine(p1, p2, color, viewProjection);
    }
}


std::string GraphicsSystem::GetName()
{
    return "Graphics System";
}

// Load a 2D texture from a file using stb_image
// Returns the OpenGL texture ID (GLuint), or 0 if loading fails
GLuint GraphicsSystem::LoadTexture(const char* filepath)
{
    // Tell stb_image to flip images vertically (so origin is bottom-left, like OpenGL expects)
    stbi_set_flip_vertically_on_load(true);

    int width, height, channels;
    // Force load with 4 channels (RGBA) regardless of source image format
    unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
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
        // RenderComponent(owner, ShapeType type, const glm::vec4& color)
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

    return clonedComponent;
}