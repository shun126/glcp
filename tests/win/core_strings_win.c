#include "tests/common/test_common.h"
#include "tests/win/win_context.h"
#include "glcp/glcp.h"

typedef HGLRC (WINAPI *glcp_test_create_context_attribs_proc)(HDC, HGLRC, const int*);

int main(void)
{
	struct glcp_test_win_context context;
	glcp_test_create_context_attribs_proc create_context;
	HGLRC core_context;
	GLint major = 0, minor = 0, profile = 0;
	const GLubyte* version;
	const GLubyte* extension;
	const int attributes[] = {
		0x2091, 3, /* WGL_CONTEXT_MAJOR_VERSION_ARB */
		0x2092, 3, /* WGL_CONTEXT_MINOR_VERSION_ARB */
		0x9126, 1, /* WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB */
		0
	};

	TEST_ASSERT(glcp_test_win_create_context(&context));
	create_context = (glcp_test_create_context_attribs_proc)wglGetProcAddress("wglCreateContextAttribsARB");
	TEST_ASSERT(create_context != NULL);
	core_context = create_context(context.device_context, NULL, attributes);
	TEST_ASSERT(core_context != NULL);
	TEST_ASSERT(wglMakeCurrent(context.device_context, core_context));
	TEST_ASSERT(wglDeleteContext(context.rendering_context));
	context.rendering_context = core_context;

	glcpInitialize();
	TEST_ASSERT(glGetString != NULL);
	TEST_ASSERT(glGetStringi != NULL);
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);
	TEST_ASSERT(major > 3 || (major == 3 && minor >= 3));
	TEST_ASSERT((profile & GL_CONTEXT_CORE_PROFILE_BIT) != 0);
	version = glGetString(GL_VERSION);
	extension = glGetStringi(GL_EXTENSIONS, 0);
	TEST_ASSERT(version != NULL);
	TEST_ASSERT(extension != NULL);
	TEST_ASSERT(glGetError() == GL_NO_ERROR);
	printf("Core context requested as 3.3: GL_VERSION=%s, first extension=%s\n", version, extension);

	glcpFinalize();
	TEST_ASSERT(glGetString == NULL);
	TEST_ASSERT(glGetStringi == NULL);
	glcp_test_win_destroy_context(&context);
	return 0;
}
