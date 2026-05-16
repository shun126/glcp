#ifndef GLCP_TEST_CGL_CONTEXT_H
#define GLCP_TEST_CGL_CONTEXT_H

#include <OpenGL/OpenGL.h>

struct glcp_test_cgl_context {
	CGLPixelFormatObj pixel_format;
	CGLContextObj context;
};

static int glcp_test_cgl_create_context(struct glcp_test_cgl_context* cgl, CGLPixelFormatAttribute profile)
{
	CGLPixelFormatAttribute attributes[] = {
		kCGLPFAOpenGLProfile,
		profile,
		(CGLPixelFormatAttribute)0
	};
	GLint pixel_format_count = 0;
	CGLError error;

	if (cgl == NULL) {
		return 0;
	}

	cgl->pixel_format = NULL;
	cgl->context = NULL;

	error = CGLChoosePixelFormat(attributes, &cgl->pixel_format, &pixel_format_count);
	if (error != kCGLNoError || cgl->pixel_format == NULL || pixel_format_count <= 0) {
		return 0;
	}

	error = CGLCreateContext(cgl->pixel_format, NULL, &cgl->context);
	if (error != kCGLNoError || cgl->context == NULL) {
		return 0;
	}

	error = CGLSetCurrentContext(cgl->context);
	if (error != kCGLNoError) {
		return 0;
	}

	return 1;
}

static void glcp_test_cgl_destroy_context(struct glcp_test_cgl_context* cgl)
{
	if (cgl == NULL) {
		return;
	}

	CGLSetCurrentContext(NULL);
	if (cgl->context != NULL) {
		CGLDestroyContext(cgl->context);
		cgl->context = NULL;
	}
	if (cgl->pixel_format != NULL) {
		CGLDestroyPixelFormat(cgl->pixel_format);
		cgl->pixel_format = NULL;
	}
}

#endif
