
#include "GraphicsSystem.h"
#include "Animator.hpp"

const char* configFile = "../Logs/config.txt";


GraphicsSystem::GraphicsSystem() : window(nullptr),vao(0), vbo(0),Wwidth(800),Wheight(600),tex(0) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return;
    }
    // Simple fixed window size for now
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(Wwidth, Wheight, "Game Engine", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return;
    }
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int width, int height) {
        glViewport(0, 0, width, height);

        // optional: store width/height in GraphicsSystem if you use them later
        auto* gs = static_cast<GraphicsSystem*>(glfwGetWindowUserPointer(win));
        if (gs) {
            gs->Wwidth = width;
            gs->Wheight = height;
        }
        });

    glfwMakeContextCurrent(window);
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
    Wwidth = fbWidth;
    Wheight = fbHeight;
}

GraphicsSystem::GraphicsSystem(GLint w, GLint h) : window(nullptr), vao(0), vbo(0), Wwidth(w), Wheight(h) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return;
    }
    // Simple fixed window size for now
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(Wwidth, Wheight, "Game Engine", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return;
    }
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int width, int height) {
        glViewport(0, 0, width, height);

        // optional: store width/height in GraphicsSystem if you use them later
        auto* gs = static_cast<GraphicsSystem*>(glfwGetWindowUserPointer(win));
        if (gs) {
            gs->Wwidth = width;
            gs->Wheight = height;
        }
        });
    glfwMakeContextCurrent(window);
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
    Wwidth = fbWidth;
    Wheight = fbHeight;
}

GraphicsSystem::~GraphicsSystem() {
    if (window) {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}

void GraphicsSystem::Initialize() {

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Compile/link shaders
    shader.CompileLinkValidate({
        {GL_VERTEX_SHADER, "../Engine/Graphics/vertices.vert"},
        {GL_FRAGMENT_SHADER, "../Engine/Graphics/fragment.frag"}
        });

    // Quad geometry
    struct Vertex {
        glm::vec3 pos;
        glm::vec2 tex;
    };

    Vertex vertices[] = {
        {{-0.5f, -0.5f, 0.f}, {0.f, 0.f}}, // bottom-left
        {{ 0.5f, -0.5f, 0.f}, {1.f, 0.f}}, // bottom-right
        {{-0.5f,  0.5f, 0.f}, {0.f, 1.f}}, // top-left
        {{ 0.5f,  0.5f, 0.f}, {1.f, 1.f}}  // top-right
    };

    // VAO + VBO
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glCreateBuffers(1, &vbo);
    glNamedBufferStorage(vbo, sizeof(vertices), vertices, 0);
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));

    // Position attribute
    glEnableVertexArrayAttrib(vao, 0);
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
    glVertexArrayAttribBinding(vao, 0, 0);

    //Texcoord attribute
    glEnableVertexArrayAttrib(vao, 1);
    glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, tex));
    glVertexArrayAttribBinding(vao, 1, 0);
}

void GraphicsSystem::Update(float dt) {
    if (!window) return;

    glClearColor(0.2f, 0.5f, 0.5f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glLineWidth(2.f);
    glPointSize(50.f);

    shader.Use();

    float aspect = static_cast<float>(Wwidth) / static_cast<float>(Wheight);
    glm::mat4 projection = glm::ortho(0.0f, (float)Wwidth, 0.0f, (float)Wheight);

    glm::vec3 camPos(0.0f);
    GameObject* player = GameObjectFactory::Instance().FindGameObject("Hero");
    if (player) {
        auto* transform = player->GetComponent<TransformComponent>();
        if (transform) {
            float px{ 0 }, py{ 0 };
            transform->GetPosition(px, py);
            camPos.x = px - Wwidth / 2.0f;
            camPos.y = py - Wheight / 2.0f;
        }
    }

    glm::mat4 view = glm::translate(glm::mat4(1.0f), -camPos);
    glm::mat4 vp = projection * view;

    auto& gameObjects = GameObjectFactory::Instance().GetAllGameObjects();
    int count = 0;
    for (auto& obj : gameObjects) {
        if (!obj->IsActive()) {
            continue;
        };

        auto* transform = obj->GetComponent<TransformComponent>();
        if (!transform) {
            continue;
        }

        if (auto* rend = obj->GetComponent<RenderComponent>()) {
            switch (rend->GetShapeType()) {
            case RenderComponent::ShapeType::Square:
            case RenderComponent::ShapeType::Circle:
            {
                shader.SetUniform("uColor", glm::vec4(rend->GetColor()));
                shader.SetUniform("uTiling", glm::vec2(1.0f));
                shader.SetUniform("uIsCircle", rend->GetShapeType() == RenderComponent::ShapeType::Circle ? 1 : 0);
                shader.SetUniform("uIsLine", 0);
                glm::vec2 uvScale = glm::vec2(1.0f);
                glm::vec2 uvOffset = glm::vec2(0.0f);

                if (auto* animator = obj->GetComponent<Spritesheet>()) {
                    uvScale = animator->GetUVScale();     // Scale (1/Cols, 1/Rows)
                    uvOffset = animator->GetUVOffset();   // Current frame's offset
                }

                shader.SetUniform("uTiling", uvScale);
                shader.SetUniform("uTexOffset", uvOffset);


                if (rend->GetTextureID() > 0) {
                    shader.SetUniform("uHasTexture", 1);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, rend->GetTextureID());
                    shader.SetUniform("uTexture", 0);
                }
                else {
                    shader.SetUniform("uHasTexture", 0);
                    glBindTexture(GL_TEXTURE_2D, 0);
                };

                float x, y, sx, sy;
                transform->GetPosition(x, y);
                transform->GetScale(sx, sy);
                float rot = transform->GetRotation();

                glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
                model = glm::rotate(model, rot, glm::vec3(0, 0, 1));
                model = glm::scale(model, glm::vec3(sx, sy, 1.0f));

                shader.SetUniform("uMVP", vp * model);

                glBindVertexArray(vao);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                break;
            }

            case RenderComponent::ShapeType::Line:
            {
                auto line = rend->GetLine();
                DrawLine(line.start, line.end, line.color, vp);
                //DrawLine({ 0,0 }, { Wwidth, Wheight }, { 1,0,0,1 }, vp);
                break;
            }

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

void GraphicsSystem::DrawLine(const glm::vec2& start, const glm::vec2& end,
    const glm::vec4& color, const glm::mat4& vp) {
    shader.SetUniform("uIsLine", 1);
    shader.SetUniform("uColor", color);
    shader.SetUniform("uHasTexture", 0);
    shader.SetUniform("uIsCircle", 0);

    glm::vec3 verts[2] = {
        { start.x, start.y, 0.0f },
        { end.x,   end.y,   0.0f }
    };

    GLuint tempVBO, tempVAO;
    glGenVertexArrays(1, &tempVAO);
    glGenBuffers(1, &tempVBO);

    glBindVertexArray(tempVAO);
    glBindBuffer(GL_ARRAY_BUFFER, tempVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    shader.SetUniform("uMVP", vp);

    glDrawArrays(GL_LINES, 0, 2);

    glDeleteBuffers(1, &tempVBO);
    glDeleteVertexArrays(1, &tempVAO);
}

void GraphicsSystem::DrawPoint(const glm::vec2& ppos,
    const glm::vec4& color, const glm::mat4& vp) {
    shader.SetUniform("uIsLine", 0);
    shader.SetUniform("uColor", color);
    shader.SetUniform("uHasTexture", 0);

    glm::vec2 pos = ppos;

    GLuint tempVBO, tempVAO;
    glGenVertexArrays(1, &tempVAO);
    glGenBuffers(1, &tempVBO);

    glBindVertexArray(tempVAO);
    glBindBuffer(GL_ARRAY_BUFFER, tempVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pos), &pos, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);

    shader.SetUniform("uMVP", vp);

    glDrawArrays(GL_POINTS, 0, 1);

    glDeleteBuffers(1, &tempVBO);
    glDeleteVertexArrays(1, &tempVAO);
}

void GraphicsSystem::DrawRectOutline(float x, float y, float w, float h,
    const glm::vec4& color, const glm::mat4& vp) {
    glm::vec2 p1(x, y);
    glm::vec2 p2(x + w, y);
    glm::vec2 p3(x + w, y + h);
    glm::vec2 p4(x, y + h);

    // 4 sides
    DrawLine(p1, p2, color, vp);
    DrawLine(p2, p3, color, vp);
    DrawLine(p3, p4, color, vp);
    DrawLine(p4, p1, color, vp);
}

void GraphicsSystem::DrawCircleOutline(float cx, float cy, float r, int segments,
    const glm::vec4& color, const glm::mat4& vp) {
    if (segments < 8) segments = 8; // minimum for circle shape

    float step = glm::two_pi<float>() / segments;
    for (int i = 0; i < segments; i++) {
        float theta1 = step * i;
        float theta2 = step * (i + 1);

        glm::vec2 p1 = { cx + r * cos(theta1), cy + r * sin(theta1) };
        glm::vec2 p2 = { cx + r * cos(theta2), cy + r * sin(theta2) };

        DrawLine(p1, p2, color, vp);
    }
}

std::string GraphicsSystem::GetName() {
    return "Graphics System";
}

GLuint GraphicsSystem::LoadTexture(const char* filepath) {
    stbi_set_flip_vertically_on_load(true);

    int width, height, channels;
    unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
    if (!data) {
        std::cerr << "Failed to load texture: " << filepath << std::endl;
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return texture;
}
std::unique_ptr<IComponent> RenderComponent::Clone(GameObject* NewClone) const {

    // 1. Determine which constructor to use based on the original component's state.
    std::unique_ptr<RenderComponent> clonedComponent;

    // Check if the component uses a texture (textureID is non-zero)
    if (this->textureID != 0) {
        // Use the constructor for textured objects: 
        // RenderComponent(owner, GLuint texID)
        clonedComponent = std::make_unique<RenderComponent>(NewClone, this->textureID);
    }
    else {
        // Use the constructor for shape/color: 
        // RenderComponent(owner, ShapeType type, const glm::vec4& color)
        clonedComponent = std::make_unique<RenderComponent>(NewClone, this->shape, this->color);
    }

    // 2. Clone specialized data for Line or Point types.
    if (this->shape == RenderComponent::ShapeType::Line) {
        // Copy the line data using the public setter
        clonedComponent->SetLine(this->line.start, this->line.end);
    }
    else if (this->shape == RenderComponent::ShapeType::Point) {
        // Copy the point data using the public setter
        clonedComponent->SetPoint(this->point.pos, this->point.size);
    }

    // 3. Clone the enabled state.
    clonedComponent->SetEnabled(this->IsEnabled());

    return clonedComponent;
}