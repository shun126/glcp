# glcp
OpenGL core profile extension library

# How to generate

1.�T�C�g<https://www.opengl.org/registry/>���� glcorearb.h �Ƃ����t�@�C�����_�E�����[�h���܂�
2.glcp.rb�Ɠ����f�B���N�g���փR�s�[
3.glcp.rb�����s
`ruby glcp.rb`

# How to use
glcp�f�B���N�g���ɐ������ꂽglcp.h��glcp.c��ΏۃA�v���P�[�V�����̃v���W�F�N�g�ɒǉ����āA
�����_�����O�R���e�L�X�g�������glcpInitialize���ĂԂ��Ƃŗ��p�ł��܂��B

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
