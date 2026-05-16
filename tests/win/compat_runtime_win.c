#include "tests/common/test_common.h"
#include "tests/win/win_context.h"
#include "glcp/glcp_compat.h"
#include "tests/common/compat_runtime_coverage.h"

int main(void)
{
	struct glcp_test_win_context context;
	const GLubyte* version;

	TEST_ASSERT(glcp_test_win_create_context(&context));

	glcpCompatInitialize();

	TEST_ASSERT(glcp_test_report_compat_inventory());
	TEST_ASSERT(glcp_test_report_compat_versioned_inventory());
	TEST_ASSERT(glcp_test_expect_compat_functions_loaded());
	TEST_ASSERT(glcp_test_run_common_runtime_calls());
	TEST_ASSERT(glcp_test_run_compat_fixed_function_calls());

	version = glGetString(GL_VERSION);
	TEST_ASSERT(version != NULL);

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPopAttrib();

	if (glGenBuffers != NULL && glBindBuffer != NULL && glDeleteBuffers != NULL) {
		GLuint buffer = 0;
		glGenBuffers(1, &buffer);
		if (buffer != 0) {
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glDeleteBuffers(1, &buffer);
		}
	}

	glcpCompatFinalize();
	TEST_ASSERT(glcp_test_expect_compat_finalized());

	glcp_test_win_destroy_context(&context);
	return 0;
}
