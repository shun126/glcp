#include "tests/common/test_common.h"
#include "tests/glx/glx_context.h"
#include "glcp/glcp.h"
#include "tests/common/core_runtime_coverage.h"

int main(void)
{
	struct glcp_test_glx_context context;

	TEST_ASSERT(glcp_test_glx_create_context(&context));

	glcpInitialize();

	TEST_ASSERT(glcp_test_report_core_inventory());
	TEST_ASSERT(glcp_test_report_core_versioned_inventory());
	TEST_ASSERT(glcp_test_expect_core_functions_loaded());
	TEST_ASSERT(glcp_test_run_common_runtime_calls());

	TEST_ASSERT(glClearColor != NULL);
	TEST_ASSERT(glClear != NULL);

	glClearColor(0.0f, 0.25f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	if (glCreateShader != NULL && glDeleteShader != NULL) {
		GLuint shader = glCreateShader(GL_VERTEX_SHADER);
		if (shader != 0) {
			glDeleteShader(shader);
		}
	}

	glcpFinalize();
	TEST_ASSERT(glcp_test_expect_core_finalized());

	glcp_test_glx_destroy_context(&context);
	return 0;
}
