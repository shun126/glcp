# glcp
OpenGL core profile extension library

Copyright (c) 2013 Shun Moriya

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software in
   a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not
   be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
   distribution.

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
���g�p���@

���L�T�C�g���� glcorearb.h �Ƃ����t�@�C�����_�E�����[�h���܂�
 https://www.opengl.org/registry/

glcp.rb�Ɠ����f�B���N�g���փR�s�[

glcp.rb�����s
 ruby glcp.rb

glcp�f�B���N�g���ɐ������ꂽglcp.h��glcp.c��ΏۃA�v���P�[�V�����̃v���W�F�N�g�ɒǉ����āA
�����_�����O�R���e�L�X�g�������glcpInitialize���ĂԂ��Ƃŗ��p�ł��܂��B

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
Sample code
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
For Visual C++

#include "glcp/glcp.h"
#pragma comment(lib, "opengl32.lib")

HGLRC glRC = wglCreateContext(dc);
wglMakeCurrent(dc, glRC);
glcpInitialize();
    ;
glcpFinalize();
