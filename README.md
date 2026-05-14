# glcp
OpenGL core profile extension library for Windows.

## Supported OpenGL version
Current repository state: **0.0** (placeholder header in `gl/glcorearb.h`).
When a valid official `glcorearb.h` is supplied, `glcp` version follows:

- `glcp = <OpenGL major>.<OpenGL minor>.<glcp release>`
- Example: OpenGL `2.1` => glcp `2.1.0`

## Local generation
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

## Usage
Add `glcp/glcp.h` and `glcp/glcp.c` to your project, then call `glcpInitialize()` after creating and activating an OpenGL context.

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

Encoding: UTF-8
Line ending: CRLF

---
Shun Moriya http://mnu.sakura.ne.jp
