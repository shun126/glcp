#include "tests/common/test_common.h"
#include "tests/glx/glx_context.h"
#include "glcp/glcp_compat.h"
#include "tests/common/compat_runtime_coverage.h"

int main(void)
{
	struct glcp_test_glx_context context;
	const GLubyte* version;

	TEST_ASSERT(glcp_test_glx_create_context(&context));

	glcpCompatInitialize();

	TEST_ASSERT(glcp_test_report_compat_inventory());
	TEST_ASSERT(glcp_test_report_compat_versioned_inventory());
	TEST_ASSERT(glcp_test_expect_compat_functions_loaded());
	TEST_ASSERT(glcp_test_run_common_runtime_calls());
	TEST_ASSERT(glcp_test_run_compat_fixed_function_calls());

	version = glGetString(GL_VERSION);
	TEST_ASSERT(version != NULL);

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glBegin(GL_POINTS);
	glVertex2f(0.0f, 0.0f);
	glEnd();
	glPopAttrib();

	glcpCompatFinalize();
	TEST_ASSERT(glcp_test_expect_compat_finalized());

	glcp_test_glx_destroy_context(&context);
	return 0;
}
