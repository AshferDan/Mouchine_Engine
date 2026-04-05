/* Start Header **************************************************************/
/*!
\file   SplashScreenSystem.h
\brief  Plays the DigiPen + game logo splash sequence on release startup.
        Renders directly to the default framebuffer (no scene loaded yet).
        Call Run() after engine Init() and before RequestSceneLoad().
*/
/* End Header ****************************************************************/
#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>

class GraphicsSystem;

class SplashScreenSystem
{
public:
    // logoHoldSecs – how long each logo is fully visible
    // fadeSecs     – how long each fade-in / fade-out takes
    SplashScreenSystem(GraphicsSystem* gfx,
        const std::string& schoolLogoPath,
        const std::string& gameLogoPath,
        float logoHoldSecs = 2.0f,
        float fadeSecs = 0.8f);

    ~SplashScreenSystem();

    void Initialize();

    // Blocking: runs its own mini-loop until both logos have played,
    // then returns. Call from main() before RequestSceneLoad().
    void Run();

private:
    enum class State
    {
        FadeInSchool,
        HoldSchool,
        FadeOutSchool,
        FadeInGame,
        HoldGame,
        FadeOutGame,
        Done
    };

    GraphicsSystem* m_gfx;
    std::string     m_schoolPath;
    std::string     m_gamePath;
    float           m_holdSecs;
    float           m_fadeSecs;

    GLuint  m_schoolTex = 0;
    GLuint  m_gameTex = 0;
    GLuint  m_quadVAO = 0;
    GLuint  m_quadVBO = 0;
    GLuint  m_progHandle = 0;   // raw GL program, no GLSLShader wrapper

    State   m_state = State::FadeInSchool;
    float   m_stateTime = 0.f;
    double  m_prevTime = 0.0;

    void BuildQuad(float halfW, float halfH);
    void BuildShader();
    void DrawLogo(GLuint tex, float alpha, int imgW, int imgH, int fbW, int fbH);
    void AdvanceState(float dt);

    int m_schoolW = 0, m_schoolH = 0;  // pixel dimensions of school logo
    int m_gameW = 0, m_gameH = 0;  // pixel dimensions of game logo
};