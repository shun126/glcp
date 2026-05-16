#ifndef GLCP_TEST_COMPAT_RUNTIME_COVERAGE_H
#define GLCP_TEST_COMPAT_RUNTIME_COVERAGE_H

#include "tests/common/runtime_coverage.h"

static int glcp_test_report_compat_inventory(void)
{
	struct glcp_test_function_report_state state;

	glcp_test_begin_function_report(&state, "glcp compat inventory");
#undef GLCP_TEST_FUNCTION
#define GLCP_TEST_FUNCTION(function_name) glcp_test_note_function_report(&state, #function_name, ((function_name) != NULL));
#include "glcp_compat_function_inventory.inc"
#undef GLCP_TEST_FUNCTION
	return glcp_test_finish_function_report(&state);
}

static int glcp_test_report_compat_versioned_inventory(void)
{
	struct glcp_test_versioned_report_state state;

	glcp_test_begin_versioned_report(&state, "glcp compat versioned inventory");
#undef GLCP_TEST_VERSION_FUNCTION
#define GLCP_TEST_VERSION_FUNCTION(version_name, function_name) glcp_test_note_versioned_report(&state, #version_name, #function_name, ((function_name) != NULL));
#include "glcp_compat_versioned_function_inventory.inc"
#undef GLCP_TEST_VERSION_FUNCTION
	return glcp_test_finish_versioned_report(&state);
}

static int glcp_test_expect_compat_functions_loaded(void)
{
	int major = 0;
	int minor = 0;
	struct glcp_test_versioned_expect_state state;

	TEST_ASSERT(glcp_test_parse_runtime_gl_version(&major, &minor));
	glcp_test_begin_versioned_expect(&state, "glcp compat runtime load validation", major, minor);
#undef GLCP_TEST_VERSION_FUNCTION
#define GLCP_TEST_VERSION_FUNCTION(version_name, function_name) glcp_test_note_versioned_expect(&state, #version_name, #function_name, ((function_name) != NULL));
#include "glcp_compat_versioned_function_inventory.inc"
#undef GLCP_TEST_VERSION_FUNCTION
	return glcp_test_finish_versioned_expect(&state);
}

static int glcp_test_expect_compat_finalized(void)
{
	struct glcp_test_function_finalize_state state;

	glcp_test_begin_function_finalize(&state, "glcp compat finalize");
#undef GLCP_TEST_FUNCTION
#define GLCP_TEST_FUNCTION(function_name) glcp_test_note_function_finalize(&state, #function_name, ((function_name) != NULL));
#include "glcp_compat_function_inventory.inc"
#undef GLCP_TEST_FUNCTION
	return glcp_test_finish_function_finalize(&state);
}

#endif
