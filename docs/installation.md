# Installation Guide

Complete installation instructions for the Disjoint Interval Set library.

## Overview

DIS is a **header-only library** with **zero dependencies**. Installation simply involves making the headers available to your compiler.

## Requirements

### Compiler Requirements

- **C++17 or later** (uses `std::optional`, structured bindings)
- **C++20 recommended** (for full ranges support and concepts)
- One of the following compilers:
  - GCC 9.0 or later
  - Clang 10.0 or later
  - MSVC 2019 (19.20) or later
  - Apple Clang 12.0 or later

### Operating Systems

Tested on:

- Linux (Ubuntu 20.04+, Fedora 33+, Arch Linux)
- macOS 11.0+ (Big Sur and later)
- Windows 10/11 (with MSVC or MinGW-w64)
- FreeBSD 13.0+

### Dependencies

#### Library Usage
**None!** DIS has zero external dependencies beyond the C++ standard library for library usage.

#### Building and Testing (Optional)
If you want to build and run the test suite:
- **CMake 3.14+** for build system
- **Google Test** for running tests
- **gcovr** (optional) for HTML coverage reports: `pip install gcovr`

## Installation Methods

### Method 1: Copy Headers (Simplest)

The fastest way to start using DIS:

```bash
# Clone the repository
git clone https://github.com/yourusername/disjoint_interval_set.git

# Copy headers to your project's include directory
cp -r disjoint_interval_set/include/dis /path/to/your/project/include/
```

Then include in your code:

```cpp
#include <dis/dis.hpp>  // Main header (includes everything)

// Or include specific components
#include <dis/core/interval.hpp>
#include <dis/core/disjoint_interval_set.hpp>
```

### Method 2: Git Submodule

For projects using Git:

```bash
# Add DIS as a submodule
git submodule add https://github.com/yourusername/disjoint_interval_set.git external/dis
git submodule update --init --recursive
```

#### With CMake

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.14)
project(MyProject)

# Add DIS include directory
include_directories(external/dis/include)

# Your executable
add_executable(my_app main.cpp)
target_compile_features(my_app PRIVATE cxx_std_17)
```

#### With Makefile

```makefile
# Makefile
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2
INCLUDES = -Iexternal/dis/include

my_app: main.cpp
    $(CXX) $(CXXFLAGS) $(INCLUDES) main.cpp -o my_app
```

### Method 3: System-Wide Installation

Install headers system-wide (requires root):

#### Linux/macOS

```bash
# Clone repository
git clone https://github.com/yourusername/disjoint_interval_set.git
cd disjoint_interval_set

# Install to /usr/local/include
sudo cp -r include/dis /usr/local/include/

# Or to a custom prefix
PREFIX=$HOME/.local
mkdir -p $PREFIX/include
cp -r include/dis $PREFIX/include/
```

#### Windows (Administrator PowerShell)

```powershell
# Clone repository
git clone https://github.com/yourusername/disjoint_interval_set.git
cd disjoint_interval_set

# Copy to MSVC include directory
$VS_PATH = "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\<version>\include"
Copy-Item -Recurse include\dis "$VS_PATH\dis"
```

### Method 4: Package Managers

#### Conan (Coming Soon)

```bash
# In conanfile.txt
[requires]
disjoint_interval_set/1.0.0

[generators]
cmake
```

#### vcpkg (Coming Soon)

```bash
vcpkg install disjoint-interval-set
```

## Verification

### Test Installation

Create `test.cpp`:

```cpp
#include <dis/dis.hpp>
#include <iostream>

int main() {
    using namespace dis;

    auto interval = real_interval::closed(0, 10);
    std::cout << "Interval: " << interval << '\n';
    std::cout << "Contains 5: " << std::boolalpha << interval.contains(5) << '\n';

    return 0;
}
```

Compile and run:

```bash
# GCC/Clang
g++ -std=c++17 test.cpp -o test
./test

# Expected output:
# Interval: [0,10]
# Contains 5: true
```

### Run Test Suite

To verify the installation with the full test suite:

#### Step 1: Install Google Test

```bash
# Ubuntu/Debian
sudo apt-get install libgtest-dev cmake

# macOS with Homebrew
brew install googletest

# Fedora/RHEL
sudo dnf install gtest-devel cmake

# Or build from source
git clone https://github.com/google/googletest.git
cd googletest
mkdir build && cd build
cmake ..
make
sudo make install
```

#### Step 2: Build and Run Tests

```bash
# Clone repository
git clone https://github.com/yourusername/disjoint_interval_set.git
cd disjoint_interval_set

# Build tests
mkdir build && cd build
cmake ..
make

# Run tests
ctest --verbose

# Or use the convenience target
make run_tests

# Generate coverage report (requires gcovr: pip install gcovr)
make coverage
# Opens: build/coverage/index.html
```

Expected output:
```
Test project /path/to/build
    Start 1: test_interval
1/8 Test #1: test_interval ....................   Passed    0.02 sec
    Start 2: test_disjoint_interval_set
2/8 Test #2: test_disjoint_interval_set .......   Passed    0.03 sec
    Start 3: test_algorithms
3/8 Test #3: test_algorithms ..................   Passed    0.02 sec
    Start 4: test_integration
4/8 Test #4: test_integration .................   Passed    0.02 sec
    Start 5: test_elegant_api
5/8 Test #5: test_elegant_api .................   Passed    0.03 sec
    Start 6: test_interval_comprehensive
6/8 Test #6: test_interval_comprehensive ......   Passed    0.05 sec
    Start 7: test_dis_comprehensive
7/8 Test #7: test_dis_comprehensive ...........   Passed    0.08 sec
    Start 8: test_parser_formatter_comprehensive
8/8 Test #8: test_parser_formatter_comprehensive Passed    0.04 sec

100% tests passed, 0 tests failed out of 8
```

## IDE Setup

### Visual Studio Code

Create `.vscode/c_cpp_properties.json`:

```json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/**",
                "${workspaceFolder}/external/dis/include"
            ],
            "defines": [],
            "compilerPath": "/usr/bin/g++",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "linux-gcc-x64"
        }
    ],
    "version": 4
}
```

### CLion

CMake configuration is automatically detected from `CMakeLists.txt`.

### Visual Studio

1. Open project properties
2. Go to C/C++ → General → Additional Include Directories
3. Add path to `dis/include`
4. Go to C/C++ → Language → C++ Language Standard
5. Set to ISO C++17 or later

### Xcode

1. Select target → Build Settings
2. Search for "Header Search Paths"
3. Add path to `dis/include`
4. Search for "C++ Language Dialect"
5. Set to C++17 or later

## Build System Integration

### CMake

```cmake
cmake_minimum_required(VERSION 3.14)
project(MyProject CXX)

# Option 1: Add as subdirectory (if using git submodule)
add_subdirectory(external/dis)

# Option 2: Find installed package
find_package(dis REQUIRED)

# Your target
add_executable(my_app src/main.cpp)
target_link_libraries(my_app PRIVATE dis::dis)
target_compile_features(my_app PRIVATE cxx_std_17)
```

### Meson

```meson
project('myproject', 'cpp',
  default_options: ['cpp_std=c++17'])

dis_dep = declare_dependency(
  include_directories: include_directories('external/dis/include'))

executable('my_app',
  sources: 'main.cpp',
  dependencies: [dis_dep])
```

### Bazel

```python
# WORKSPACE
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "dis",
    remote = "https://github.com/yourusername/disjoint_interval_set.git",
    tag = "v1.0.0",
)

# BUILD
cc_binary(
    name = "my_app",
    srcs = ["main.cpp"],
    deps = ["@dis//:dis"],
)
```

## Advanced Configuration

### Compiler Flags

#### Optimization

```bash
# Development (fast compilation, debug info)
g++ -std=c++17 -O0 -g main.cpp

# Release (maximum optimization)
g++ -std=c++17 -O3 -DNDEBUG main.cpp

# With native CPU optimizations
g++ -std=c++17 -O3 -march=native main.cpp
```

#### Warnings

```bash
# Recommended warning flags
g++ -std=c++17 -Wall -Wextra -Wpedantic main.cpp

# Stricter warnings
clang++ -std=c++17 -Weverything -Wno-c++98-compat main.cpp
```

### Custom Namespace

By default, all DIS symbols are in the `dis` namespace. To avoid conflicts:

```cpp
// Option 1: Use namespace alias
namespace my_dis = dis;

// Option 2: Selective imports
using dis::real_interval;
using dis::real_set;
```

### Static vs Dynamic Linking

DIS is header-only, so there's no choice between static and dynamic linking. However, if you create a wrapper library:

```cmake
# Create a static library wrapper (optional)
add_library(dis_wrapper STATIC src/dis_wrapper.cpp)
target_include_directories(dis_wrapper PUBLIC external/dis/include)
target_compile_features(dis_wrapper PUBLIC cxx_std_17)
```

## Troubleshooting

### Common Issues

#### Issue: "dis/dis.hpp: No such file or directory"

**Solution**: Ensure include path is set correctly:

```bash
# Check include path
g++ -std=c++17 -I./include test.cpp

# Or for system-wide installation
g++ -std=c++17 -I/usr/local/include test.cpp
```

#### Issue: C++17 features not available

**Solution**: Explicitly enable C++17:

```bash
# GCC/Clang
g++ -std=c++17 ...
clang++ -std=c++17 ...

# MSVC
cl /std:c++17 ...
```

#### Issue: Linker errors with std::optional

**Solution**: On some older systems, may need to link against C++ filesystem library:

```bash
g++ -std=c++17 test.cpp -lstdc++fs
```

#### Issue: Slow compilation times

**Solutions**:

1. Use precompiled headers
2. Include only what you need:
   ```cpp
   #include <dis/core/interval.hpp>  // Instead of <dis/dis.hpp>
   ```
3. Enable ccache:
   ```bash
   export CXX="ccache g++"
   ```

### Platform-Specific Issues

#### macOS: Xcode Command Line Tools

Ensure Xcode CLT are installed:

```bash
xcode-select --install
```

#### Windows: MinGW PATH Issues

Add MinGW to PATH:

```powershell
$env:Path += ";C:\mingw64\bin"
```

#### Linux: Old GCC Version

Update GCC on Ubuntu/Debian:

```bash
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install gcc-11 g++-11
```

## Next Steps

Once installed, proceed to:

- **[Getting Started Guide](getting-started.md)**: Your first program
- **[API Reference](api/interval.md)**: Detailed documentation
- **[Examples](examples.md)**: Real-world use cases

## Uninstallation

To remove DIS:

```bash
# If installed system-wide
sudo rm -rf /usr/local/include/dis

# If using git submodule
git submodule deinit external/dis
git rm external/dis
rm -rf .git/modules/external/dis
```
