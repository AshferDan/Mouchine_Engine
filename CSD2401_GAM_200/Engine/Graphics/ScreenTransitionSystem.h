#pragma once
#include <functional>
#include <string>
#include <vector>
#include "System.h"   // ISystem base + IMessageHandler

/* Start Header ************************************************************************/
/*!
\file       ScreenTransition.h
\author     Guan Shao Jun [100%]
\par        shaojun.g@digipen.edu
\date       March 10 2026

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

enum class TransitionState {
    Idle,
    IrisClose,      // circle shrinks  black
    LoadScreen,     // GIF plays while scene loads
    IrisOpen        // circle grows  reveals new scene
};

class ScreenTransitionSystem : public ISystem {
public:
    ScreenTransitionSystem() = default;
    ~ScreenTransitionSystem() override;

    //  ISystem interface 
    void        Initialize()            override;
    void        Update(float timeslice) override;
    std::string GetName()               override { return "ScreenTransitionSystem"; }

    //  Public API 

    // Call this to kick off a transition.
    //   gifPath    : path to your .gif loading animation
    //   onMidpoint : fired when screen is fully black  safe to swap scenes here
    void StartTransition(const std::string& gifPath, std::function<void()> onMidpoint);

    // Call this LAST in your render loop (after GraphicsSystem::Render)
    void Render();

    bool IsTransitioning() const { return m_State != TransitionState::Idle; }

    TransitionState GetState() const { return m_State; }

    void StartLoadScreenOnly(const std::string& gifPath,
        std::function<void()> onMidpoint);

private:
    void InitGL();
    void LoadGif(const std::string& path);
    void AdvanceGif(float deltaTime);
    void RenderIris(float radius);
    void RenderGif();

    //  State 
    TransitionState         m_State = TransitionState::Idle;
    float                   m_Timer = 0.0f;

    //  Iris timing (seconds) 
    static constexpr float  IRIS_CLOSE_DUR = 1.55f;
    static constexpr float  LOAD_MIN_DUR = 2.0f;   // minimum gif display time
    static constexpr float  IRIS_OPEN_DUR = 0.55f;

    //  Easing 
    float EaseInQuart(float t)  const { return t * t * t * t; }
    float EaseOutQuart(float t) const { float u = 1.0f - t; return 1.0f - u * u * u * u; }

    //  Scene callback 
    std::function<void()>   m_OnMidpoint;
    bool                    m_MidpointFired = false;

    //  OpenGL: iris shader 
    unsigned int            m_IrisShader = 0;
    unsigned int            m_QuadVAO = 0;
    unsigned int            m_QuadVBO = 0;
    int                     m_uRadius = -1;
    int                     m_uResolution = -1;
    int                     m_uCenter = -1;

    //  GIF playback (stb_image-based frame list) 
    struct GifFrame {
        unsigned int textureID = 0;
        float        delay = 0.1f;   // seconds
    };
    std::vector<GifFrame>   m_GifFrames;
    int                     m_CurrentFrame = 0;
    float                   m_FrameTimer = 0.0f;

    //  OpenGL: gif quad shader (simple textured) 
    unsigned int            m_GifShader = 0;
    int                     m_uGifTex = -1;
};