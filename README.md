I led an 8-person team building a C++ 2D engine with C# scripting and replay system and component system.

Case studies
⭐ Flagship	Custom 2D Game Engine	Engine architecture, team leadership Engine and game Repo · Demo build
⏱ Constrained	Original C/C++ Game Performance  · Repo
🛠 Skill proof	Unity Procedural & Editor Tools	Tools programming for designers	Tooling	Repo
1. Flagship — Custom 2D Game Engine (C++)

Tech Lead, team of 8 · C++, C# · Game Engine Architecture module · for about 6 months 

Problem: The problem for this was set in the base of the game engine architecture and 
Constraint: Eight people working in one codebase, a fixed semester deadlines.
What I tested/did: C++/C# scripting with mono so the gameplay can be coded in c#. Input recording and playback so a can be replayed and for the VS ghost system. Game Engine Architecture was done and tested by me
What failed: For the scripting was finding the way to even convert from c# to c++, thinking which recording method would be good for the game like input-based, position based and so on. as for the architecture was which system (like ECS,OOP, or something else) would i use and good for the game.

What I changed: 
C++/C# scripting
For the scripting wise i chose mono instead of .net for the scripting as would be simpler to implement and have added APIs to the game engine so i added a app domain and and creating internal call would be easier than .net's setup of hostfxr a library and get function pointers.

Replay System
It was built choosing to just record they player's position and animation states for every frame and stored in json it was done as my game there was not a lot of moving obstacles, i created a VS ghost mode that is powered by said replay system where you can challenge the player with the best time so the ghost is a playback of the players best run or depending on what they chosen therefore a same path on the same timing in every run. Because the world is in the same state on every frame as it was during recording, the ghost's positions are always valid, and its movement stays consistent with the level.

Game Engine Architecture 
With 0 experience when creating the architecture, my team and i was discuss which system should we used for the game engine itself, it was either ECS or OOP programming i had to research and see which is suited for our engine, with ECS in the long run if my team and i want to create more game objects like in the thousands and maybe if we turn it to a 3d project and can be hard to setup with no experience and for OOP programming is just too much to handle and can be confusing at times, so i came up with another solution a 3rd option which is a normal component system where has a factory which produces the objects and entities and a blueprint to be sent to the factory to be produced, its simpler than ecs and not as confusing as OOP.
