#ifndef GLCP_TEST_WIN_CONTEXT_H
#define GLCP_TEST_WIN_CONTEXT_H

#include <windows.h>

struct glcp_test_win_context {
	HWND window;
	HDC device_context;
	HGLRC rendering_context;
};

static LRESULT CALLBACK glcp_test_window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	(void)wparam;
	(void)lparam;
	return DefWindowProcA(window, message, wparam, lparam);
}

static int glcp_test_win_create_context(struct glcp_test_win_context* context)
{
	WNDCLASSA window_class;
	PIXELFORMATDESCRIPTOR pfd;
	int pixel_format;
	HINSTANCE instance;

	if (context == NULL) {
		return 0;
	}

	context->window = NULL;
	context->device_context = NULL;
	context->rendering_context = NULL;

	instance = GetModuleHandleA(NULL);
	ZeroMemory(&window_class, sizeof(window_class));
	window_class.style = CS_OWNDC;
	window_class.lpfnWndProc = glcp_test_window_proc;
	window_class.hInstance = instance;
	window_class.lpszClassName = "glcp_test_window";

	if (RegisterClassA(&window_class) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
		return 0;
	}

	context->window = CreateWindowA(
		window_class.lpszClassName,
		"glcp test",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		64,
		64,
		NULL,
		NULL,
		instance,
		NULL
	);
	if (context->window == NULL) {
		return 0;
	}

	context->device_context = GetDC(context->window);
	if (context->device_context == NULL) {
		return 0;
	}

	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;

	pixel_format = ChoosePixelFormat(context->device_context, &pfd);
	if (pixel_format == 0) {
		return 0;
	}
	if (!SetPixelFormat(context->device_context, pixel_format, &pfd)) {
		return 0;
	}

	context->rendering_context = wglCreateContext(context->device_context);
	if (context->rendering_context == NULL) {
		return 0;
	}
	if (!wglMakeCurrent(context->device_context, context->rendering_context)) {
		return 0;
	}

	return 1;
}

static void glcp_test_win_destroy_context(struct glcp_test_win_context* context)
{
	if (context == NULL) {
		return;
	}

	wglMakeCurrent(NULL, NULL);

	if (context->rendering_context != NULL) {
		wglDeleteContext(context->rendering_context);
		context->rendering_context = NULL;
	}
	if (context->device_context != NULL && context->window != NULL) {
		ReleaseDC(context->window, context->device_context);
		context->device_context = NULL;
	}
	if (context->window != NULL) {
		DestroyWindow(context->window);
		context->window = NULL;
	}
}

#endif
