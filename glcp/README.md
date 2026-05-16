# glcp Usage Guide

`glcp.c` / `glcp.h` are the Core Profile runtime loader portion of the `glcp` project. `glcp_compat.c` / `glcp_compat.h` provide a legacy-friendly loader for legacy OpenGL usage on Windows, Linux, and macOS.

## What this provides

- Core Profile declarations generated from Khronos `external/gl/glcorearb.h`
- Legacy/compatibility-style loader declarations that rely on platform `gl.h` for OpenGL 1.0/1.1 and load 1.2+ dynamically
- Runtime resolution in `glcpInitialize()` and `glcpCompatInitialize()` through platform-specific Desktop OpenGL loaders
- `glcpFinalize()` and `glcpCompatFinalize()` as cleanup hooks

## Platform notes

- The repository generation workflow can run outside Windows, including Linux environments.
- The generated loaders target Desktop OpenGL on Windows, Linux, and macOS.
- iOS and Android are not supported by this generator because they typically use OpenGL ES rather than Desktop OpenGL Core.

## Integration steps

1. Add these files to your project:
   - `glcp/glcp.h`
   - `glcp/glcp.c`
   - `glcp/glcp_compat.h`
   - `glcp/glcp_compat.c`
2. Include the header that matches your target profile in a translation unit where you initialize OpenGL:

```cpp
#include "glcp/glcp.h"
#pragma comment(lib, "opengl32.lib")
```

3. Create and make current a valid OpenGL rendering context.
4. Call `glcpInitialize()` for Core Profile or `glcpCompatInitialize()` for legacy/compatibility-style usage after the context is current.
5. Use the matching OpenGL API surface through the loaded entry points.
6. Optionally call the matching finalize function before application shutdown.

## Minimal example

### Windows:

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

### Linux (GLX):

```cpp
#include "glcp/glcp.h"

void InitGL(Display* display, GLXDrawable drawable, GLXContext context)
{
    glXMakeCurrent(display, drawable, context);

    glcpInitialize();

    // Example: call loaded OpenGL functions here.

    glcpFinalize();
}
```

### macOS:

```cpp
#include "glcp/glcp.h"

void InitGL(NSOpenGLContext* context)
{
    [context makeCurrentContext];

    glcpInitialize();

    // Example: call loaded OpenGL functions here.

    glcpFinalize();
}
```

### Compatibility example:

```cpp
#include "glcp/glcp_compat.h"

void InitCompatGL()
{
    glcpCompatInitialize();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPopAttrib();
    glcpCompatFinalize();
}
```

On macOS, use `NSOpenGLProfileVersionLegacy` with `glcp_compat`. Apple deprecated OpenGL on macOS 10.14, but the legacy profile is still the relevant mode for fixed-function style code.

## Important notes

- `glcpInitialize()` and `glcpCompatInitialize()` must run on a thread with a current Desktop OpenGL context.
- If you recreate the context, call the matching initialize function again.
- Supported OpenGL and `glcp` versions are documented in the repository root `README.md` and `CHANGELOG.md`.

## Tooltips

- **Initialize timing**: "Call `glcpInitialize()` or `glcpCompatInitialize()` only after a Desktop OpenGL context becomes current."
- **Context lifecycle**: "If the OpenGL context changes, re-run the matching initialize function."
- **Runtime scope**: "These generated loaders target Desktop OpenGL on Windows, Linux, and macOS."
