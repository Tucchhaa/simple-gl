# SimpleGL

This is a simple cross-platform OpenGL engine. 

## Instructions

### Development Environment Setup
1. Install `cmake [CMake download page](https://cmake.org/download/)
2. Install `vcpkg` in project root. [vcpkg Getting Started page](https://learn.microsoft.com/ru-ru/vcpkg/get_started/get-started?pivots=shell-bash) 
3. Run `./vcpkg/vcpkg install` to install dependencies
4. Run `mkdir build && cd build && cmake ..`

### Build and Run
1. Run `cmake --build .` to build
2. Run `./main` to run

### Logger Helper
`helpers/logger.*` can dump any node hierarchy to a text file. Create a logger with a filesystem path and call `logNode` on the subtree you want to inspect.

```cpp
#include "helpers/logger.h"

auto root = Engine::instance().scene()->rootNode();
SimpleGL::Logger logger("scene_dump.txt");
logger.logNode(root);
```

Each line contains the node name plus its component list, with box-drawing characters indicating parent/child relationships. The output lives at the path you pass to the constructor.

### Smoke Test
`tests/test.cpp` hosts `runLoggerSmokeTest()`, used in `main.cpp`. It builds a toy graph and fails the program if the logger output changes unexpectedly. On a successful run it stays silent. To purposely see it fail, tweak the expected string in the test, rebuild, and run `./main`; the thrown exception will show the exact difference between expected and actual output. Undo the tweak afterwards.
