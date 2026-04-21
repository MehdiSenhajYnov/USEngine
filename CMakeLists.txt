cmake_minimum_required(VERSION 3.24)

project(vde_ynov LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(VDE_FETCH_MISSING_DEPS "Download missing third-party dependencies at configure time" ON)

if(WIN32 AND NOT MSVC)
    message(FATAL_ERROR "Ce projet doit etre configure avec un toolchain MSVC/Visual Studio dans CLion, pas MinGW/GNU.")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(VDE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(VDE_ENGINE_INCLUDE_DIR ${VDE_ROOT}/Engine/include)
set(VDE_IMGUI_INCLUDE_DIR ${VDE_ROOT}/Imgui/include)
set(VDE_COMPAT_INCLUDE_DIR ${VDE_ROOT}/Compat/include)
set(VDE_RUNTIME_ROOT ${CMAKE_BINARY_DIR})

if(EXISTS "${VDE_ROOT}/App/assets")
    set(VDE_ASSETS_SOURCE_DIR "${VDE_ROOT}/App/assets")
elseif(EXISTS "${VDE_ROOT}/bin/assets")
    set(VDE_ASSETS_SOURCE_DIR "${VDE_ROOT}/bin/assets")
else()
    set(VDE_ASSETS_SOURCE_DIR "")
endif()

if(EXISTS "${VDE_ROOT}/assets.zip")
    set(VDE_ASSETS_ARCHIVE "${VDE_ROOT}/assets.zip")
else()
    set(VDE_ASSETS_ARCHIVE "")
endif()

function(vde_require_target out_var)
    foreach(candidate IN LISTS ARGN)
        if(TARGET "${candidate}")
            set(${out_var} "${candidate}" PARENT_SCOPE)
            return()
        endif()
    endforeach()

    string(REPLACE ";" ", " candidate_list "${ARGN}")
    message(FATAL_ERROR "Impossible de trouver une target compatible parmi: ${candidate_list}")
endfunction()

find_package(Vulkan REQUIRED)

find_package(glfw3 CONFIG QUIET)
if(TARGET glfw)
    set(VDE_GLFW_TARGET glfw)
elseif(TARGET glfw3)
    set(VDE_GLFW_TARGET glfw3)
elseif(VDE_FETCH_MISSING_DEPS)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(glfw)
    vde_require_target(VDE_GLFW_TARGET glfw glfw3)
else()
    message(FATAL_ERROR "GLFW est introuvable. Activez VDE_FETCH_MISSING_DEPS ou installez glfw.")
endif()

find_package(glm CONFIG QUIET)
if(TARGET glm::glm)
    set(VDE_GLM_TARGET glm::glm)
elseif(TARGET glm)
    set(VDE_GLM_TARGET glm)
elseif(VDE_FETCH_MISSING_DEPS)
    set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.1
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(glm)
    vde_require_target(VDE_GLM_TARGET glm::glm glm-header-only glm)
else()
    message(FATAL_ERROR "GLM est introuvable. Activez VDE_FETCH_MISSING_DEPS ou installez glm.")
endif()

find_package(vk-bootstrap CONFIG QUIET)
if(NOT TARGET vk-bootstrap::vk-bootstrap AND NOT TARGET vk-bootstrap)
    if(NOT VDE_FETCH_MISSING_DEPS)
        message(FATAL_ERROR "vk-bootstrap est introuvable. Activez VDE_FETCH_MISSING_DEPS ou installez-le.")
    endif()

    set(VK_BOOTSTRAP_TEST OFF CACHE BOOL "" FORCE)
    set(VK_BOOTSTRAP_INSTALL OFF CACHE BOOL "" FORCE)
    set(VK_BOOTSTRAP_DISABLE_WARNINGS ON CACHE BOOL "" FORCE)
    set(VK_BOOTSTRAP_POSITION_INDEPENDENT_CODE ON CACHE BOOL "" FORCE)
    FetchContent_Declare(vkbootstrap
        GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap.git
        GIT_TAG v1.4.341
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(vkbootstrap)
endif()
vde_require_target(VDE_VKBOOTSTRAP_TARGET vk-bootstrap::vk-bootstrap vk-bootstrap)
if(TARGET vk-bootstrap)
    set_target_properties(vk-bootstrap PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

find_package(SPIRV-Reflect CONFIG QUIET)
if(NOT TARGET spirv-reflect-static AND NOT TARGET spirv-reflect)
    if(NOT VDE_FETCH_MISSING_DEPS)
        message(FATAL_ERROR "SPIRV-Reflect est introuvable. Activez VDE_FETCH_MISSING_DEPS ou installez-le.")
    endif()

    set(SPIRV_REFLECT_EXECUTABLE OFF CACHE BOOL "" FORCE)
    set(SPIRV_REFLECT_STATIC_LIB ON CACHE BOOL "" FORCE)
    set(SPIRV_REFLECT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(SPIRV_REFLECT_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(spirv_reflect
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Reflect.git
        GIT_TAG vulkan-sdk-1.4.341.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(spirv_reflect)
endif()
vde_require_target(VDE_SPIRV_REFLECT_TARGET spirv-reflect-static spirv-reflect)

if(TARGET spirv-reflect-static)
    target_include_directories(spirv-reflect-static PUBLIC ${VDE_COMPAT_INCLUDE_DIR})
    set_target_properties(spirv-reflect-static PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
if(TARGET spirv-reflect)
    target_include_directories(spirv-reflect PUBLIC ${VDE_COMPAT_INCLUDE_DIR})
    set_target_properties(spirv-reflect PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

add_library(vde_compat_headers INTERFACE)
target_include_directories(vde_compat_headers INTERFACE ${VDE_COMPAT_INCLUDE_DIR})

add_library(vde_project_warnings INTERFACE)
if(MSVC)
    target_compile_options(vde_project_warnings INTERFACE /MP /W3 /permissive- /EHsc)
else()
    target_compile_options(vde_project_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif()

function(vde_set_runtime_output_dirs target_name output_dir)
    set_target_properties(${target_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${output_dir}"
        LIBRARY_OUTPUT_DIRECTORY "${output_dir}"
    )

    foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel)
        string(TOUPPER "${config}" config_upper)
        set_target_properties(${target_name} PROPERTIES
            "RUNTIME_OUTPUT_DIRECTORY_${config_upper}" "${output_dir}"
            "LIBRARY_OUTPUT_DIRECTORY_${config_upper}" "${output_dir}"
        )
    endforeach()
endfunction()

add_library(VDE::Vulkan ALIAS Vulkan::Vulkan)

add_library(vde_engine STATIC
    Engine/src/core/assets/assetsource.cpp
    Engine/src/core/assets/meshasset.cpp
    Engine/src/core/assets/shaderasset.cpp
    Engine/src/core/assets/textureasset.cpp
    Engine/src/core/gpu/buffer_impl_vulkan.cpp
    Engine/src/core/gpu/commandbuffer_impl_vulkan.cpp
    Engine/src/core/gpu/commandpool_impl_vulkan.cpp
    Engine/src/core/gpu/descriptorpool_impl_vulkan.cpp
    Engine/src/core/gpu/descriptorset_impl_vulkan.cpp
    Engine/src/core/gpu/enums.cpp
    Engine/src/core/gpu/image_impl_vulkan.cpp
    Engine/src/core/gpu/pipeline_impl_vulkan.cpp
    Engine/src/core/gpu/shader.cpp
    Engine/src/core/gpu/shader_impl_vulkan.cpp
    Engine/src/core/gpu/shader_impl_vulkan__reflect.cpp
    Engine/src/core/gpu_detail/vk_converters.cpp
    Engine/src/core/gpu_detail/vk_images.cpp
    Engine/src/core/gpu_detail/vk_initializers.cpp
    Engine/src/core/graphicscontext_impl_vulkan.cpp
    Engine/src/core/window_impl_glfw_vulkan.cpp
    Engine/src/graphics/mesh.cpp
    Engine/src/util/pluginregistry.cpp
    Engine/src/util/property.cpp
    Engine/src/util/stringhash.cpp
)

target_include_directories(vde_engine
    PUBLIC
        ${VDE_ENGINE_INCLUDE_DIR}
)

target_link_libraries(vde_engine
    PUBLIC
        VDE::Vulkan
        ${VDE_GLFW_TARGET}
        ${VDE_GLM_TARGET}
        ${VDE_VKBOOTSTRAP_TARGET}
        ${VDE_SPIRV_REFLECT_TARGET}
        vde_compat_headers
        ${CMAKE_DL_LIBS}
    PRIVATE
        vde_project_warnings
)

set_target_properties(vde_engine PROPERTIES
    OUTPUT_NAME "Engine"
)

add_library(vde_imgui STATIC
    Imgui/src/imgui.cpp
    Imgui/src/imgui_draw.cpp
    Imgui/src/imgui_impl_glfw.cpp
    Imgui/src/imgui_impl_vulkan.cpp
    Imgui/src/imgui_stdlib.cpp
    Imgui/src/imgui_tables.cpp
    Imgui/src/imgui_vde.cpp
    Imgui/src/imgui_widgets.cpp
)

target_include_directories(vde_imgui
    PUBLIC
        ${VDE_IMGUI_INCLUDE_DIR}
        ${VDE_ENGINE_INCLUDE_DIR}
)

target_compile_definitions(vde_imgui
    PRIVATE
        GLFW_INCLUDE_VULKAN
)

target_link_libraries(vde_imgui
    PUBLIC
        vde_engine
        VDE::Vulkan
        ${VDE_GLFW_TARGET}
    PRIVATE
        vde_project_warnings
)

set_target_properties(vde_imgui PROPERTIES
    OUTPUT_NAME "ImGui"
)

function(vde_add_plugin target_name)
    cmake_parse_arguments(ARG "" "INCLUDE_DIR" "SOURCES" ${ARGN})

    add_library(${target_name} SHARED ${ARG_SOURCES})
    target_include_directories(${target_name}
        PRIVATE
            ${ARG_INCLUDE_DIR}
    )
    target_link_libraries(${target_name}
        PRIVATE
            vde_engine
            vde_compat_headers
            vde_project_warnings
    )

    vde_set_runtime_output_dirs(${target_name} "${VDE_RUNTIME_ROOT}/engine/plugins")
endfunction()

vde_add_plugin(vde_fileformat_obj
    INCLUDE_DIR ${VDE_ROOT}/Plugins/FileFormat_obj/include
    SOURCES
        Plugins/FileFormat_obj/src/fileformat_obj.cpp
        Plugins/FileFormat_obj/src/fileformat_obj_export.cpp
)

set_target_properties(vde_fileformat_obj PROPERTIES OUTPUT_NAME "FileFormat_obj")
target_compile_definitions(vde_fileformat_obj PRIVATE GLM_ENABLE_EXPERIMENTAL)

vde_add_plugin(vde_fileformat_stbimage
    INCLUDE_DIR ${VDE_ROOT}/Plugins/FileFormat_stbimage/include
    SOURCES
        Plugins/FileFormat_stbimage/src/fileformat_stbimage.cpp
        Plugins/FileFormat_stbimage/src/fileformat_stbimage_export.cpp
)

set_target_properties(vde_fileformat_stbimage PROPERTIES OUTPUT_NAME "FileFormat_stbimage")

add_executable(vde_app
    App/main.cpp
    App/Core/BaseObject.cpp
    App/Core/Components/CameraComponent.cpp
    App/Core/Components/Component.cpp
    App/Core/Components/Render/RenderComponent.cpp
    App/Core/Components/TransformComponent.cpp
    App/Core/GameObjects/CameraObject.cpp
    App/Core/GameObjects/GameObject.cpp
    App/Core/GameObjects/RendererObject.cpp
    App/Core/Input/KeyEnum.cpp
    App/Core/Input/WindowHandler.cpp
    App/Core/List.cpp
    App/Core/Managers/AssetsManager.cpp
    App/Core/Managers/CameraManager.cpp
    App/Core/Managers/InputManager.cpp
    App/Core/Scene/Scene.cpp
    App/OptRef.cpp
)

target_include_directories(vde_app
    PRIVATE
        ${VDE_ROOT}/App
)

target_link_libraries(vde_app
    PRIVATE
        vde_engine
        vde_imgui
        vde_compat_headers
        vde_project_warnings
)
target_compile_definitions(vde_app PRIVATE GLM_ENABLE_EXPERIMENTAL)

set_target_properties(vde_app PROPERTIES
    OUTPUT_NAME "App"
)
vde_set_runtime_output_dirs(vde_app "${VDE_RUNTIME_ROOT}")

set(VDE_RUNTIME_DIR "$<TARGET_FILE_DIR:vde_app>")
set_property(TARGET vde_app PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${VDE_RUNTIME_ROOT}")

add_custom_command(TARGET vde_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${VDE_RUNTIME_DIR}/engine/plugins"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:vde_fileformat_obj>"
        "${VDE_RUNTIME_DIR}/engine/plugins/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:vde_fileformat_stbimage>"
        "${VDE_RUNTIME_DIR}/engine/plugins/"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${VDE_RUNTIME_DIR}/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${VDE_ROOT}/App/shaders"
        "${VDE_RUNTIME_DIR}/shaders"
    COMMENT "Copy runtime dependencies"
)

if(VDE_ASSETS_SOURCE_DIR)
    add_custom_command(TARGET vde_app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${VDE_RUNTIME_DIR}/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${VDE_ASSETS_SOURCE_DIR}"
            "${VDE_RUNTIME_DIR}/assets"
        COMMENT "Copy app assets"
    )
elseif(VDE_ASSETS_ARCHIVE)
    add_custom_command(TARGET vde_app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E chdir "${VDE_RUNTIME_DIR}" ${CMAKE_COMMAND} -E tar xf "${VDE_ASSETS_ARCHIVE}"
        COMMENT "Extract app assets"
    )
endif()

find_program(GLSLC_EXECUTABLE glslc
    HINTS
        "${Vulkan_GLSLC_EXECUTABLE}"
        "$ENV{VULKAN_SDK}/Bin"
        "$ENV{VULKAN_SDK}/Bin32"
)

if(GLSLC_EXECUTABLE)
    foreach(shader IN ITEMS base.vert base.frag)
        add_custom_command(
            TARGET vde_app POST_BUILD
            COMMAND ${GLSLC_EXECUTABLE}
                "${VDE_ROOT}/App/shaders/${shader}"
                -o "${VDE_RUNTIME_DIR}/shaders/${shader}.spv"
            COMMENT "Compile shader ${shader}"
        )
    endforeach()
else()
    message(WARNING "glslc introuvable: les shaders SPIR-V devront etre generes manuellement.")
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT vde_app)
