#include "tests/glx/glx_context.h"

#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>

int glcp_test_glx_create_context(struct glcp_test_glx_context* glx)
{
	int attributes[] = {
		GLX_RGBA,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE, 24,
		None
	};
	XSetWindowAttributes swa;
	Display* display;
	XVisualInfo* visual;
	GLXContext context;
	Colormap colormap;
	Window window;

	if (glx == NULL) {
		return 0;
	}

	display = XOpenDisplay(NULL);
	if (display == NULL) {
		return 0;
	}

	visual = glXChooseVisual(display, DefaultScreen(display), attributes);
	if (visual == NULL) {
		XCloseDisplay(display);
		return 0;
	}

	context = glXCreateContext(display, visual, NULL, True);
	if (context == NULL) {
		XFree(visual);
		XCloseDisplay(display);
		return 0;
	}

	colormap = XCreateColormap(
		display,
		RootWindow(display, visual->screen),
		visual->visual,
		AllocNone
	);

	swa.colormap = colormap;
	swa.event_mask = StructureNotifyMask;
	window = XCreateWindow(
		display,
		RootWindow(display, visual->screen),
		0,
		0,
		64,
		64,
		0,
		visual->depth,
		InputOutput,
		visual->visual,
		CWColormap | CWEventMask,
		&swa
	);
	if (window == 0) {
		glXDestroyContext(display, context);
		XFreeColormap(display, colormap);
		XFree(visual);
		XCloseDisplay(display);
		return 0;
	}

	XMapWindow(display, window);
	XSync(display, False);

	if (!glXMakeCurrent(display, window, context)) {
		XDestroyWindow(display, window);
		glXDestroyContext(display, context);
		XFreeColormap(display, colormap);
		XFree(visual);
		XCloseDisplay(display);
		return 0;
	}

	glx->display = display;
	glx->window = window;
	glx->colormap = colormap;
	glx->visual = visual;
	glx->context = context;
	return 1;
}

void glcp_test_glx_destroy_context(struct glcp_test_glx_context* glx)
{
	Display* display;

	if (glx == NULL) {
		return;
	}

	display = (Display*)glx->display;
	if (display != NULL) {
		glXMakeCurrent(display, None, NULL);
	}
	if (glx->context != NULL && display != NULL) {
		glXDestroyContext(display, (GLXContext)glx->context);
		glx->context = NULL;
	}
	if (glx->window != 0 && display != NULL) {
		XDestroyWindow(display, (Window)glx->window);
		glx->window = 0;
	}
	if (glx->colormap != 0 && display != NULL) {
		XFreeColormap(display, (Colormap)glx->colormap);
		glx->colormap = 0;
	}
	if (glx->visual != NULL) {
		XFree((XVisualInfo*)glx->visual);
		glx->visual = NULL;
	}
	if (display != NULL) {
		XCloseDisplay(display);
		glx->display = NULL;
	}
}
