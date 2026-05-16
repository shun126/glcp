# glcp

[![license](https://img.shields.io/github/license/shun126/glcp)](https://github.com/shun126/glcp/blob/main/LICENSE)
[![release](https://img.shields.io/github/v/release/shun126/glcp)](https://github.com/shun126/glcp/releases)
[![downloads](https://img.shields.io/github/downloads/shun126/glcp/total)](https://github.com/shun126/glcp/releases)
[![stars](https://img.shields.io/github/stars/shun126/glcp?style=social)](https://github.com/shun126/glcp/stargazers)

`glcp` is a lightweight helper library for engineers who want to use Desktop OpenGL APIs from C or C++ without pulling in a larger dependency stack. It provides generated Core Profile declarations together with runtime function loading, and also ships a legacy-friendly loader for fixed-function style code.

The normal way to use `glcp` is to download a released ZIP package and add the shipped source files to your project. Use `glcp.h` / `glcp.c` for Core Profile code, or `glcp_compat.h` / `glcp_compat.c` for legacy/compatibility-style code. Generating these files from `glcp.rb` is optional and mainly relevant for maintainers or contributors.

## What glcp is

- OpenGL Core Profile declarations and loading helper for C and C++ projects
- Legacy/compatibility-style loader for fixed-function and matrix/attrib-stack code
- Based on Khronos `external/gl/glcorearb.h`
- Includes a Core Profile loader and a separate legacy-friendly loader
- Intended for Desktop OpenGL Core on Windows, Linux, and macOS
- Not intended for iOS or Android OpenGL ES environments

## Get the release package

Download the latest release ZIP from the repository Releases page and use the packaged files directly.

The release ZIP includes:

- `glcp/glcp.c`
- `glcp/glcp.h`
- `glcp/glcp_compat.c`
- `glcp/glcp_compat.h`
- `glcp/README.md`
- `LICENSE.txt`

If you only want to integrate `glcp` into an application, you do not need to clone this repository or run the generator.

## Use in your project

1. Extract the release ZIP.
2. Choose one profile and add the matching files to your project.
   - Core Profile: `glcp/glcp.c` and `glcp/glcp.h`
   - Legacy/compatibility-style usage: `glcp/glcp_compat.c` and `glcp/glcp_compat.h`
3. Create and make current a valid OpenGL context.
4. Call the matching initialize function after the context is current.
   - Core Profile: `glcpInitialize()`
   - Legacy/compatibility-style usage: `glcpCompatInitialize()`
5. Use the matching OpenGL API set through the loaded entry points.
6. Optionally call the matching finalize function before application shutdown.

### Minimal examples:

#### Windows:

```cpp
#include "glcp/glcp.h"
#pragma comment(lib, "opengl32.lib")

void InitGL(HDC dc)
{
    HGLRC glRC = wglCreateContext(dc);
    wglMakeCurrent(dc, glRC);

    glcpInitialize();

    // Use OpenGL Core Profile functions here.

    glcpFinalize();
}
```

#### Windows Legacy/Compatibility-style:

```cpp
#include "glcp/glcp_compat.h"
#pragma comment(lib, "opengl32.lib")

void InitCompatGL(HDC dc)
{
    HGLRC glRC = wglCreateContext(dc);
    wglMakeCurrent(dc, glRC);

    glcpCompatInitialize();

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPopAttrib();

    glcpCompatFinalize();
}
```

#### Linux (GLX):

```cpp
#include "glcp/glcp.h"

void InitGL(Display* display, GLXDrawable drawable, GLXContext context)
{
    glXMakeCurrent(display, drawable, context);

    glcpInitialize();

    // Use OpenGL Core Profile functions here.

    glcpFinalize();
}
```

#### macOS:

```cpp
#include "glcp/glcp.h"

void InitGL(NSOpenGLContext* context)
{
    [context makeCurrentContext];

    glcpInitialize();

    // Use OpenGL Core Profile functions here.

    glcpFinalize();
}
```

## Runtime notes

- `glcpInitialize()` and `glcpCompatInitialize()` must run only after a current Desktop OpenGL context exists on the calling thread.
- If the current OpenGL context is recreated or replaced, run the matching initialize function again for that context.
- `glcp.h` / `glcp.c` are built from `glcorearb.h`, so they are aimed at OpenGL Core Profile declarations rather than compatibility-only APIs.
- `glcp_compat.h` / `glcp_compat.c` target legacy/compatibility-style usage by combining platform `gl.h` declarations with dynamically loaded entry points where the platform requires them.
- On macOS, `glcp_compat` corresponds to Apple legacy OpenGL profile usage rather than a Khronos Compatibility Profile.
- On macOS, create a legacy context with `NSOpenGLProfileVersionLegacy` when using `glcp_compat`.
- Apple deprecated OpenGL on macOS 10.14, but the legacy profile remains the relevant mode for fixed-function style code.
- The generated loader uses platform-specific Desktop OpenGL entry-point resolution: Windows via WGL, Linux via GLX, and macOS via the OpenGL framework.
- iOS and Android are out of scope for this generator because they are typically OpenGL ES platforms rather than Desktop OpenGL Core platforms.

## Versioning

`glcp` version numbers follow the supported OpenGL Core version in the generated header:

- `glcp = <OpenGL major>.<OpenGL minor>.<glcp release>`
- Example: OpenGL `2.1` => glcp `2.1.0`

Current repository state: **0.0** while `external/gl/glcorearb.h` is still a placeholder.

## Generate locally (optional)

This section is for contributors or maintainers who want to regenerate `glcp.c`, `glcp.h`, `glcp_compat.c`, and `glcp_compat.h`. It is not the normal integration path for application engineers.

1. Download official `glcorearb.h` from <https://registry.khronos.org/OpenGL/api/GL/glcorearb.h>.
2. Download official `khrplatform.h` from <https://registry.khronos.org/EGL/api/KHR/khrplatform.h>.
3. Place them at:
   - `external/gl/glcorearb.h`
   - `external/KHR/khrplatform.h`
4. Run:

```bash
ruby glcp.rb
```

Official compile and runtime smoke validation for generated sources is handled by the `generate-and-package.yml` and `generate-and-test.yml` GitHub Actions workflows after generation.

## Smoke tests

This repository also includes native smoke tests under `tests/`, grouped by target platform.

- Compile smoke tests validate header/API shape for both `glcp` and `glcp_compat`.
- Runtime smoke tests create a real native OpenGL context on each platform, call the matching initialize/finalize entry point, inspect the generated function-pointer inventory, and exercise safe representative API calls.
- These tests are integration-oriented checks for loader correctness rather than a classic unit-test suite.

Build and run them locally with:

```bash
cmake -S . -B build-tests
cmake --build build-tests
ctest --test-dir build-tests --output-on-failure
```

### One-step update from OpenGL Registry

```bash
bash .github/scripts/generate_from_registry.sh
```

## Release automation

This repository includes GitHub Actions automation to regenerate sources, package distribution ZIP files, and publish release assets for released versions.

## Star History

<a href="https://www.star-history.com/?repos=shun126%2Fglcp&type=timeline&legend=bottom-right">
 <picture>
   <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/chart?repos=shun126/glcp&type=timeline&theme=dark&legend=bottom-right" />
   <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/chart?repos=shun126/glcp&type=timeline&legend=bottom-right" />
   <img alt="Star History Chart" src="https://api.star-history.com/chart?repos=shun126/glcp&type=timeline&legend=bottom-right" />
 </picture>
</a>

If this project helps your development, please consider giving it a star⭐.
