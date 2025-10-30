#/* Start Header ************************************************************************/
#\file       CMakeLists.txt
#\author     Jonathan Fam Qian Wei [100% - 309 lines]
#\par        Fam.q@digipen.edu
#\date       October, 2, 2025

#Copyright (C) 2025 DigiPen Institute of Technology.
#Reproduction or disclosure of this file or its contents
#without the prior written consent of DigiPen Institute of
#Technology is prohibited.
#/* End Header **************************************************************************/


# ======================= Project Configuration =========================
cmake_minimum_required(VERSION 3.15)
#cmake_policy(VERSION 2.7...3.5)

# Set policy for imported targets
if(POLICY CMP0111)
    cmake_policy(SET CMP0111 NEW)
endif()

project(GAM_200_AME)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(ImportDependencies)
importDependencies()

add_definitions(-DUSE_CSD3151_AUTOMATION=0)

# ======================= Directories =============================
# Game directory - contains Main.cpp and Main.h only
set(Game_DIR          ./Game)

# Engine directories - all other engine systems
set(Audio_DIR         ./Engine/Audio)
set(Camera_DIR        ./Engine/Camera)
set(Collision_DIR     ./Engine/Collision)
set(GameManager_DIR   ./Engine/GameManager)
set(Graphics_DIR      ./Engine/Graphics)
set(LevelEditor_DIR   ./Engine/LevelEditor)
set(Logic_DIR         ./Engine/Logic)
set(Physics_DIR       ./Engine/Physics)
set(Serialization_DIR ./Engine/Serialization)
set(Transform_DIR     ./Engine/Transform)
set(Input_DIR         ./Engine/Input)
set(Factory_DIR       ./Engine/Factory)
set(Core_DIR          ./Engine/Core)
set(Resource_DIR      ./Engine/ResourceManager)
set(Scripting_DIR     ./Engine/Scripting)


# External dependencies
set(Extern_DIR        ./Engine/Extern)

# ======================= File Globbing ============================
# Game files (Main.cpp and Main.h only)
file(GLOB Game_SRC        "${Game_DIR}/*.cpp")
file(GLOB Game_HEAD       "${Game_DIR}/*.h" "${Game_DIR}/*.hpp")

# Engine files
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

file(GLOB_RECURSE Physics_SRC      "${Physics_DIR}/*.cpp")
file(GLOB_RECURSE Physics_HEAD     "${Physics_DIR}/*.h" "${Physics_DIR}/*.hpp")

file(GLOB_RECURSE Serialization_SRC "${Serialization_DIR}/*.cpp")
file(GLOB_RECURSE Serialization_HEAD "${Serialization_DIR}/*.h" "${Serialization_DIR}/*.hpp")

file(GLOB_RECURSE Transform_SRC    "${Transform_DIR}/*.cpp")
file(GLOB_RECURSE Transform_HEAD   "${Transform_DIR}/*.h" "${Transform_DIR}/*.hpp")

file(GLOB_RECURSE Input_SRC        "${Input_DIR}/*.cpp")
file(GLOB_RECURSE Input_HEAD       "${Input_DIR}/*.h" "${Input_DIR}/*.hpp")

file(GLOB_RECURSE Factory_SRC      "${Factory_DIR}/*.cpp")
file(GLOB_RECURSE Factory_HEAD     "${Factory_DIR}/*.h" "${Factory_DIR}/*.hpp")

file(GLOB_RECURSE Core_SRC         "${Core_DIR}/*.cpp")
file(GLOB_RECURSE Core_HEAD        "${Core_DIR}/*.h" "${Core_DIR}/*.hpp")

file(GLOB_RECURSE Resource_SRC  "${Resource_DIR}/*.cpp")
file(GLOB_RECURSE Resource_HEAD "${Resource_DIR}/*.h" "${Resource_DIR}/*.hpp")

file(GLOB_RECURSE Scripting_SRC    "${Scripting_DIR}/*.cpp")
file(GLOB_RECURSE Scripting_HEAD    "${Scripting_DIR}/*.h" "${Scripting_DIR}/*.hpp")


# External files (if any cpp/h files exist in Extern)
file(GLOB_RECURSE Extern_SRC       "${Extern_DIR}/*.cpp")
file(GLOB_RECURSE Extern_HEAD      "${Extern_DIR}/*.h" "${Extern_DIR}/*.hpp")


# ======================= Collect Engine Sources =========================
set(ENGINE_SOURCES
    ${Audio_SRC}
    ${Camera_SRC}
    ${Collision_SRC}
    ${GameManager_SRC}
    ${Graphics_SRC}
    ${LevelEditor_SRC}
    ${Logic_SRC}
    ${Physics_SRC}
    ${Serialization_SRC}
    ${Transform_SRC}
    ${Input_SRC}
    ${Factory_SRC}
    ${Core_SRC}
    ${Extern_SRC}
    ${Resource_SRC}
    ${Scripting_SRC}
)

set(ENGINE_HEADERS
    ${Audio_HEAD}
    ${Camera_HEAD}
    ${Collision_HEAD}
    ${GameManager_HEAD}
    ${Graphics_HEAD}
    ${LevelEditor_HEAD}
    ${Logic_HEAD}
    ${Physics_HEAD}
    ${Serialization_HEAD}
    ${Transform_HEAD}
    ${Input_HEAD}
    ${Factory_HEAD}
    ${Core_HEAD}
    ${Extern_HEAD}
    ${Resource_HEAD}
    ${Scripting_HEAD}
)

# ======================= Create Engine Static Library ===============
add_library(Engine STATIC
    ${ENGINE_SOURCES}
    ${ENGINE_HEADERS}
)

# Set C++ standard for Engine
set_property(TARGET Engine PROPERTY CXX_STANDARD 20)
set_property(TARGET Engine PROPERTY CXX_STANDARD_REQUIRED ON)

# ======================= Source Groups for Engine Project ===========
source_group("Audio\\Source Files" FILES ${Audio_SRC})
source_group("Audio\\Header Files" FILES ${Audio_HEAD})

source_group("Camera\\Source Files" FILES ${Camera_SRC})
source_group("Camera\\Header Files" FILES ${Camera_HEAD})

source_group("Collision\\Source Files" FILES ${Collision_SRC})
source_group("Collision\\Header Files" FILES ${Collision_HEAD})

source_group("GameManager\\Source Files" FILES ${GameManager_SRC})
source_group("GameManager\\Header Files" FILES ${GameManager_HEAD})

source_group("Graphics\\Source Files" FILES ${Graphics_SRC})
source_group("Graphics\\Header Files" FILES ${Graphics_HEAD})

source_group("LevelEditor\\Source Files" FILES ${LevelEditor_SRC})
source_group("LevelEditor\\Header Files" FILES ${LevelEditor_HEAD})

source_group("Logic\\Source Files" FILES ${Logic_SRC})
source_group("Logic\\Header Files" FILES ${Logic_HEAD})

source_group("Physics\\Source Files" FILES ${Physics_SRC})
source_group("Physics\\Header Files" FILES ${Physics_HEAD})

source_group("Serialization\\Source Files" FILES ${Serialization_SRC})
source_group("Serialization\\Header Files" FILES ${Serialization_HEAD})

source_group("Transform\\Source Files" FILES ${Transform_SRC})
source_group("Transform\\Header Files" FILES ${Transform_HEAD})

source_group("Input\\Source Files" FILES ${Input_SRC})
source_group("Input\\Header Files" FILES ${Input_HEAD})

source_group("Factory\\Source Files" FILES ${Factory_SRC})
source_group("Factory\\Header Files" FILES ${Factory_HEAD})

source_group("Core\\Source Files" FILES ${Core_SRC})
source_group("Core\\Header Files" FILES ${Core_HEAD})

source_group("ResourceManager\\Source Files" FILES ${Resource_SRC})
source_group("ResourceManager\\Header Files" FILES ${Resource_HEAD})

source_group("Scripting\\Source Files" FILES ${Scripting_SRC})
source_group("Scripting\\Header Files" FILES ${Scripting_HEAD})

source_group("Extern\\Source Files" FILES ${Extern_SRC})
source_group("Extern\\Header Files" FILES ${Extern_HEAD})

# ======================= Engine Include Directories ================
target_include_directories(Engine PUBLIC
    ${Audio_DIR}
    ${Camera_DIR}
    ${Collision_DIR}
    ${GameManager_DIR}
    ${Graphics_DIR}
    ${LevelEditor_DIR}
    ${Logic_DIR}
    ${Physics_DIR}
    ${Serialization_DIR}
    ${Transform_DIR}
    ${Input_DIR}
    ${Core_DIR}
    ${Factory_DIR}
    ${Extern_DIR}
    ${Resource_DIR}
    ${Scripting_DIR}
)

# ======================= Link Libraries to Engine ===================
target_link_libraries(Engine PUBLIC
    glfw
    libglew_static 
    glm
    nlohmann_json::nlohmann_json
    imgui
    freetype
    stb_image
    mono
)

# Link FMOD to Engine if available
if(TARGET fmod)
    target_link_libraries(Engine PUBLIC fmod)
    message(STATUS "FMOD will be linked to the Engine")
endif()

if(TARGET fmodstudio)
    target_link_libraries(Engine PUBLIC fmodstudio)
    message(STATUS "FMOD Studio will be linked to the Engine")
endif()

# ======================= Compiler Warnings for Engine ===============
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(Engine PRIVATE -Wall ${DisableWarnings})
elseif (MSVC)
    target_compile_options(Engine PRIVATE /W4 /WX-)
endif()

# ======================= Create Game Executable ===================
add_executable(Game
    ${Game_SRC}
    ${Game_HEAD}
)

# Set C++ standard for Game
set_property(TARGET Game PROPERTY CXX_STANDARD 20)
set_property(TARGET Game PROPERTY CXX_STANDARD_REQUIRED ON)

# ======================= Source Groups for Game Project =============
source_group("Source Files" FILES ${Game_SRC})
source_group("Header Files" FILES ${Game_HEAD})

# ======================= Game Dependencies ===========================
target_include_directories(Game PRIVATE
    ${Game_DIR}
)

# Link Game to Engine (this gives Game access to all Engine functionality)
target_link_libraries(Game PRIVATE Engine)

# ======================= Set Build Output Directory ================
# Engine library output
set_target_properties(Engine PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Build"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/Build/Debug"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/Build/Release"
)

# Game executable output
set_target_properties(Game PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Build"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/Build/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/Build/Release"
)

# ======================= Copy Runtime Assets to Build Directory =============

# --- Copy Assets, Shaders, etc. to Configuration Directory (e.g., Build/Debug) ---

add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Assets"
        "$<TARGET_FILE_DIR:Game>/Assets"
    COMMENT "Copying Assets folder to configuration directory"
)

add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Extern/Mathlib.h"
        "$<TARGET_FILE_DIR:Game>"
    COMMENT "Copying Mathlib.h folder to configuration directory"
)

add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Extern/Mathlib.cpp"
        "$<TARGET_FILE_DIR:Game>"
    COMMENT "Copying Mathlib.cpp folder to configuration directory"
)

add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Shaders"
        "$<TARGET_FILE_DIR:Game>/Shaders"
    COMMENT "Copying Shaders folder to configuration directory"
)

# --- Copy Logs to Configuration Directory (e.g., Build/Debug) ---

add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:Game>/Logs"
    COMMENT "Creating Logs folder in configuration directory"
)

add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/Logs/config.txt"
        "$<TARGET_FILE_DIR:Game>/Logs/config.txt"
    COMMENT "Copying config.txt to Logs folder in configuration directory"
)

# --- Copy Assets, Shaders, and Logs to Top-Level Build Directory (Build/) ---

add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Build/Logs"
    COMMENT "Creating Logs folder in top-level Build directory"
)

add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/Logs/config.txt"
        "${CMAKE_CURRENT_SOURCE_DIR}/Build/Logs/config.txt"
    COMMENT "Copying config.txt to Logs folder in top-level Build directory"
)

add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Assets"
        "${CMAKE_CURRENT_SOURCE_DIR}/Build/Assets"
    COMMENT "Copying Assets folder to top-level Build directory"
)

add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Shaders"
        "${CMAKE_CURRENT_SOURCE_DIR}/Build/Shaders"
    COMMENT "Copying Shaders folder to top-level Build directory"
)

message(STATUS "Asset copy commands configured successfully for all build locations.")

# ======================= Copy FMOD DLLs to Build Directory =================
if(TARGET fmod)
    set(FMOD_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Extern/FMOD")
    
    # Determine architecture
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(FMOD_DLL_DIR "${FMOD_ROOT_DIR}/api/core/lib/x64")
        set(FMOD_STUDIO_DLL_DIR "${FMOD_ROOT_DIR}/api/studio/lib/x64")
    else()
        set(FMOD_DLL_DIR "${FMOD_ROOT_DIR}/api/core/lib/x86")
        set(FMOD_STUDIO_DLL_DIR "${FMOD_ROOT_DIR}/api/studio/lib/x86")
    endif()
    
    # Find FMOD DLLs
    find_file(FMOD_DEBUG_DLL
        NAMES fmodL.dll
        PATHS ${FMOD_DLL_DIR}
        NO_DEFAULT_PATH
    )
    
    find_file(FMOD_RELEASE_DLL
        NAMES fmod.dll
        PATHS ${FMOD_DLL_DIR}
        NO_DEFAULT_PATH
    )
    
    # Find FMOD Studio DLLs
    find_file(FMOD_STUDIO_DEBUG_DLL
        NAMES fmodstudioL.dll
        PATHS ${FMOD_STUDIO_DLL_DIR}
        NO_DEFAULT_PATH
    )
    
    find_file(FMOD_STUDIO_RELEASE_DLL
        NAMES fmodstudio.dll
        PATHS ${FMOD_STUDIO_DLL_DIR}
        NO_DEFAULT_PATH
    )
    
    # Copy Debug DLLs
    if(FMOD_DEBUG_DLL)
        add_custom_command(TARGET Game POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${FMOD_DEBUG_DLL}"
                "$<TARGET_FILE_DIR:Game>"
            COMMENT "Copying fmodL.dll to Debug build directory"
        )
        message(STATUS "Will copy fmodL.dll: ${FMOD_DEBUG_DLL}")
    endif()
    
    if(FMOD_STUDIO_DEBUG_DLL)
        add_custom_command(TARGET Game POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${FMOD_STUDIO_DEBUG_DLL}"
                "$<TARGET_FILE_DIR:Game>"
            COMMENT "Copying fmodstudioL.dll to Debug build directory"
        )
        message(STATUS "Will copy fmodstudioL.dll: ${FMOD_STUDIO_DEBUG_DLL}")
    endif()
    
    # Copy Release DLLs
    if(FMOD_RELEASE_DLL)
        add_custom_command(TARGET Game POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${FMOD_RELEASE_DLL}"
                "$<TARGET_FILE_DIR:Game>"
            COMMENT "Copying fmod.dll to Release build directory"
        )
        message(STATUS "Will copy fmod.dll: ${FMOD_RELEASE_DLL}")
    endif()
    
    if(FMOD_STUDIO_RELEASE_DLL)
        add_custom_command(TARGET Game POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${FMOD_STUDIO_RELEASE_DLL}"
                "$<TARGET_FILE_DIR:Game>"
            COMMENT "Copying fmodstudio.dll to Release build directory"
        )
        message(STATUS "Will copy fmodstudio.dll: ${FMOD_STUDIO_RELEASE_DLL}")
    endif()
    
    message(STATUS "FMOD DLL copy commands configured successfully")
endif()
# ======================= Copy Mono Runtime Files =================
set(MONO_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Extern/Mono")
set(MONO_DLL_DIR "${MONO_ROOT_DIR}/bin")
set(MONO_LIB_DIR "${MONO_ROOT_DIR}/lib")
set(MONO_ETC_DIR "${MONO_ROOT_DIR}/etc")

if(EXISTS "${MONO_ROOT_DIR}")
    message(STATUS "Found Mono at: ${MONO_ROOT_DIR}")
    
    # Find and copy Mono DLLs
    find_file(MONO_DLL
        NAMES mono-2.0-sgen.dll mono-2.0-sgend.dll
        PATHS ${MONO_DLL_DIR}
        NO_DEFAULT_PATH
    )
    
    if(MONO_DLL)
        add_custom_command(TARGET Game POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MONO_DLL}"
                "$<TARGET_FILE_DIR:Game>"
            COMMENT "Copying Mono DLL to build directory"
        )
        message(STATUS "Will copy Mono DLL: ${MONO_DLL}")
    endif()
    
    find_file(MONO_POSIX_HELPER_DLL
        NAMES MonoPosixHelper.dll
        PATHS ${MONO_DLL_DIR}
        NO_DEFAULT_PATH
    )
    
    if(MONO_POSIX_HELPER_DLL)
        add_custom_command(TARGET Game POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MONO_POSIX_HELPER_DLL}"
                "$<TARGET_FILE_DIR:Game>"
            COMMENT "Copying MonoPosixHelper.dll"
        )
        message(STATUS "Will copy MonoPosixHelper.dll: ${MONO_POSIX_HELPER_DLL}")
    endif()
    
    # Copy Mono lib directory (CRITICAL - contains mscorlib.dll)
    if(EXISTS "${MONO_LIB_DIR}")
        add_custom_command(TARGET Game POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${MONO_LIB_DIR}"
                "$<TARGET_FILE_DIR:Game>/mono/lib"
            COMMENT "Copying Mono lib directory (includes mscorlib.dll)"
        )
        message(STATUS "Will copy Mono lib: ${MONO_LIB_DIR}")
    else()
        message(FATAL_ERROR "Mono lib directory not found at: ${MONO_LIB_DIR}")
    endif()
    
    # Copy Mono etc directory
    if(EXISTS "${MONO_ETC_DIR}")
        add_custom_command(TARGET Game POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${MONO_ETC_DIR}"
                "$<TARGET_FILE_DIR:Game>/mono/etc"
            COMMENT "Copying Mono etc directory"
        )
        message(STATUS "Will copy Mono etc: ${MONO_ETC_DIR}")
    else()
        message(FATAL_ERROR "Mono etc directory not found at: ${MONO_ETC_DIR}")
    endif()
    
    message(STATUS "✓ Mono runtime copy commands configured")
else()
    message(FATAL_ERROR "Mono not found at: ${MONO_ROOT_DIR}")
endif()

# ======================= Create Scripts Directory =================
add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:Game>/Scripts"
    COMMENT "Creating Scripts folder"
)
# ======================= ALSO Copy Mono to Solution Root =================
# This ensures Mono is available regardless of working directory

# Copy to top-level Build directory
if(EXISTS "${MONO_ROOT_DIR}")
    add_custom_command(TARGET Game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${MONO_LIB_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}/Build/mono/lib"
        COMMENT "Copying Mono lib to Build root"
    )
    
    add_custom_command(TARGET Game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${MONO_ETC_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}/Build/mono/etc"
        COMMENT "Copying Mono etc to Build root"
    )
    
    # Copy Mono DLL to Build root too
    if(MONO_DLL)
        add_custom_command(TARGET Game POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MONO_DLL}"
                "${CMAKE_CURRENT_SOURCE_DIR}/Build/"
            COMMENT "Copying Mono DLL to Build root"
        )
    endif()
    
    if(MONO_POSIX_HELPER_DLL)
        add_custom_command(TARGET Game POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MONO_POSIX_HELPER_DLL}"
                "${CMAKE_CURRENT_SOURCE_DIR}/Build/"
            COMMENT "Copying MonoPosixHelper to Build root"
        )
    endif()
    
    message(STATUS "Will also copy Mono to Build root directory")
endif()

# ======================= Copy to Project Root (Solution Directory) =================
# In case VS runs from project root
if(EXISTS "${MONO_ROOT_DIR}")
    add_custom_command(TARGET Game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${MONO_LIB_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}/mono/lib"
        COMMENT "Copying Mono lib to project root"
    )
    
    add_custom_command(TARGET Game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${MONO_ETC_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}/mono/etc"
        COMMENT "Copying Mono etc to project root"
    )
    
    if(MONO_DLL)
        add_custom_command(TARGET Game POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MONO_DLL}"
                "${CMAKE_CURRENT_SOURCE_DIR}/"
            COMMENT "Copying Mono DLL to project root"
        )
    endif()
    
    if(MONO_POSIX_HELPER_DLL)
        add_custom_command(TARGET Game POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MONO_POSIX_HELPER_DLL}"
                "${CMAKE_CURRENT_SOURCE_DIR}/"
            COMMENT "Copying MonoPosixHelper to project root"
        )
    endif()
    
    message(STATUS "Will also copy Mono to project root directory")
endif()

# ======================= Copy C# Scripts (Manual Build) =================
# NOTE: Build Scripts/GameScripts.csproj manually in Visual Studio
message(STATUS "C# scripts must be built manually - open Scripts/GameScripts.csproj in VS")

# Copy pre-compiled GameScripts.dll if it exists
add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/Scripts/bin/Release/net48/GameScripts.dll"
        "$<TARGET_FILE_DIR:Game>/Scripts/GameScripts.dll"
    COMMENT "Copying GameScripts.dll (if exists)"
)

# ======================= Copy to Top-Level Build Directory =================
add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Build/Scripts"
    COMMENT "Creating Scripts in top-level Build directory"
)

add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE_DIR:Game>/Scripts/GameScripts.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/Build/Scripts/GameScripts.dll"
    COMMENT "Copying GameScripts.dll to Build directory"
)

# ======================= Compiler Warnings =================
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(Game PRIVATE -Wall)
elseif(MSVC)
    target_compile_options(Game PRIVATE /W4 /WX-)
endif()