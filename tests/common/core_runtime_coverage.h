#ifndef GLCP_TEST_CORE_RUNTIME_COVERAGE_H
#define GLCP_TEST_CORE_RUNTIME_COVERAGE_H

#include "tests/common/runtime_coverage.h"

static int glcp_test_report_core_inventory(void)
{
	struct glcp_test_function_report_state state;

	glcp_test_begin_function_report(&state, "glcp core inventory");
#undef GLCP_TEST_FUNCTION
#define GLCP_TEST_FUNCTION(function_name) glcp_test_note_function_report(&state, #function_name, ((function_name) != NULL));
#include "glcp_core_function_inventory.inc"
#undef GLCP_TEST_FUNCTION
	return glcp_test_finish_function_report(&state);
}

static int glcp_test_report_core_versioned_inventory(void)
{
	struct glcp_test_versioned_report_state state;

	glcp_test_begin_versioned_report(&state, "glcp core versioned inventory");
#undef GLCP_TEST_VERSION_FUNCTION
#define GLCP_TEST_VERSION_FUNCTION(version_name, function_name) glcp_test_note_versioned_report(&state, #version_name, #function_name, ((function_name) != NULL));
#include "glcp_core_versioned_function_inventory.inc"
#undef GLCP_TEST_VERSION_FUNCTION
	return glcp_test_finish_versioned_report(&state);
}

static int glcp_test_expect_core_functions_loaded(void)
{
	int major = 0;
	int minor = 0;
	struct glcp_test_versioned_expect_state state;

	TEST_ASSERT(glcp_test_parse_runtime_gl_version(&major, &minor));
	glcp_test_begin_versioned_expect(&state, "glcp core runtime load validation", major, minor);
#undef GLCP_TEST_VERSION_FUNCTION
#define GLCP_TEST_VERSION_FUNCTION(version_name, function_name) glcp_test_note_versioned_expect(&state, #version_name, #function_name, ((function_name) != NULL));
#include "glcp_core_versioned_function_inventory.inc"
#undef GLCP_TEST_VERSION_FUNCTION
	return glcp_test_finish_versioned_expect(&state);
}

static int glcp_test_expect_core_finalized(void)
{
	struct glcp_test_function_finalize_state state;

	glcp_test_begin_function_finalize(&state, "glcp core finalize");
#undef GLCP_TEST_FUNCTION
#define GLCP_TEST_FUNCTION(function_name) glcp_test_note_function_finalize(&state, #function_name, ((function_name) != NULL));
#include "glcp_core_function_inventory.inc"
#undef GLCP_TEST_FUNCTION
	return glcp_test_finish_function_finalize(&state);
}

#endif
