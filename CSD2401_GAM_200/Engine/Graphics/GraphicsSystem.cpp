#define STB_IMAGE_IMPLEMENTATION
#include "GraphicsSystem.h"
#include "stb_image.h"


GraphicsSystem::GraphicsSystem() : window(nullptr), vao(0), vbo(0) {}

GraphicsSystem::~GraphicsSystem() {
    if (window) {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}

void GraphicsSystem::Initialize() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return;
    }

    // Simple fixed window size for now
    window = glfwCreateWindow(800, 600, "Game Engine", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);

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
    glPointSize(5.f);

    shader.Use();

    glm::mat4 projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f);

    glm::vec3 camPos(0.0f);
    GameObject* player = GameObjectFactory::Instance().FindGameObject("Hero");
    if (player) {
        auto* transform = player->GetComponent<TransformComponent>();
        if (transform) {
            float px{ 0 }, py{ 0 };
            transform->GetPosition(px, py);
            camPos.x = px + 2.5f;
            camPos.y = py;
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
                shader.SetUniform("uColor", line.color);
                shader.SetUniform("uHasTexture", 0);

                glm::vec2 verts[2] = { line.start, line.end };

                GLuint tempVBO, tempVAO;
                glGenVertexArrays(1, &tempVAO);
                glGenBuffers(1, &tempVBO);

                glBindVertexArray(tempVAO);
                glBindBuffer(GL_ARRAY_BUFFER, tempVBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);

                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);

                shader.SetUniform("uMVP", vp);

                glDrawArrays(GL_LINES, 0, 2);

                glDeleteBuffers(1, &tempVBO);
                glDeleteVertexArrays(1, &tempVAO);

                break;
            }

            case RenderComponent::ShapeType::Point:
            {
                auto point = rend->GetPoint();
                shader.SetUniform("uColor", point.color);
                shader.SetUniform("uHasTexture", 0);

                glm::vec2 pos = point.pos;

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
                break;
            }
            }
        }
    }
        glfwSwapBuffers(window);
        glfwPollEvents();
    
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
