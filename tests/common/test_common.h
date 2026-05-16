#ifndef GLCP_TEST_COMMON_H
#define GLCP_TEST_COMMON_H

#include <stdio.h>

#define TEST_ASSERT(expr) \
	do { \
		if (!(expr)) { \
			fprintf(stderr, "%s:%d: assertion failed: %s\n", __FILE__, __LINE__, #expr); \
			return 1; \
		} \
	} while (0)

#define TEST_FAIL(message) \
	do { \
		fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, message); \
		return 1; \
	} while (0)

#endif
