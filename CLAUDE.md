# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

USEngine (VDE - Vulkan Development Engine) is a modern C++ graphics engine built on Vulkan. It uses a clean abstraction layer over Vulkan APIs with a plugin system for extensibility. The project follows C++23 standards and uses Visual Studio 2022 (v143 toolset).

## Build System

The project uses Visual Studio solution/project files (MSBuild):

- **Build the entire solution:**
  ```
  msbuild Engine.sln /p:Configuration=Debug /p:Platform=x64
  ```

- **Build release:**
  ```
  msbuild Engine.sln /p:Configuration=Release /p:Platform=x64
  ```

- **Build a specific project:**
  ```
  msbuild Engine/Engine.vcxproj /p:Configuration=Debug /p:Platform=x64
  ```

**Build outputs:**
- Compiled libraries → `lib/` directory (Engine_Debug.lib, ImGui_Debug.lib, etc.)
- Executables → `bin/` directory (App_Debug.exe, App_Release.exe)
- Intermediate build files → `build/` directory

**Note:** Shader compilation happens automatically during App project build via custom build steps that invoke `$(VULKAN_SDK)\Bin\glslc.exe` to compile `.vert` and `.frag` files to `.spv` SPIR-V binaries in the `bin/` directory.

## Dependencies

**Required:**
- Vulkan SDK (must have `VULKAN_SDK` environment variable set)
- Windows 10 SDK

**External libraries** (in `external/include` and `external/lib`):
- GLFW (windowing)
- GLM (mathematics)
- VkBootstrap (Vulkan initialization)
- VMA (Vulkan Memory Allocator)
- SPIRV-Reflect (shader reflection)
- stb_image (image loading)

## Architecture

### Project Structure

The solution contains these projects:

1. **Engine** (Static Library) - Core engine functionality
2. **ImGui** (Static Library) - ImGui integration with Vulkan backend
3. **App** (Application) - Example application/entry point
4. **Plugins/FileFormat_obj** - OBJ mesh file loader plugin
5. **Plugins/FileFormat_stbimage** - Image loader plugin

### Core Design Patterns

**pImpl (Pointer to Implementation):**
The engine extensively uses the pImpl idiom to hide Vulkan implementation details. Public headers expose minimal interfaces while `*_impl_vulkan.h` files in `Engine/src/core/` contain the actual Vulkan implementation.

Example:
- Public: `Engine/include/core/graphicscontext.h` declares `GraphicsContext` with `struct Impl`
- Private: `Engine/src/core/graphicscontext_impl_vulkan.h` defines the actual `GraphicsContext::Impl` structure

This keeps Vulkan types isolated from the public API.

**Badge Pattern:**
Used for access control (see `util/badge.h`). Certain constructors require a `Badge<T>` which can only be created by friend class `T`, ensuring only specific classes can construct objects.

**Plugin System:**
Extensible via `util/plugin.h` and `util/pluginregistry.h`. Plugins inherit from `Plugin<EPluginFamily>` base class. Currently supports `FileFormat` plugins for loading assets.

### Key Components

**GPU Abstraction Layer** (`Engine/include/core/gpu/`):
- `Buffer` - GPU buffers with upload/download capabilities
- `Shader` - SPIR-V shader modules with reflection via SPIRV-Reflect
- `Pipeline` - Graphics and compute pipelines
- `Image` - Textures and render targets
- `CommandBuffer`, `CommandPool` - Command recording and submission
- `DescriptorPool`, `DescriptorSet` - Resource binding

**Assets** (`Engine/include/core/assets/`):
- `Asset<T>` - Type-safe asset handles
- `AssetSource` - Base for loading asset data (file, memory, etc.)

**ShaderDataStore:**
A reflection-based system for shader uniform/push constant data. The engine uses SPIRV-Reflect to introspect shader layouts and provides type-safe access via `ShaderDataStore` and `ShaderDataProxy` (see `core/gpu/shader.h`).

Example from App/src/main.cpp:62-64:
```cpp
ShaderDataStore cameraDataStore(vs->PushConstants());
cameraDataStore["projection"] = glm::perspective(...);
```

### Graphics Context Lifecycle

1. Create `Window` (GLFW wrapper)
2. Create `GraphicsContext` with window (initializes Vulkan device, swapchain, command pools)
3. Create GPU resources (buffers, shaders, pipelines)
4. Render loop:
   - `CommandPool().Acquire()` to get command buffer
   - `cmdBuffer.Record()` returns `CommandEncoder`
   - Record rendering commands
   - `Submit(cmdBuffer)` and `Present()`
5. `WaitForIdle()` before cleanup
6. Destroy resources in reverse order

**Important:** The engine uses frame overlap (2 frames in flight). Resources must be properly synchronized.

## Common Tasks

**Compile shaders manually:**
```
%VULKAN_SDK%\Bin\glslc.exe App/shaders/base.vert -o bin/shaders/base.vert.spv
```

**Run the application:**
```
bin\App_Debug.exe
```
or
```
bin\App_Release.exe
```

**Clean build artifacts:**
```
rmdir /s /q bin build lib
```

## Code Style Notes

- C++23 features are available (project uses `stdcpp23` language standard)
- Vulkan implementation details are in `*_impl_vulkan.h/.cpp` files
- Use `vde::core`, `vde::core::gpu`, `vde::graphics`, `vde::util` namespaces
- Header guards use `VDE__NAMESPACE__FILE_H` pattern
- Include paths use angle brackets from project roots (e.g., `<core/graphicscontext.h>`)
