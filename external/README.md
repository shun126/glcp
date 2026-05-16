# `external` directory

This directory stores external Khronos headers used as generator input.

## Required files

- `external/gl/glcorearb.h`
- `external/KHR/khrplatform.h`

## How to obtain the files

1. Download the official `glcorearb.h` header from <https://registry.khronos.org/OpenGL/api/GL/glcorearb.h>.
2. Download the official `khrplatform.h` header from <https://registry.khronos.org/EGL/api/KHR/khrplatform.h>.
3. Save the downloaded files at:
   - `external/gl/glcorearb.h`
   - `external/KHR/khrplatform.h`

After updating the external headers, run the generator from the repository root:

```bash
ruby glcp.rb
```
