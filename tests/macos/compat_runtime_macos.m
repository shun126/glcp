#include "tests/common/test_common.h"
#include "tests/macos/cgl_context.h"
#include "glcp/glcp_compat.h"
#include "tests/common/compat_runtime_coverage.h"

int main(void)
{
	struct glcp_test_cgl_context context;
	const GLubyte* version;

	TEST_ASSERT(glcp_test_cgl_create_context(&context, (CGLPixelFormatAttribute)kCGLOGLPVersion_Legacy));

	glcpCompatInitialize();

	TEST_ASSERT(glcp_test_report_compat_inventory());
	TEST_ASSERT(glcp_test_report_compat_versioned_inventory());
	TEST_ASSERT(glcp_test_expect_compat_functions_loaded());

	version = glGetString(GL_VERSION);
	TEST_ASSERT(version != NULL);

	glcpCompatFinalize();
	TEST_ASSERT(glcp_test_expect_compat_finalized());

	glcp_test_cgl_destroy_context(&context);
	return 0;
}
