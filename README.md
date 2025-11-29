# HexaEngine.Prism

<p align="center">
  <img width="300" height="300" src="https://raw.githubusercontent.com/HexaEngine/HexaEngine.Prism/main/icon.png">
</p>

A modern, cross-platform graphics abstraction layer written in C++23. Prism provides a unified API for multiple graphics backends, designed primarily for use with the [HexaEngine](https://github.com/HexaEngine/HexaEngine) game engine.

## 🎯 Overview

HexaEngine.Prism is a lightweight, high-performance graphics API abstraction layer that simplifies cross-platform rendering. It offers a clean, modern C++ interface with smart reference counting and a flexible pipeline system.

## ✨ Features

- **Modern C++23**: Leverages the latest C++ features for better performance and cleaner code
- **Cross-Platform**: Write once, deploy everywhere
- **Multiple Backends**:
  - ✅ **Direct3D 11** (Implemented)
  - 🚧 **Direct3D 12** (Planned)
  - 🚧 **Vulkan** (Planned)
- **Smart Resource Management**: Automatic reference counting with `PrismObj<T>` smart pointers
- **Flexible Pipeline System**: Support for both graphics and compute pipelines
- **SDL3 Integration**: Window and event handling powered by SDL3
- **Shader Caching**: Built-in shader compilation and caching system
- **Thread-Safe**: Event system and resource management designed for multi-threaded applications

## 🏗️ Architecture

### Core Components

- **GraphicsDevice**: Main entry point for creating resources and pipelines
- **CommandList**: Records and executes rendering commands (immediate and deferred modes)
- **Resources**: Buffers, Textures (1D/2D/3D), with full view support (RTV, DSV, SRV, UAV)
- **Pipelines**: Graphics and compute pipeline states with flexible resource binding
- **Smart Pointers**: `PrismObj<T>` for automatic reference counting

### Resource Management

```cpp
// Smart pointer with automatic reference counting
PrismObj<GraphicsDevice> device = GraphicsDevice::Create();

// Create a texture
Texture2DDesc desc = {};
desc.format = Format::RGBA8_UNorm;
desc.width = 1920;
desc.height = 1080;
desc.arraySize = 1;
desc.mipLevels = 1;
desc.gpuAccessFlags = GpuAccessFlags::RW;
auto texture = device->CreateTexture2D(desc);
```

## 🚀 Getting Started

### Prerequisites

- **CMake**: Version 3.16 or higher
- **C++23 Compiler**: MSVC, GCC, or Clang with C++23 support
- **Ninja**: Build system (automatically configured)
- **Windows**: For Direct3D 11 backend (currently implemented)

### Building

```bash
# Clone the repository
git clone https://github.com/HexaEngine/HexaEngine.Prism.git
cd HexaEngine.Prism

# Configure with CMake
cmake -B build -G Ninja

# Build
cmake --build build

# Build specific target
cmake --build build --target PrismStatic
cmake --build build --target PrismShared
```

### Integration

#### As a Submodule

```bash
git submodule add https://github.com/HexaEngine/HexaEngine.Prism.git external/Prism
```

#### CMake Integration

```cmake
add_subdirectory(external/Prism)

target_link_libraries(YourTarget PRIVATE PrismStatic)
# or
target_link_libraries(YourTarget PRIVATE PrismShared)
```

## 📖 Usage Example

```cpp
#include <graphics.hpp>
#include <SDL3/SDL.h>

using namespace HexaEngine::Prism;

int main() {
    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    SDL_Window* window = SDL_CreateWindow("My App", 1280, 720, SDL_WINDOW_RESIZABLE);
    
    // Create graphics device
    PrismObj<GraphicsDevice> device = GraphicsDevice::Create();
    
    // Create a render target
    Texture2DDesc desc = {};
    desc.format = Format::RGBA8_UNorm;
    desc.width = 1280;
    desc.height = 720;
    desc.gpuAccessFlags = GpuAccessFlags::RW;
    desc.arraySize = 1;
    desc.mipLevels = 1;
    
    auto texture = device->CreateTexture2D(desc);
    
    // Get immediate command list
    CommandList* cmdList = device->GetImmediateCommandList();
    
    // Your rendering code here...
    
    return 0;
}
```

## 🔧 C# Wrapper

A C# wrapper is planned for seamless integration with HexaEngine and .NET-based applications. This will allow C# developers to leverage Prism's capabilities without writing C++ code.

## 🎮 HexaEngine Integration

Prism is designed as the rendering backend for [HexaEngine](https://github.com/HexaEngine/HexaEngine), providing:

- High-performance rendering abstractions
- Multi-backend support for maximum compatibility
- Modern GPU features (compute shaders, UAVs, etc.)
- Flexible resource binding system
- Efficient shader compilation and caching

## 📋 Roadmap

- [ ] **Direct3D 12 Backend**: Modern low-level graphics API support
- [ ] **Vulkan Backend**: Cross-platform low-level graphics API
- [ ] **C# Wrapper**: .NET bindings for HexaEngine integration
- [ ] **Metal Backend**: Native macOS/iOS support (future consideration)
- [ ] **Extended Pipeline Features**: Geometry shaders, tessellation, mesh shaders
- [ ] **Ray Tracing Support**: DXR and Vulkan ray tracing extensions

## 🤝 Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs, feature requests, or improvements.

### Development Guidelines

1. Follow the existing code style (C++23 features encouraged)
2. Ensure all backends remain synchronized in API design
3. Add tests for new features
4. Update documentation as needed

## 📄 License

This project is licensed under the MIT License - see the [LICENSE.txt](LICENSE.txt) file for details.

## 🔗 Links

- **HexaEngine**: [https://github.com/HexaEngine/HexaEngine](https://github.com/HexaEngine/HexaEngine)
- **Issues**: [https://github.com/HexaEngine/HexaEngine.Prism/issues](https://github.com/HexaEngine/HexaEngine.Prism/issues)

## 🙏 Acknowledgments

- SDL3 for cross-platform window and event handling

---

*Built with ❤️ for HexaEngine*
