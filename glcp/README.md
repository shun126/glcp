# glcp Usage Guide

`glcp.c` and `glcp.h` are the runtime loader portion of the `glcp` project. They provide generated OpenGL core-profile declarations together with function loading for projects that use WGL on Windows.

## What this provides

- OpenGL declarations generated from Khronos `glcorearb.h`
- Function pointer definitions in `glcp.c`
- Runtime resolution in `glcpInitialize()` through `wglGetProcAddress`
- `glcpFinalize()` as a cleanup hook

## Platform notes

- The repository generation workflow can run outside Windows, including Linux environments.
- The generated loader itself currently targets Windows runtime integration through `windows.h` and `wglGetProcAddress`.

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
4. Call `glcpInitialize()` after the context is current.
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
- Supported OpenGL and `glcp` versions are documented in the repository root `README.md` and `CHANGELOG.md`.

## Tooltips

- **Initialize timing**: "Call `glcpInitialize()` only after `wglMakeCurrent` succeeds."
- **Context lifecycle**: "If the OpenGL context changes, re-run `glcpInitialize()`."
- **Runtime scope**: "This generated loader currently uses WGL for Windows OpenGL contexts."

---
Shun Moriya http://mnu.sakura.ne.jp
