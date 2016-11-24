# glcp
OpenGL core profile extension library

# How to generate
1.サイト<https://www.opengl.org/registry/>から glcorearb.h というファイルをダウンロードします
2.glcorearb.hをglディレクトリへコピー
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

---
Shun Moriya http://mnu.sakura.ne.jp
