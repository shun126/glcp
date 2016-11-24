# glcp
OpenGL core profile extension library

# How to generate
1.�T�C�g<https://www.opengl.org/registry/>���� glcorearb.h �Ƃ����t�@�C�����_�E�����[�h���܂�
2.glcorearb.h��gl�f�B���N�g���փR�s�[
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

---
Shun Moriya http://mnu.sakura.ne.jp
