# glcp Usage Guide

`glcp.c` and `glcp.h` provide runtime loading for OpenGL core-profile functions on Windows.

## What this does

- Declares OpenGL function pointers in `glcp.h`
- Resolves each pointer in `glcpInitialize()` using `wglGetProcAddress`
- Provides `glcpFinalize()` as a cleanup hook (currently no-op)

## Integration steps

1. Add these files to your project:
   - `glcp/glcp.h`
   - `glcp/glcp.c`
2. Include the header in a translation unit where you initialize OpenGL:

```cpp
#include "glcp/glcp.h"
#pragma comment(lib, "opengl32.lib")
```

3. Create and make current a valid OpenGL rendering context.
4. Call `glcpInitialize()` **after** the context is current.
5. Use OpenGL core-profile APIs through the loaded function pointers.
6. Optionally call `glcpFinalize()` before application shutdown.

## Minimal example

```cpp
#include "glcp/glcp.h"
#pragma comment(lib, "opengl32.lib")

void InitGL(HDC dc)
{
    HGLRC glRC = wglCreateContext(dc);
    wglMakeCurrent(dc, glRC);

    glcpInitialize();

    // Example: call loaded OpenGL functions here.

    glcpFinalize();
}
```

## Important notes

- `glcpInitialize()` must run on a thread with a current OpenGL context.
- If you recreate the context, call `glcpInitialize()` again.
- This project targets Windows (`windows.h`, `wglGetProcAddress`).
- Supported OpenGL and glcp versions are documented in the repository root README and CHANGELOG.

## Tooltips

- **Initialize timing**: "Call `glcpInitialize()` only after `wglMakeCurrent` succeeds."
- **Context lifecycle**: "If the OpenGL context changes, re-run `glcpInitialize()`."
- **Platform scope**: "This loader uses WGL and is intended for Windows builds."
