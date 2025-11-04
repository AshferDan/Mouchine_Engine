#/* Start Header ************************************************************************/
#\file       CMakeLists.txt
#\author     Jonathan Fam Qian Wei [100% - 309 lines] - Optimized Version
#\par        Fam.q@digipen.edu
#\date       October, 2, 2025

#Copyright (C) 2025 DigiPen Institute of Technology.
#Reproduction or disclosure of this file or its contents
#without the prior written consent of DigiPen Institute of
#Technology is prohibited.
#/* End Header **************************************************************************/


# ======================= Project Configuration =========================
cmake_minimum_required(VERSION 3.15)

# Set policy for imported targets
if(POLICY CMP0111)
    cmake_policy(SET CMP0111 NEW)
endif()

project(GAM_200_AME)

# Enable parallel builds for Visual Studio
if(MSVC)
    add_compile_options(/MP)
    message(STATUS "Enabled Visual Studio parallel compilation (/MP)")
endif()

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
    ${Factory_DIR}
    ${Core_DIR}
    ${Resource_DIR}
    ${Scripting_DIR}
    ${Extern_DIR}
)

# ======================= Engine Library Linking ===================
target_link_libraries(Engine PUBLIC
    glfw
    libglew_static
    glm::glm
    nlohmann_json::nlohmann_json
    imgui
    freetype
    stb_image
)

# Link FMOD if available
if(TARGET fmod)
    target_link_libraries(Engine PUBLIC fmod)
endif()

if(TARGET fmodstudio)
    target_link_libraries(Engine PUBLIC fmodstudio)
endif()

# Link Mono if available
if(TARGET mono)
    target_link_libraries(Engine PUBLIC mono)
endif()

# ======================= Game Executable ===============
add_executable(Game
    ${Game_SRC}
    ${Game_HEAD}
)

# Link Game with Engine
target_link_libraries(Game PRIVATE Engine)

# Set C++ standard for Game
set_property(TARGET Game PROPERTY CXX_STANDARD 20)
set_property(TARGET Game PROPERTY CXX_STANDARD_REQUIRED ON)

# ======================= Game Include Directories ================
target_include_directories(Game PRIVATE ${Game_DIR})

# Set Game as startup project for Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Game)

# ======================= Output Directories ========================
# Set consistent output directories for all configurations (Debug and Release only)
set_target_properties(Game PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
)

# ======================= Helper Functions for File Copying =================
# One-time copy at configure time (for DLLs that don't change)
function(copy_runtime_dependency_once source_file dest_dir)
    get_filename_component(filename "${source_file}" NAME)
    
    # Ensure destination directory exists
    file(MAKE_DIRECTORY "${dest_dir}")
    
    if(NOT EXISTS "${dest_dir}/${filename}" OR 
       "${source_file}" IS_NEWER_THAN "${dest_dir}/${filename}")
        file(COPY "${source_file}" DESTINATION "${dest_dir}")
        message(STATUS "  Copied ${filename} to ${dest_dir}")
    else()
        message(STATUS "  Skipped ${filename} (up to date)")
    endif()
endfunction()

# One-time directory copy at configure time
function(copy_directory_once source_dir dest_dir comment_text)
    if(NOT EXISTS "${dest_dir}")
        file(MAKE_DIRECTORY "${dest_dir}")
        file(COPY "${source_dir}/" DESTINATION "${dest_dir}")
        message(STATUS "  ${comment_text}")
    else()
        message(STATUS "  ${comment_text} (already exists, skipped)")
    endif()
endfunction()

# POST_BUILD copy for frequently changing files (C# DLLs)
function(copy_runtime_dependency_postbuild target_name source_file dest_dir)
    get_filename_component(filename "${source_file}" NAME)
    
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${source_file}"
            "${dest_dir}/${filename}"
        COMMENT "Copying ${filename} (only if changed)"
        VERBATIM
    )
endfunction()

# ======================= Copy FMOD DLLs (One-time at configure) =================
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
    
    # Find Debug DLLs
    find_file(FMOD_DEBUG_DLL
        NAMES fmodL.dll
        PATHS ${FMOD_DLL_DIR}
        NO_DEFAULT_PATH
    )
    
    find_file(FMOD_STUDIO_DEBUG_DLL
        NAMES fmodstudioL.dll
        PATHS ${FMOD_STUDIO_DLL_DIR}
        NO_DEFAULT_PATH
    )
    
    # Find Release DLLs
    find_file(FMOD_RELEASE_DLL
        NAMES fmod.dll
        PATHS ${FMOD_DLL_DIR}
        NO_DEFAULT_PATH
    )
    
    find_file(FMOD_STUDIO_RELEASE_DLL
        NAMES fmodstudio.dll
        PATHS ${FMOD_STUDIO_DLL_DIR}
        NO_DEFAULT_PATH
    )
    
    message(STATUS "Copying FMOD DLLs (one-time at configure)...")
    
    # Copy Debug DLLs to Debug
    if(FMOD_DEBUG_DLL)
        copy_runtime_dependency_once("${FMOD_DEBUG_DLL}" "${CMAKE_BINARY_DIR}/bin/Debug")
    endif()
    
    if(FMOD_STUDIO_DEBUG_DLL)
        copy_runtime_dependency_once("${FMOD_STUDIO_DEBUG_DLL}" "${CMAKE_BINARY_DIR}/bin/Debug")
    endif()
    
    # Copy Release DLLs to Release
    if(FMOD_RELEASE_DLL)
        copy_runtime_dependency_once("${FMOD_RELEASE_DLL}" "${CMAKE_BINARY_DIR}/bin/Release")
    endif()
    
    if(FMOD_STUDIO_RELEASE_DLL)
        copy_runtime_dependency_once("${FMOD_STUDIO_RELEASE_DLL}" "${CMAKE_BINARY_DIR}/bin/Release")
    endif()
    
    message(STATUS "✓ FMOD DLLs copied successfully")
endif()

# ======================= Copy Mono Runtime Files (One-time at configure) =================
set(MONO_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Extern/Mono")
set(MONO_DLL_DIR "${MONO_ROOT_DIR}/bin")
set(MONO_LIB_DIR "${MONO_ROOT_DIR}/lib")
set(MONO_ETC_DIR "${MONO_ROOT_DIR}/etc")

if(EXISTS "${MONO_ROOT_DIR}")
    message(STATUS "Copying Mono runtime files (one-time at configure)...")
    
    # Find Mono DLLs
    find_file(MONO_DLL
        NAMES mono-2.0-sgen.dll mono-2.0-sgend.dll
        PATHS ${MONO_DLL_DIR}
        NO_DEFAULT_PATH
    )
    
    find_file(MONO_POSIX_HELPER_DLL
        NAMES MonoPosixHelper.dll
        PATHS ${MONO_DLL_DIR}
        NO_DEFAULT_PATH
    )
    
    # Copy Mono DLLs to all build configurations (Debug and Release)
    if(MONO_DLL)
        copy_runtime_dependency_once("${MONO_DLL}" "${CMAKE_BINARY_DIR}/bin/Debug")
        copy_runtime_dependency_once("${MONO_DLL}" "${CMAKE_BINARY_DIR}/bin/Release")
        copy_runtime_dependency_once("${MONO_DLL}" "${CMAKE_CURRENT_SOURCE_DIR}/Build")
        copy_runtime_dependency_once("${MONO_DLL}" "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
    
    if(MONO_POSIX_HELPER_DLL)
        copy_runtime_dependency_once("${MONO_POSIX_HELPER_DLL}" "${CMAKE_BINARY_DIR}/bin/Debug")
        copy_runtime_dependency_once("${MONO_POSIX_HELPER_DLL}" "${CMAKE_BINARY_DIR}/bin/Release")
        copy_runtime_dependency_once("${MONO_POSIX_HELPER_DLL}" "${CMAKE_CURRENT_SOURCE_DIR}/Build")
        copy_runtime_dependency_once("${MONO_POSIX_HELPER_DLL}" "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
    
    # Copy Mono lib directory (CRITICAL - contains mscorlib.dll)
    if(EXISTS "${MONO_LIB_DIR}")
        copy_directory_once("${MONO_LIB_DIR}" "${CMAKE_BINARY_DIR}/bin/Debug/mono/lib" 
            "Copied Mono lib to Debug")
        copy_directory_once("${MONO_LIB_DIR}" "${CMAKE_BINARY_DIR}/bin/Release/mono/lib" 
            "Copied Mono lib to Release")
        copy_directory_once("${MONO_LIB_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/Build/mono/lib"
            "Copied Mono lib to Build root")
        copy_directory_once("${MONO_LIB_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/mono/lib"
            "Copied Mono lib to project root")
    else()
        message(FATAL_ERROR "Mono lib directory not found at: ${MONO_LIB_DIR}")
    endif()
    
    # Copy Mono etc directory
    if(EXISTS "${MONO_ETC_DIR}")
        copy_directory_once("${MONO_ETC_DIR}" "${CMAKE_BINARY_DIR}/bin/Debug/mono/etc"
            "Copied Mono etc to Debug")
        copy_directory_once("${MONO_ETC_DIR}" "${CMAKE_BINARY_DIR}/bin/Release/mono/etc"
            "Copied Mono etc to Release")
        copy_directory_once("${MONO_ETC_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/Build/mono/etc"
            "Copied Mono etc to Build root")
        copy_directory_once("${MONO_ETC_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/mono/etc"
            "Copied Mono etc to project root")
    else()
        message(FATAL_ERROR "Mono etc directory not found at: ${MONO_ETC_DIR}")
    endif()
    
    message(STATUS "✓ Mono runtime files copied successfully")
else()
    message(FATAL_ERROR "Mono not found at: ${MONO_ROOT_DIR}")
endif()

# ======================= Create Scripts Directory =================
add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:Game>/Scripts"
    COMMENT "Creating Scripts folder"
    VERBATIM
)

# ======================= C# Project Configuration ===========================
set(CSHARP_PROJECT_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/Scripts")
set(CSHARP_PROJECT_FILE "${CSHARP_PROJECT_DIR}/GameScript.csproj")
set(CSHARP_DLL_SOURCE   "${CSHARP_PROJECT_DIR}/bin/Debug/net48/GameScripts.dll")

# ======================= Add C# Project to Solution ========================
message(STATUS "Adding C# project to solution: ${CSHARP_PROJECT_FILE}")

include_external_msproject(CSharpSrcipts ${CSHARP_PROJECT_FILE})

add_dependencies(Game CSharpSrcipts)

# ======================= Copy C# Scripts (POST_BUILD) =====================

# 3. Create the 'Scripts' folder in the output directory
add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:Game>/Scripts"
    COMMENT "Creating Scripts folder"
    VERBATIM
)

# 4. Copy the newly built DLL to the 'Scripts' folder
if(EXISTS "${CSHARP_DLL_SOURCE}")
    message(STATUS "GameScripts.dll found, configuring copy for hot-reload")
    
    copy_runtime_dependency_postbuild(Game 
        "${CSHARP_DLL_SOURCE}"
        "$<TARGET_FILE_DIR:Game>/Scripts"
    )
else()
    message(WARNING "GameScripts.dll not found after build. Check C# project.")
endif()

# ======================= Compiler Warnings =================
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(Game PRIVATE -Wall)
elseif(MSVC)
    target_compile_options(Game PRIVATE /W4 /WX-)
endif()