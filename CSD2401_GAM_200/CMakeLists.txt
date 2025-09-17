# ======================= Project Configuration =========================
cmake_minimum_required(VERSION 3.13)  # Updated minimum version
cmake_policy(VERSION 3.13)
project(GAM_200_AME)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(ImportDependencies)
importDependencies()

add_definitions(-DGLEW_STATIC)
add_definitions(-DUSE_CSD3151_AUTOMATION=0)

# ======================= Directories =============================
set(Audio_DIR         ./Build/Audio)
set(Camera_DIR        ./Build/Camera)
set(Collision_DIR     ./Build/Collision)
set(GameManager_DIR   ./Build/GameManager)
set(Graphics_DIR      ./Build/Graphics)
set(LevelEditor_DIR   ./Build/LevelEditor)
set(Logic_DIR         ./Build/Logic)
set(Main_DIR          ./Build/Main)
set(Physics_DIR       ./Build/Physics)
set(Serialization_DIR ./Build/Serialization)
set(Transform_DIR     ./Build/Transform)
set(Input_DIR 	      ./Build/Input)
set(Extern_DIR        ./Build/Extern)
set(Factory_DIR       ./Build/Factory)
set(Core_DIR          ./Build/Core)

# ======================= File Globbing ============================
file(GLOB_RECURSE Audio_SRC        "${Audio_DIR}/*.cpp")
file(GLOB_RECURSE Audio_HEAD       "${Audio_DIR}/*.h" "${Audio_DIR}/*.hpp")

file(GLOB_RECURSE Camera_SRC       "${Camera_DIR}/*.cpp")
file(GLOB_RECURSE Camera_HEAD      "${Camera_DIR}/*.h" "${Camera_DIR}/*.hpp")

file(GLOB_RECURSE Collision_SRC    "${Collision_DIR}/*.cpp")
file(GLOB_RECURSE Collision_HEAD   "${Collision_DIR}/*.h" "${Collision_DIR}/*.hpp")

file(GLOB_RECURSE GameManager_SRC  "${GameManager_DIR}/*.cpp")
file(GLOB_RECURSE GameManager_HEAD "${GameManager_DIR}/*.h" "${GameManager_DIR}/*.hpp")

file(GLOB_RECURSE Graphics_SRC     "${Graphics_DIR}/*.cpp")
file(GLOB_RECURSE Graphics_HEAD    "${Graphics_DIR}/*.h" "${Graphics_DIR}/*.hpp")

file(GLOB_RECURSE LevelEditor_SRC  "${LevelEditor_DIR}/*.cpp")
file(GLOB_RECURSE LevelEditor_HEAD "${LevelEditor_DIR}/*.h" "${LevelEditor_DIR}/*.hpp")

file(GLOB_RECURSE Logic_SRC        "${Logic_DIR}/*.cpp")
file(GLOB_RECURSE Logic_HEAD       "${Logic_DIR}/*.h" "${Logic_DIR}/*.hpp")

file(GLOB_RECURSE Main_SRC         "${Main_DIR}/*.cpp")
file(GLOB_RECURSE Main_HEAD        "${Main_DIR}/*.h" "${Main_DIR}/*.hpp")

file(GLOB_RECURSE Physics_SRC      "${Physics_DIR}/*.cpp")
file(GLOB_RECURSE Physics_HEAD     "${Physics_DIR}/*.h" "${Physics_DIR}/*.hpp")

file(GLOB_RECURSE Serialization_SRC "${Serialization_DIR}/*.cpp")
file(GLOB_RECURSE Serialization_HEAD "${Serialization_DIR}/*.h" "${Serialization_DIR}/*.hpp")

file(GLOB_RECURSE Transform_SRC    "${Transform_DIR}/*.cpp")
file(GLOB_RECURSE Transform_HEAD   "${Transform_DIR}/*.h" "${Transform_DIR}/*.hpp")

file(GLOB_RECURSE Extern_SRC    "${Extern_DIR}/*.cpp")
file(GLOB_RECURSE Extern_HEAD   "${Extern_DIR}/*.h" "${Extern_DIR}/*.hpp")

file(GLOB_RECURSE Input_SRC    "${Input_DIR}/*.cpp")
file(GLOB_RECURSE Input_HEAD   "${Input_DIR}/*.h" "${Input_DIR}/*.hpp")

file(GLOB_RECURSE Factory_SRC    "${Factory_DIR}/*.cpp")
file(GLOB_RECURSE Factory_HEAD   "${Factory_DIR}/*.h" "${Factory_DIR}/*.hpp")

file(GLOB_RECURSE Core_SRC    "${Core_DIR}/*.cpp")
file(GLOB_RECURSE Core_HEAD   "${Core_DIR}/*.h" "${Core_DIR}/*.hpp")



# ======================= Source Groups for Visual Studio ============
source_group("Audio\\Header Files" FILES ${Audio_HEAD})
source_group("Audio\\Source Files" FILES ${Audio_SRC})

source_group("Camera\\Header Files" FILES ${Camera_HEAD})
source_group("Camera\\Source Files" FILES ${Camera_SRC})

source_group("Collision\\Header Files" FILES ${Collision_HEAD})
source_group("Collision\\Source Files" FILES ${Collision_SRC})

source_group("GameManager\\Header Files" FILES ${GameManager_HEAD})
source_group("GameManager\\Source Files" FILES ${GameManager_SRC})

source_group("Graphics\\Header Files" FILES ${Graphics_HEAD})
source_group("Graphics\\Source Files" FILES ${Graphics_SRC})

source_group("LevelEditor\\Header Files" FILES ${LevelEditor_HEAD})
source_group("LevelEditor\\Source Files" FILES ${LevelEditor_SRC})

source_group("Logic\\Header Files" FILES ${Logic_HEAD})
source_group("Logic\\Source Files" FILES ${Logic_SRC})

source_group("Main\\Header Files" FILES ${Main_HEAD})
source_group("Main\\Source Files" FILES ${Main_SRC})

source_group("Physics\\Header Files" FILES ${Physics_HEAD})
source_group("Physics\\Source Files" FILES ${Physics_SRC})

source_group("Serialization\\Header Files" FILES ${Serialization_HEAD})
source_group("Serialization\\Source Files" FILES ${Serialization_SRC})

source_group("Transform\\Header Files" FILES ${Transform_HEAD})
source_group("Transform\\Source Files" FILES ${Transform_SRC})

source_group("Input\\Header Files" FILES ${Input_HEAD})
source_group("Input\\Source Files" FILES ${Input_SRC})

source_group("Extern\\Header Files" FILES ${Extern_HEAD})
source_group("Extern\\Source Files" FILES ${Extern_SRC})

source_group("Core\\Header Files" FILES ${Core_HEAD})
source_group("Core\\Source Files" FILES ${Core_SRC})

source_group("Factory\\Header Files" FILES ${Factory_HEAD})
source_group("Factory\\Source Files" FILES ${Factory_SRC})

# ======================= Collect All Sources =========================
set(ALL_SOURCES
    ${Audio_SRC}
    ${Camera_SRC}
    ${Collision_SRC}
    ${GameManager_SRC}
    ${Graphics_SRC}
    ${LevelEditor_SRC}
    ${Logic_SRC}
    ${Main_SRC}
    ${Physics_SRC}
    ${Serialization_SRC}
    ${Transform_SRC}
    ${Extern_SRC}
    ${Input_SRC}
    ${Factory_SRC}
    ${Core_SRC}
)

set(ALL_HEADERS
    ${Audio_HEAD}
    ${Camera_HEAD}
    ${Collision_HEAD}
    ${GameManager_HEAD}
    ${Graphics_HEAD}
    ${LevelEditor_HEAD}
    ${Logic_HEAD}
    ${Main_HEAD}
    ${Physics_HEAD}
    ${Serialization_HEAD}
    ${Transform_HEAD}
    ${Extern_HEAD}
    ${Input_HEAD}
    ${Factory_HEAD}
    ${Core_HEAD}
)

# ======================= Executable =========================
add_executable(${CMAKE_PROJECT_NAME}
    ${ALL_SOURCES}
    ${ALL_HEADERS}
)

# ======================= Set C++ Standard ===========================
set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

# ======================= Link Libraries =============================
target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE glfw
    PRIVATE libglew_static
    PRIVATE imgui
    PRIVATE nlohmann_json::nlohmann_json 
    #PRIVATE freetype
)

# ======================= Platform-specific Libraries ================
if(WIN32)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE opengl32)
elseif(APPLE)
    find_library(OPENGL_LIBRARY OpenGL)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE 
        ${OPENGL_LIBRARY} 
        ${COCOA_LIBRARY} 
        ${IOKIT_LIBRARY} 
        ${COREVIDEO_LIBRARY}
    )
elseif(UNIX)
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE 
        OpenGL::GL 
        ${X11_LIBRARIES} 
        ${CMAKE_DL_LIBS}
    )
endif()

# ======================= Compiler Warnings ==========================
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall ${DisableWarnings})
elseif (MSVC)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W3 /WX-)
endif()

# ======================= Include Directories ========================
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${Audio_DIR}
    ${Camera_DIR}
    ${Collision_DIR}
    ${GameManager_DIR}
    ${Graphics_DIR}
    ${LevelEditor_DIR}
    ${Logic_DIR}
    ${Main_DIR}
    ${Physics_DIR}
    ${Serialization_DIR}
    ${Transform_DIR}
    ${Extern_DIR}
    ${Input_DIR}
    ${Core_DIR}
    ${Factory_DIR}
)