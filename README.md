# Vulkan C++ Engine

> Edited from [littleVulkanEngine](https://github.com/blurrypiano/littleVulkanEngine).

Original purpose: to learn Vulkan and C++

Will try to make this project a simple 2D particle physics based game engine.

Currently working on MPM implementation.

## Build Settings

Compiler: MSVC (Visual Studio 2022)

Platform: Windows Only (for now)

Build command:

```
cmake -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE --no-warn-unused-cli -SC:/Users/Kyr-PC/Documents/Projects/VulkanCppEngine -Bc:/Users/Kyr-PC/Documents/Projects/VulkanCppEngine/build -G "Visual Studio 17 2022" -T host=x64 -A x6
```

## TODO

-   Separate global ubo etc. from gpu resources.
-   Separate private and public into more detailed sections.
