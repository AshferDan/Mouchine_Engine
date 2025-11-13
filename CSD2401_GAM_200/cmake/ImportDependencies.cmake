#/* Start Header ************************************************************************/
#\file       ImportDependencies.cmake
#\author     Optimized Version
#\date       October 30, 2025
#/* End Header **************************************************************************/

include(FetchContent)

# Performance optimizations
set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# ======================= GLFW ==========================
macro(import_glfw)
    if(NOT TARGET glfw)
        message(STATUS "Configuring GLFW...")
        FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.4
            GIT_SHALLOW TRUE
        )
        set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(glfw)
        message(STATUS "✓ GLFW configured")
    endif()
endmacro()

# ======================= GLM ===========================
macro(import_glm)
    if(NOT TARGET glm)
        message(STATUS "Configuring GLM...")
        FetchContent_Declare(
            glm
            GIT_REPOSITORY https://github.com/g-truc/glm.git
            GIT_TAG 1.0.1
            GIT_SHALLOW TRUE
        )
        set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(glm)
        message(STATUS "✓ GLM configured")
    endif()
endmacro()

# ======================= GLEW ==========================
macro(import_glew)
    if(NOT TARGET libglew_static)
        message(STATUS "Configuring GLEW...")
        FetchContent_Declare(
            glew
            GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
            GIT_TAG glew-cmake-2.2.0
            GIT_SHALLOW TRUE
        )
        set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "" FORCE)
        set(glew-cmake_BUILD_STATIC ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(glew)
        message(STATUS "✓ GLEW configured")
    endif()
endmacro()

# ======================= nlohmann/json =================
macro(import_json)
    if(NOT TARGET nlohmann_json::nlohmann_json)
        message(STATUS "Configuring nlohmann/json...")
        FetchContent_Declare(
            json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG v3.11.3
            GIT_SHALLOW TRUE
        )
        set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
        set(JSON_Install OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(json)
        message(STATUS "✓ nlohmann/json configured")
    endif()
endmacro()

# ======================= ImGui =========================
macro(import_imgui)
    if(NOT TARGET imgui)
        message(STATUS "Configuring ImGui...")
        FetchContent_Declare(
            imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG v1.92.4-docking
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(imgui)
        
        add_library(imgui STATIC
            ${imgui_SOURCE_DIR}/imgui.cpp
            ${imgui_SOURCE_DIR}/imgui_demo.cpp
            ${imgui_SOURCE_DIR}/imgui_draw.cpp
            ${imgui_SOURCE_DIR}/imgui_tables.cpp
            ${imgui_SOURCE_DIR}/imgui_widgets.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        )
        target_include_directories(imgui PUBLIC 
            ${imgui_SOURCE_DIR}
            ${imgui_SOURCE_DIR}/backends
        )
        target_link_libraries(imgui PUBLIC glfw libglew_static)
        set_property(TARGET imgui PROPERTY CXX_STANDARD 11)
        set_property(TARGET imgui PROPERTY CXX_STANDARD_REQUIRED ON)
        message(STATUS "✓ ImGui configured")
    endif()
endmacro()

macro(import_imguizmo)  
    if(NOT TARGET imguizmo)
        message(STATUS " Configuring Imguizmo...")
        FetchContent_Declare(
            imguizmo
            GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git
            GIT_TAG master
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(imguizmo)

      
        add_library(imguizmo STATIC
            ${imguizmo_SOURCE_DIR}/ImGuizmo.cpp
        )
        
        target_include_directories(imguizmo PUBLIC
            ${imguizmo_SOURCE_DIR}
        )
        
        target_link_libraries(imguizmo PUBLIC imgui)

        message(STATUS "✓ Imguizmo configured")
    endif()
endmacro()



# ======================= FreeType ======================
macro(import_freetype)
    if(NOT TARGET freetype)
        message(STATUS "Configuring FreeType...")
        FetchContent_Declare(
            freetype
            GIT_REPOSITORY https://github.com/freetype/freetype.git
            GIT_TAG VER-2-13-2
            GIT_SHALLOW TRUE
        )
        set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
        set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
        set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
        set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
        set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(freetype)
        message(STATUS "✓ FreeType configured")
    endif()
endmacro()

# ======================= stb_image =====================
macro(import_stb_image)
    if(NOT TARGET stb_image)
        message(STATUS "Configuring stb_image...")
        FetchContent_Declare(
            stb
            GIT_REPOSITORY https://github.com/nothings/stb.git
            GIT_TAG master
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(stb)
        
        set(STB_IMPL_FILE ${CMAKE_CURRENT_BINARY_DIR}/stb_image_impl.c)
        if(NOT EXISTS ${STB_IMPL_FILE})
            file(WRITE ${STB_IMPL_FILE}
                "#define STB_IMAGE_IMPLEMENTATION\n"
                "#include \"${stb_SOURCE_DIR}/stb_image.h\"\n"
            )
        endif()
        
        add_library(stb_image STATIC ${STB_IMPL_FILE})
        target_include_directories(stb_image PUBLIC ${stb_SOURCE_DIR})
        
        if(MSVC)
            target_compile_options(stb_image PRIVATE /W0)
        else()
            target_compile_options(stb_image PRIVATE -w)
        endif()
        message(STATUS "✓ stb_image configured")
    endif()
endmacro()

# ======================= FMOD ==========================
macro(import_fmod)
    if(NOT TARGET fmod)
        message(STATUS "Configuring FMOD...")
        set(FMOD_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Extern/FMOD")
        
        if(NOT EXISTS "${FMOD_ROOT_DIR}")
            message(WARNING "FMOD not found at: ${FMOD_ROOT_DIR}")
            return()
        endif()
        
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(FMOD_ARCH "x64")
        else()
            set(FMOD_ARCH "x86")
        endif()
        
        find_path(FMOD_INCLUDE_DIR NAMES fmod.h
            PATHS ${FMOD_ROOT_DIR}/api/core/inc NO_DEFAULT_PATH)
        set(FMOD_LIB_DIR "${FMOD_ROOT_DIR}/api/core/lib/${FMOD_ARCH}")
        find_file(FMOD_RELEASE_LIB NAMES fmod_vc.lib
            PATHS ${FMOD_LIB_DIR} NO_DEFAULT_PATH)
        find_file(FMOD_DEBUG_LIB NAMES fmodL_vc.lib
            PATHS ${FMOD_LIB_DIR} NO_DEFAULT_PATH)
        
        if(FMOD_INCLUDE_DIR AND FMOD_RELEASE_LIB AND FMOD_DEBUG_LIB)
            add_library(fmod STATIC IMPORTED GLOBAL)
            set_target_properties(fmod PROPERTIES
                IMPORTED_LOCATION_DEBUG "${FMOD_DEBUG_LIB}"
                IMPORTED_LOCATION_RELEASE "${FMOD_RELEASE_LIB}"
                IMPORTED_LOCATION_MINSIZEREL "${FMOD_RELEASE_LIB}"
                IMPORTED_LOCATION_RELWITHDEBINFO "${FMOD_RELEASE_LIB}"
                INTERFACE_INCLUDE_DIRECTORIES "${FMOD_INCLUDE_DIR}"
            )
            message(STATUS "✓ FMOD Core configured (${FMOD_ARCH})")
            
            # FMOD Studio
            set(FMOD_STUDIO_LIB_DIR "${FMOD_ROOT_DIR}/api/studio/lib/${FMOD_ARCH}")
            find_path(FMOD_STUDIO_INCLUDE_DIR NAMES fmod_studio.h
                PATHS ${FMOD_ROOT_DIR}/api/studio/inc NO_DEFAULT_PATH)
            find_file(FMOD_STUDIO_RELEASE_LIB NAMES fmodstudio_vc.lib
                PATHS ${FMOD_STUDIO_LIB_DIR} NO_DEFAULT_PATH)
            find_file(FMOD_STUDIO_DEBUG_LIB NAMES fmodstudioL_vc.lib
                PATHS ${FMOD_STUDIO_LIB_DIR} NO_DEFAULT_PATH)
            
            if(FMOD_STUDIO_INCLUDE_DIR AND FMOD_STUDIO_RELEASE_LIB AND FMOD_STUDIO_DEBUG_LIB)
                add_library(fmodstudio STATIC IMPORTED GLOBAL)
                set_target_properties(fmodstudio PROPERTIES
                    IMPORTED_LOCATION_DEBUG "${FMOD_STUDIO_DEBUG_LIB}"
                    IMPORTED_LOCATION_RELEASE "${FMOD_STUDIO_RELEASE_LIB}"
                    IMPORTED_LOCATION_MINSIZEREL "${FMOD_STUDIO_RELEASE_LIB}"
                    IMPORTED_LOCATION_RELWITHDEBINFO "${FMOD_STUDIO_RELEASE_LIB}"
                    INTERFACE_INCLUDE_DIRECTORIES "${FMOD_STUDIO_INCLUDE_DIR}"
                )
                message(STATUS "✓ FMOD Studio configured (${FMOD_ARCH})")
            endif()
        endif()
    endif()
endmacro()

# ======================= Mono ==========================
macro(import_mono)
    if(NOT TARGET mono)
        message(STATUS "Configuring Mono...")
        set(MONO_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Extern/Mono")
        
        if(NOT EXISTS "${MONO_ROOT_DIR}")
            message(WARNING "Mono not found at: ${MONO_ROOT_DIR}")
            return()
        endif()
        
        find_path(MONO_INCLUDE_DIR NAMES mono/jit/jit.h
            PATHS ${MONO_ROOT_DIR}/include NO_DEFAULT_PATH)
        find_file(MONO_IMPORT_LIB NAMES mono-2.0-sgen.lib
            PATHS ${MONO_ROOT_DIR}/lib NO_DEFAULT_PATH)
        
        if(MONO_INCLUDE_DIR AND MONO_IMPORT_LIB)
            add_library(mono SHARED IMPORTED GLOBAL)
            set_target_properties(mono PROPERTIES
                IMPORTED_IMPLIB "${MONO_IMPORT_LIB}"
                INTERFACE_INCLUDE_DIRECTORIES "${MONO_INCLUDE_DIR}"
            )
            message(STATUS "✓ Mono configured")
        else()
            message(WARNING "Mono library or includes not found")
        endif()
    endif()
endmacro()

# ======================= Import All ====================
macro(importDependencies)
    message(STATUS "")
    message(STATUS "========== Importing Dependencies ==========")
    import_glfw()
    import_glew()
    import_glm()
    import_json()
    import_imgui()
    import_imguizmo()
    import_freetype()
    import_stb_image()
    import_fmod()
    import_mono()
    message(STATUS "=========== Dependencies Complete ===========")
    message(STATUS "")
endmacro()