#ifndef GLCP_TEST_GLX_CONTEXT_H
#define GLCP_TEST_GLX_CONTEXT_H

struct glcp_test_glx_context {
	void* display;
	unsigned long window;
	unsigned long colormap;
	void* visual;
	void* context;
};

int glcp_test_glx_create_context(struct glcp_test_glx_context* glx);
void glcp_test_glx_destroy_context(struct glcp_test_glx_context* glx);

#endif
