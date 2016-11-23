# glcp
OpenGL core profile extension library

# How to generate

1.サイト<https://www.opengl.org/registry/>から glcorearb.h というファイルをダウンロードします
2.glcp.rbと同じディレクトリへコピー
3.glcp.rbを実行
`ruby glcp.rb`

# How to use
glcpディレクトリに生成されたglcp.hとglcp.cを対象アプリケーションのプロジェクトに追加して、
レンダリングコンテキスト生成後にglcpInitializeを呼ぶことで利用できます。

## For Visual C++
    #include "glcp/glcp.h"
    #pragma comment(lib, "opengl32.lib")
    
    void function() {	
        HGLRC glRC = wglCreateContext(dc);
        wglMakeCurrent(dc, glRC);
        glcpInitialize();
            ;
        glcpFinalize();
    }

# License
The zlib/libpng License
Copyright (c) 2002 Shun Moriya <shun@mnu.sakura.ne.jp>

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from
the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

1. The origin of this software must not be misrepresented;
   you must not claim that you wrote the original software.
   If you use this software in a product, an acknowledgment in the product
   documentation would be appreciated but is not required.

2. Altered source versions must be plainly marked as such,
   and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.

---
Shun Moriya http://mnu.sakura.ne.jp
