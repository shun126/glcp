# glcp

`glcp` is a generated OpenGL core-profile loader for C and C++ projects. It packages the declarations and runtime loading code into a small `glcp.h` / `glcp.c` pair, so you can add current OpenGL entry points to an existing codebase without bringing in a larger dependency stack.

## Highlights

- Generated from the official Khronos `glcorearb.h`
- Lightweight integration with `glcp/glcp.h` and `glcp/glcp.c`
- Generation and CI workflow can run in Linux environments
- Generated runtime loader currently uses WGL on Windows

## Platform model

`glcp` has two parts:

- The generator workflow is not Windows-only. The repository CI runs generation on Linux, downloads the latest Khronos header, regenerates sources, and compile-checks the result.
- The generated loader is currently WGL-based at runtime. `glcp/glcp.h` includes `windows.h`, and `glcpInitialize()` resolves functions through `wglGetProcAddress`.

## Quick start

Add `glcp/glcp.h` and `glcp/glcp.c` to your project, create a current OpenGL context, then call `glcpInitialize()`.

```cpp
#include "glcp/glcp.h"
#pragma comment(lib, "opengl32.lib")

void function(HDC dc) {
    HGLRC glRC = wglCreateContext(dc);
    wglMakeCurrent(dc, glRC);

    glcpInitialize();
    // use OpenGL functions
    glcpFinalize();
}
```

## Supported OpenGL version

Current repository state: **0.0** (placeholder header in `gl/glcorearb.h`).
When a valid official `glcorearb.h` is supplied, `glcp` version follows:

- `glcp = <OpenGL major>.<OpenGL minor>.<glcp release>`
- Example: OpenGL `2.1` => glcp `2.1.0`

## Generate locally

1. Download official `glcorearb.h` from <https://registry.khronos.org/OpenGL/api/GL/glcorearb.h>.
2. Place it at `gl/glcorearb.h`.
3. Run:

```bash
ruby glcp.rb
```

The generator also performs a compile check for `glcp/glcp.c` when a supported C compiler is available.

### One-step generation from OpenGL Registry

```bash
bash .github/scripts/generate_from_registry.sh
```

## Runtime integration notes

- Call `glcpInitialize()` only after the OpenGL context becomes current.
- If the current context is recreated or replaced, run `glcpInitialize()` again for that context.
- `glcpFinalize()` is available as the loader cleanup hook.

## CI automation

`.github/workflows/generate-and-package.yml` provides:

- `workflow_dispatch`: manual execution
- `schedule`: weekly execution
- download latest official `glcorearb.h`
- regenerate `glcp/glcp.c` and `glcp/glcp.h`
- compile-check generated `glcp/glcp.c`
- update `CHANGELOG.md`
- auto-commit generated changes when detected
- package distribution ZIP and upload artifact
- publish release asset on tag runs

## Distribution package

`dist/glcp-YYYYMMDD.zip` includes:

- `glcp/glcp.c`
- `glcp/glcp.h`
- `glcp/README.md`
- `LICENSE.txt`

## Tooltips

- **Generation source**: "Use Khronos OpenGL Registry `glcorearb.h` to keep generated bindings current."
- **Initialization timing**: "Call `glcpInitialize()` only after `wglMakeCurrent` succeeds."
- **Version policy**: "glcp version tracks OpenGL major/minor; patch number is glcp release."

---
Shun Moriya http://mnu.sakura.ne.jp
