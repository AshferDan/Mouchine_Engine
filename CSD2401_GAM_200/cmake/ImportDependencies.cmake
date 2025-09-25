include(FetchContent)

# Macro to import GLFW
macro(import_glfw)
    if(NOT TARGET glfw)  # Guard to prevent multiple inclusion
        FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.3.8
        )
        if(NOT glfw_POPULATED)
            set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
            set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
            set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
            FetchContent_Populate(glfw)
        endif()

        add_subdirectory(${glfw_SOURCE_DIR})
        include_directories(${GLFW_SOURCE_DIR}/include)
    endif()
endmacro()

# Macro to import glm
macro(import_glm)
    if(NOT TARGET glm)  # Guard to prevent multiple inclusion
        FetchContent_Declare(
            glm
            GIT_REPOSITORY https://github.com/g-truc/glm.git
            GIT_TAG master
        )
        FetchContent_MakeAvailable(glm)

        include_directories(${glm_SOURCE_DIR})
    endif()
endmacro()

# Macro to import glew
macro(import_glew)
    if(NOT TARGET glew_s)  # Guard to prevent multiple inclusion
        include(FetchContent)

        FetchContent_Declare(
            glew
            GIT_REPOSITORY https://github.com/omniavinco/glew-cmake.git
            GIT_TAG master  # Or use a specific commit or tag if preferred
        )

        # Disable shared builds if needed
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libs" FORCE)
        FetchContent_MakeAvailable(glew)

        # Optional: include directories if needed manually (though target_link_libraries handles it)
        include_directories(${glew_SOURCE_DIR}/include)
    endif()
endmacro()

# Macro to import nlohmann/json
macro(import_json)
    if(NOT TARGET nlohmann_json::nlohmann_json)  # Fixed target name and typo
        include(FetchContent)
        FetchContent_Declare(
            json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG v3.11.3  # Use a stable version instead of master
        )
        FetchContent_MakeAvailable(json)
    endif()
endmacro()

# Macro to import ImGui
macro(import_imgui)
    if(NOT TARGET imgui)  # Guard to prevent multiple inclusion
        FetchContent_Declare(
            imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG v1.90.4
        )
        if(NOT imgui_POPULATED)
            FetchContent_Populate(imgui)
        endif()

        # Create ImGui library manually since it doesn't have CMakeLists.txt
        set(IMGUI_SOURCES
            ${imgui_SOURCE_DIR}/imgui.cpp
            ${imgui_SOURCE_DIR}/imgui_demo.cpp
            ${imgui_SOURCE_DIR}/imgui_draw.cpp
            ${imgui_SOURCE_DIR}/imgui_tables.cpp
            ${imgui_SOURCE_DIR}/imgui_widgets.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        )

        add_library(imgui STATIC ${IMGUI_SOURCES})
        target_include_directories(imgui PUBLIC 
            ${imgui_SOURCE_DIR}
            ${imgui_SOURCE_DIR}/backends
        )
        target_link_libraries(imgui PUBLIC glfw libglew_static)
        
        # Set C++ standard for ImGui (required for constexpr support)
        set_property(TARGET imgui PROPERTY CXX_STANDARD 11)
        set_property(TARGET imgui PROPERTY CXX_STANDARD_REQUIRED ON)
    endif()
endmacro()


# Macro to import FreeType
macro(import_freetype)
    if(NOT TARGET freetype)  # Guard to prevent multiple inclusion
        FetchContent_Declare(
            freetype
            GIT_REPOSITORY https://github.com/freetype/freetype.git
            GIT_TAG master
        )
        FetchContent_MakeAvailable(freetype)
        
        # Include directories
        include_directories(${freetype_SOURCE_DIR}/include)
    endif()
endmacro()

macro(import_fmod)
    if(NOT TARGET fmod)
        set(FMOD_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Extern/FMOD")
        
        if(NOT EXISTS "${FMOD_ROOT_DIR}")
            message(STATUS "FMOD directory not found. Skipping FMOD integration.")
            return()
        endif()
        
        find_path(FMOD_INCLUDE_DIR 
            NAMES fmod.h
            PATHS ${FMOD_ROOT_DIR}/api/core/inc
            NO_DEFAULT_PATH
        )
        
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(FMOD_LIB_DIR "${FMOD_ROOT_DIR}/api/core/lib/x64")
            set(FMOD_STUDIO_LIB_DIR "${FMOD_ROOT_DIR}/api/studio/lib/x64")
        else()
            set(FMOD_LIB_DIR "${FMOD_ROOT_DIR}/api/core/lib/x86")
            set(FMOD_STUDIO_LIB_DIR "${FMOD_ROOT_DIR}/api/studio/lib/x86")
        endif()
        
        find_file(FMOD_LIBRARY_FILE
            NAMES fmod_vc.lib fmodL_vc.lib
            PATHS ${FMOD_LIB_DIR}
            NO_DEFAULT_PATH
        )
        
        if(FMOD_INCLUDE_DIR AND FMOD_LIBRARY_FILE AND EXISTS "${FMOD_LIBRARY_FILE}")
            add_library(fmod STATIC IMPORTED)
            
            set_target_properties(fmod PROPERTIES
                IMPORTED_LOCATION "${FMOD_LIBRARY_FILE}"
                INTERFACE_INCLUDE_DIRECTORIES "${FMOD_INCLUDE_DIR}"
            )
            
            message(STATUS "FMOD configured successfully")
            
            find_file(FMOD_STUDIO_LIBRARY_FILE
                NAMES fmodstudio_vc.lib fmodstudioL_vc.lib
                PATHS ${FMOD_STUDIO_LIB_DIR}
                NO_DEFAULT_PATH
            )
            
            if(FMOD_STUDIO_LIBRARY_FILE AND EXISTS "${FMOD_STUDIO_LIBRARY_FILE}")
                find_path(FMOD_STUDIO_INCLUDE_DIR 
                    NAMES fmod_studio.h
                    PATHS ${FMOD_ROOT_DIR}/api/studio/inc
                    NO_DEFAULT_PATH
                )
                
                if(FMOD_STUDIO_INCLUDE_DIR)
                    add_library(fmodstudio STATIC IMPORTED)
                    
                    set_target_properties(fmodstudio PROPERTIES
                        IMPORTED_LOCATION "${FMOD_STUDIO_LIBRARY_FILE}"
                        INTERFACE_INCLUDE_DIRECTORIES "${FMOD_STUDIO_INCLUDE_DIR}"
                    )
                    
                    message(STATUS "FMOD Studio configured successfully")
                endif()
            endif()
        else()
            message(STATUS "FMOD not found. Continuing without FMOD...")
        endif()
    endif()
endmacro()

# Macro to import all dependencies
macro(importDependencies)
    message(STATUS "Starting to import dependencies...")

    message(STATUS "Importing GLFW...")
    import_glfw()
    message(STATUS "GLFW imported successfully.")

    message(STATUS "Importing GLM...")
    import_glm()
    message(STATUS "GLM imported successfully.")

    message(STATUS "Importing GLEW...")
    import_glew()
    message(STATUS "GLEW imported successfully.")

    message(STATUS "Importing nlohmann/json...")
    import_json()
    message(STATUS "nlohmann/json imported successfully.")

    message(STATUS "Importing ImGui...")
    import_imgui()
    message(STATUS "ImGui imported successfully.")

    message(STATUS "Importing freetype...")
    import_freetype()
    message(STATUS "freetype imported successfully.")

    message(STATUS "Importing FMOD...")
    import_fmod()
    message(STATUS "FMOD imported successfully.")

    message(STATUS "All dependencies have been imported successfully.")
endmacro()