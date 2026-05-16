#ifndef GLCP_TEST_RUNTIME_COVERAGE_H
#define GLCP_TEST_RUNTIME_COVERAGE_H

#include "tests/common/test_common.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#if defined(__APPLE__) && defined(___GL_COMPAT_PROFILE_H___)
#define GLCP_TEST_COMPAT_MACOS_LEGACY 1
#else
#define GLCP_TEST_COMPAT_MACOS_LEGACY 0
#endif

struct glcp_test_function_status {
	const char* name;
	int loaded;
};

struct glcp_test_versioned_function_status {
	const char* version;
	const char* name;
	int loaded;
};

struct glcp_test_function_report_state {
	const char* label;
	size_t total;
	size_t loaded;
	size_t missing;
};

struct glcp_test_function_finalize_state {
	const char* label;
	size_t total;
	size_t still_loaded;
};

struct glcp_test_versioned_report_state {
	const char* label;
	const char* current_version;
	size_t version_total;
	size_t version_loaded;
	size_t version_missing;
	size_t total;
	size_t loaded;
	size_t missing;
};

struct glcp_test_versioned_expect_state {
	const char* label;
	const char* current_version;
	size_t version_missing;
	size_t total;
	size_t loaded;
	size_t missing;
	int failed;
	int required_major;
	int required_minor;
};

#define GLCP_TEST_FUNCTION(function_name) { #function_name, ((function_name) != NULL) },
#define GLCP_TEST_VERSION_FUNCTION(version_name, function_name) { #version_name, #function_name, ((function_name) != NULL) },

static void glcp_test_begin_function_report(
	struct glcp_test_function_report_state* state,
	const char* label)
{
	state->label = label;
	state->total = 0;
	state->loaded = 0;
	state->missing = 0;
}

static void glcp_test_note_function_report(
	struct glcp_test_function_report_state* state,
	const char* name,
	int loaded)
{
	if (name == NULL) {
		return;
	}
	++state->total;
	if (loaded) {
		++state->loaded;
	} else {
		++state->missing;
	}
}

static int glcp_test_finish_function_report(
	const struct glcp_test_function_report_state* state)
{
	printf("%s: %lu loaded, %lu missing, %lu total\n",
		state->label,
		(unsigned long)state->loaded,
		(unsigned long)state->missing,
		(unsigned long)state->total);

	return 1;
}

static void glcp_test_begin_function_finalize(
	struct glcp_test_function_finalize_state* state,
	const char* label)
{
	state->label = label;
	state->total = 0;
	state->still_loaded = 0;
}

static void glcp_test_note_function_finalize(
	struct glcp_test_function_finalize_state* state,
	const char* name,
	int loaded)
{
	if (name == NULL) {
		return;
	}
	++state->total;
	if (loaded) {
		fprintf(stderr, "%s: %s is still loaded after finalize\n", state->label, name);
		++state->still_loaded;
	}
}

static int glcp_test_finish_function_finalize(
	const struct glcp_test_function_finalize_state* state)
{
	if (state->total == 0) {
		printf("%s: no generated loader function inventory for this build configuration\n", state->label);
		return 1;
	}

	if (state->still_loaded != 0) {
		fprintf(stderr, "%s: %lu function pointers remained loaded after finalize\n",
			state->label,
			(unsigned long)state->still_loaded);
		return 0;
	}

	printf("%s: %lu function pointers finalized\n", state->label, (unsigned long)state->total);
	return 1;
}

static void glcp_test_begin_versioned_report(
	struct glcp_test_versioned_report_state* state,
	const char* label)
{
	state->label = label;
	state->current_version = NULL;
	state->version_total = 0;
	state->version_loaded = 0;
	state->version_missing = 0;
	state->total = 0;
	state->loaded = 0;
	state->missing = 0;
}

static void glcp_test_note_versioned_report(
	struct glcp_test_versioned_report_state* state,
	const char* version,
	const char* name,
	int loaded)
{
	if (name == NULL || version == NULL) {
		return;
	}

	if (state->current_version != NULL && strcmp(state->current_version, version) != 0) {
		printf("%s: %s -> %lu loaded, %lu missing, %lu total\n",
			state->label,
			state->current_version,
			(unsigned long)state->version_loaded,
			(unsigned long)state->version_missing,
			(unsigned long)state->version_total);
		state->version_total = 0;
		state->version_loaded = 0;
		state->version_missing = 0;
	}

	state->current_version = version;
	++state->version_total;
	++state->total;
	if (loaded) {
		++state->version_loaded;
		++state->loaded;
	} else {
		++state->version_missing;
		++state->missing;
	}
}

static int glcp_test_finish_versioned_report(
	const struct glcp_test_versioned_report_state* state)
{
	if (state->current_version != NULL) {
		printf("%s: %s -> %lu loaded, %lu missing, %lu total\n",
			state->label,
			state->current_version,
			(unsigned long)state->version_loaded,
			(unsigned long)state->version_missing,
			(unsigned long)state->version_total);
	}

	printf("%s: %lu loaded, %lu missing, %lu total\n",
		state->label,
		(unsigned long)state->loaded,
		(unsigned long)state->missing,
		(unsigned long)state->total);
	return 1;
}

static void glcp_test_begin_versioned_expect(
	struct glcp_test_versioned_expect_state* state,
	const char* label,
	int required_major,
	int required_minor)
{
	state->label = label;
	state->current_version = NULL;
	state->version_missing = 0;
	state->total = 0;
	state->loaded = 0;
	state->missing = 0;
	state->failed = 0;
	state->required_major = required_major;
	state->required_minor = required_minor;
}

static void glcp_test_note_versioned_expect(
	struct glcp_test_versioned_expect_state* state,
	const char* version,
	const char* name,
	int loaded)
{
	int version_major = 0;
	int version_minor = 0;
	int required = 0;

	if (name == NULL || version == NULL) {
		return;
	}

	if (sscanf(version, "GLCP_GL_VERSION_%d_%d", &version_major, &version_minor) == 2) {
		required = (version_major < state->required_major) ||
			(version_major == state->required_major && version_minor <= state->required_minor);
	}

	if (state->current_version != NULL && strcmp(state->current_version, version) != 0) {
		if (state->version_missing != 0) {
			fprintf(stderr, "\n");
			fprintf(stderr, "%s: %s missing %lu functions\n",
				state->label,
				state->current_version,
				(unsigned long)state->version_missing);
			state->failed = 1;
		}
		state->version_missing = 0;
	}

	state->current_version = version;
	if (!required) {
		return;
	}

	++state->total;
	if (loaded) {
		++state->loaded;
	} else {
		if (state->version_missing == 0) {
			fprintf(stderr, "%s: %s missing:", state->label, state->current_version);
		}
		fprintf(stderr, " %s", name);
		++state->version_missing;
		++state->missing;
	}
}

static int glcp_test_finish_versioned_expect(
	struct glcp_test_versioned_expect_state* state)
{
	if (state->current_version == NULL) {
		printf("%s: no generated loader function inventory for this build configuration\n", state->label);
		return 1;
	}

	if (state->version_missing != 0) {
		fprintf(stderr, "\n");
		fprintf(stderr, "%s: %s missing %lu functions\n",
			state->label,
			state->current_version,
			(unsigned long)state->version_missing);
		state->failed = 1;
	}

	printf("%s: %lu loaded, %lu missing, %lu total\n",
		state->label,
		(unsigned long)state->loaded,
		(unsigned long)state->missing,
		(unsigned long)state->total);
	return state->failed ? 0 : 1;
}

static int glcp_test_report_function_status(
	const char* label,
	const struct glcp_test_function_status* functions,
	size_t count)
{
	size_t i;
	size_t total = 0;
	size_t loaded = 0;
	size_t missing = 0;

	for (i = 0; i < count; ++i) {
		if (functions[i].name == NULL) {
			continue;
		}
		++total;
		if (functions[i].loaded) {
			++loaded;
		} else {
			++missing;
		}
	}

	printf("%s: %lu loaded, %lu missing, %lu total\n",
		label,
		(unsigned long)loaded,
		(unsigned long)missing,
		(unsigned long)total);

	return 1;
}

static int glcp_test_expect_functions_finalized(
	const char* label,
	const struct glcp_test_function_status* functions,
	size_t count)
{
	size_t i;
	size_t total = 0;
	size_t still_loaded = 0;

	for (i = 0; i < count; ++i) {
		if (functions[i].name == NULL) {
			continue;
		}
		++total;
		if (functions[i].loaded) {
			fprintf(stderr, "%s: %s is still loaded after finalize\n", label, functions[i].name);
			++still_loaded;
		}
	}

	if (total == 0) {
		printf("%s: no generated loader function inventory for this build configuration\n", label);
		return 1;
	}

	if (still_loaded != 0) {
		fprintf(stderr, "%s: %lu function pointers remained loaded after finalize\n",
			label,
			(unsigned long)still_loaded);
		return 0;
	}

	printf("%s: %lu function pointers finalized\n", label, (unsigned long)total);
	return 1;
}

static int glcp_test_report_versioned_function_status(
	const char* label,
	const struct glcp_test_versioned_function_status* functions,
	size_t count)
{
	size_t i;
	const char* current_version = NULL;
	size_t version_total = 0;
	size_t version_loaded = 0;
	size_t version_missing = 0;
	size_t total = 0;
	size_t loaded = 0;
	size_t missing = 0;

	for (i = 0; i < count; ++i) {
		if (functions[i].name == NULL || functions[i].version == NULL) {
			continue;
		}

		if (current_version != NULL && strcmp(current_version, functions[i].version) != 0) {
			printf("%s: %s -> %lu loaded, %lu missing, %lu total\n",
				label,
				current_version,
				(unsigned long)version_loaded,
				(unsigned long)version_missing,
				(unsigned long)version_total);
			version_total = 0;
			version_loaded = 0;
			version_missing = 0;
		}

		current_version = functions[i].version;
		++version_total;
		++total;
		if (functions[i].loaded) {
			++version_loaded;
			++loaded;
		} else {
			++version_missing;
			++missing;
		}
	}

	if (current_version != NULL) {
		printf("%s: %s -> %lu loaded, %lu missing, %lu total\n",
			label,
			current_version,
			(unsigned long)version_loaded,
			(unsigned long)version_missing,
			(unsigned long)version_total);
	}

	printf("%s: %lu loaded, %lu missing, %lu total\n",
		label,
		(unsigned long)loaded,
		(unsigned long)missing,
		(unsigned long)total);
	return 1;
}

static int glcp_test_expect_versioned_functions_loaded(
	const char* label,
	const struct glcp_test_versioned_function_status* functions,
	size_t count,
	int required_major,
	int required_minor)
{
	size_t i;
	const char* current_version = NULL;
	size_t version_missing = 0;
	size_t total = 0;
	size_t loaded = 0;
	size_t missing = 0;
	int failed = 0;

	for (i = 0; i < count; ++i) {
		int version_major = 0;
		int version_minor = 0;
		int required = 0;

		if (functions[i].name == NULL || functions[i].version == NULL) {
			continue;
		}

		if (sscanf(functions[i].version, "GLCP_GL_VERSION_%d_%d", &version_major, &version_minor) == 2) {
			required = (version_major < required_major) ||
				(version_major == required_major && version_minor <= required_minor);
		}

		if (current_version != NULL && strcmp(current_version, functions[i].version) != 0) {
			if (version_missing != 0) {
				fprintf(stderr, "\n");
			}
			if (version_missing != 0) {
				fprintf(stderr, "%s: %s missing %lu functions\n",
					label,
					current_version,
					(unsigned long)version_missing);
				failed = 1;
			}
			version_missing = 0;
		}

		current_version = functions[i].version;
		if (!required) {
			continue;
		}
		++total;
		if (functions[i].loaded) {
			++loaded;
		} else {
			if (version_missing == 0) {
				fprintf(stderr, "%s: %s missing:", label, current_version);
			}
			fprintf(stderr, " %s", functions[i].name);
			++version_missing;
			++missing;
		}
	}

	if (current_version == NULL) {
		printf("%s: no generated loader function inventory for this build configuration\n", label);
		return 1;
	}

	if (version_missing != 0) {
		fprintf(stderr, "\n");
		fprintf(stderr, "%s: %s missing %lu functions\n",
			label,
			current_version,
			(unsigned long)version_missing);
		failed = 1;
	}

	printf("%s: %lu loaded, %lu missing, %lu total\n",
		label,
		(unsigned long)loaded,
		(unsigned long)missing,
		(unsigned long)total);
	return failed ? 0 : 1;
}

static int glcp_test_parse_runtime_gl_version(int* major_out, int* minor_out)
{
	int major = 0;
	int minor = 0;
	GLenum error = GL_NO_ERROR;

	if (major_out == NULL || minor_out == NULL) {
		return 0;
	}

	if (glGetIntegerv != NULL) {
		if (glGetError != NULL) {
			while (glGetError() != GL_NO_ERROR) {
			}
		}
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);
		if (glGetError != NULL) {
			error = glGetError();
		}
		if (error == GL_NO_ERROR && major > 0) {
			*major_out = major;
			*minor_out = minor;
			return 1;
		}
	}

	major = 1;
	minor = 0;
#if defined(GLCP_GL_VERSION_1_1)
	minor = 1;
#endif
#if defined(GLCP_GL_VERSION_1_5) || defined(___GL_COMPAT_PROFILE_H___)
	if (glGenBuffers != NULL || glBindBuffer != NULL || glBufferData != NULL) {
		major = 1;
		minor = 5;
	}
#endif
#if defined(GLCP_GL_VERSION_2_0) || defined(___GL_COMPAT_PROFILE_H___)
	if (glCreateShader != NULL || glCreateProgram != NULL) {
		major = 2;
		minor = 0;
	}
#endif
#if defined(GLCP_GL_VERSION_3_0) && !GLCP_TEST_COMPAT_MACOS_LEGACY
	if (glGenVertexArrays != NULL || glBindVertexArray != NULL) {
		major = 3;
		minor = 0;
	}
#endif
#if defined(GLCP_GL_VERSION_3_2) && !GLCP_TEST_COMPAT_MACOS_LEGACY
	if (glFenceSync != NULL || glClientWaitSync != NULL) {
		major = 3;
		minor = 2;
	}
#endif
#if defined(GLCP_GL_VERSION_3_3) && !GLCP_TEST_COMPAT_MACOS_LEGACY
	if (glGenSamplers != NULL || glSamplerParameteri != NULL) {
		major = 3;
		minor = 3;
	}
#endif
#if defined(GLCP_GL_VERSION_4_5) && !GLCP_TEST_COMPAT_MACOS_LEGACY
	if (glCreateBuffers != NULL || glCreateTextures != NULL) {
		major = 4;
		minor = 5;
	}
#endif

	*major_out = major;
	*minor_out = minor;
	return 1;
}

static void glcp_test_clear_gl_errors(void)
{
	int guard = 0;
	if (glGetError == NULL) {
		return;
	}
	while (guard < 32 && glGetError() != GL_NO_ERROR) {
		++guard;
	}
}

static int glcp_test_expect_gl_no_error(const char* label)
{
	GLenum error;

	if (glGetError == NULL) {
		return 1;
	}

	error = glGetError();
	if (error != GL_NO_ERROR) {
		fprintf(stderr, "%s: unexpected GL error 0x%04x\n", label, (unsigned int)error);
		return 0;
	}
	return 1;
}

static int glcp_test_run_basic_state_calls(void)
{
	GLint viewport[4] = { 0, 0, 0, 0 };
	GLfloat clear_color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	GLboolean depth_mask = GL_FALSE;

	glcp_test_clear_gl_errors();

	if (glViewport != NULL) {
		glViewport(0, 0, 64, 64);
	}
	if (glScissor != NULL) {
		glScissor(0, 0, 32, 32);
	}
	if (glEnable != NULL) {
		glEnable(GL_SCISSOR_TEST);
	}
	if (glDisable != NULL) {
		glDisable(GL_SCISSOR_TEST);
	}
	if (glBlendFunc != NULL) {
		glBlendFunc(GL_ONE, GL_ZERO);
	}
	if (glDepthFunc != NULL) {
		glDepthFunc(GL_LESS);
	}
	if (glStencilFunc != NULL) {
		glStencilFunc(GL_ALWAYS, 0, 0xffu);
	}
	if (glPixelStorei != NULL) {
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	}
	if (glClearColor != NULL) {
		glClearColor(0.0f, 0.25f, 0.5f, 1.0f);
	}
	if (glClear != NULL) {
		glClear(GL_COLOR_BUFFER_BIT);
	}
	if (glGetIntegerv != NULL) {
		glGetIntegerv(GL_VIEWPORT, viewport);
	}
	if (glGetFloatv != NULL) {
		glGetFloatv(GL_COLOR_CLEAR_VALUE, clear_color);
	}
	if (glGetBooleanv != NULL) {
		glGetBooleanv(GL_DEPTH_WRITEMASK, &depth_mask);
	}

	(void)viewport;
	(void)clear_color;
	(void)depth_mask;
	return glcp_test_expect_gl_no_error("basic state coverage");
}

static int glcp_test_run_texture_calls(void)
{
	GLuint texture = 0;
	GLint min_filter = 0;

	if (glGenTextures == NULL || glBindTexture == NULL || glTexParameteri == NULL ||
		glTexImage2D == NULL || glGetTexParameteriv == NULL || glDeleteTextures == NULL) {
		return 1;
	}

	glcp_test_clear_gl_errors();
	glGenTextures(1, &texture);
	TEST_ASSERT(texture != 0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &min_filter);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &texture);

	(void)min_filter;
	return glcp_test_expect_gl_no_error("texture coverage");
}

static int glcp_test_run_buffer_calls(void)
{
	GLuint buffer = 0;
	GLint size = 0;
	const unsigned char input[4] = { 1u, 2u, 3u, 4u };
	unsigned char output[4] = { 0u, 0u, 0u, 0u };

	if (glGenBuffers == NULL || glBindBuffer == NULL || glBufferData == NULL ||
		glBufferSubData == NULL || glGetBufferParameteriv == NULL ||
		glGetBufferSubData == NULL || glDeleteBuffers == NULL) {
		return 1;
	}

	glcp_test_clear_gl_errors();
	glGenBuffers(1, &buffer);
	TEST_ASSERT(buffer != 0);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)sizeof(input), input, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)sizeof(input), input);
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)sizeof(output), output);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &buffer);

	TEST_ASSERT(size >= (GLint)sizeof(input));
	return glcp_test_expect_gl_no_error("buffer coverage");
}

static int glcp_test_run_shader_program_calls(void)
{
	GLuint shader = 0;
	GLuint program = 0;
	GLint status = 0;
	const GLchar* source =
		"#version 110\n"
		"void main(void) { gl_Position = vec4(0.0); }\n";

	if (glCreateShader == NULL || glShaderSource == NULL || glCompileShader == NULL ||
		glGetShaderiv == NULL || glCreateProgram == NULL || glAttachShader == NULL ||
		glLinkProgram == NULL || glGetProgramiv == NULL || glDeleteShader == NULL ||
		glDeleteProgram == NULL) {
		return 1;
	}

	glcp_test_clear_gl_errors();
	shader = glCreateShader(GL_VERTEX_SHADER);
	TEST_ASSERT(shader != 0);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	program = glCreateProgram();
	TEST_ASSERT(program != 0);
	glAttachShader(program, shader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);

	glDeleteProgram(program);
	glDeleteShader(shader);

	(void)status;
	return glcp_test_expect_gl_no_error("shader/program coverage");
}

#if defined(GLCP_GL_VERSION_3_0) && !GLCP_TEST_COMPAT_MACOS_LEGACY
static int glcp_test_run_vertex_array_calls(void)
{
	GLuint vao = 0;

	if (glGenVertexArrays == NULL || glBindVertexArray == NULL || glDeleteVertexArrays == NULL) {
		return 1;
	}

	glcp_test_clear_gl_errors();
	glGenVertexArrays(1, &vao);
	TEST_ASSERT(vao != 0);
	glBindVertexArray(vao);
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vao);
	return glcp_test_expect_gl_no_error("vertex array coverage");
}

static int glcp_test_run_framebuffer_calls(void)
{
	GLuint framebuffer = 0;
	GLuint renderbuffer = 0;
	GLenum status;

	if (glGenFramebuffers == NULL || glBindFramebuffer == NULL ||
		glCheckFramebufferStatus == NULL || glDeleteFramebuffers == NULL ||
		glGenRenderbuffers == NULL || glBindRenderbuffer == NULL ||
		glRenderbufferStorage == NULL || glFramebufferRenderbuffer == NULL ||
		glDeleteRenderbuffers == NULL) {
		return 1;
	}

	glcp_test_clear_gl_errors();
	glGenFramebuffers(1, &framebuffer);
	glGenRenderbuffers(1, &renderbuffer);
	TEST_ASSERT(framebuffer != 0);
	TEST_ASSERT(renderbuffer != 0);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 1, 1);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glDeleteFramebuffers(1, &framebuffer);
	glDeleteRenderbuffers(1, &renderbuffer);

	(void)status;
	return glcp_test_expect_gl_no_error("framebuffer coverage");
}
#endif

#if defined(GLCP_GL_VERSION_1_5)
static int glcp_test_run_query_calls(void)
{
	GLuint query = 0;
	GLint bits = 0;

	if (glGenQueries == NULL || glGetQueryiv == NULL || glIsQuery == NULL ||
		glDeleteQueries == NULL) {
		return 1;
	}

	glcp_test_clear_gl_errors();
	glGenQueries(1, &query);
	TEST_ASSERT(query != 0);
	(void)glIsQuery(query);
	glGetQueryiv(GL_SAMPLES_PASSED, GL_QUERY_COUNTER_BITS, &bits);
	glDeleteQueries(1, &query);

	(void)bits;
	return glcp_test_expect_gl_no_error("query coverage");
}
#endif

#if defined(GLCP_GL_VERSION_3_2) && !GLCP_TEST_COMPAT_MACOS_LEGACY
static int glcp_test_run_sync_calls(void)
{
	GLsync sync;

	if (glFenceSync == NULL || glClientWaitSync == NULL || glDeleteSync == NULL) {
		return 1;
	}

	glcp_test_clear_gl_errors();
	sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	TEST_ASSERT(sync != NULL);
	(void)glClientWaitSync(sync, 0, 0);
	glDeleteSync(sync);
	return glcp_test_expect_gl_no_error("sync coverage");
}
#endif

#if defined(GLCP_GL_VERSION_3_3) && !GLCP_TEST_COMPAT_MACOS_LEGACY
static int glcp_test_run_sampler_calls(void)
{
	GLuint sampler = 0;

	if (glGenSamplers == NULL || glSamplerParameteri == NULL || glDeleteSamplers == NULL) {
		return 1;
	}

	glcp_test_clear_gl_errors();
	glGenSamplers(1, &sampler);
	TEST_ASSERT(sampler != 0);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glDeleteSamplers(1, &sampler);
	return glcp_test_expect_gl_no_error("sampler coverage");
}
#endif

#if defined(GLCP_GL_VERSION_4_5) && !GLCP_TEST_COMPAT_MACOS_LEGACY
static int glcp_test_run_dsa_calls(void)
{
	GLuint buffer = 0;
	GLuint texture = 0;
	GLint size = 0;
	const unsigned char data[4] = { 1u, 2u, 3u, 4u };

	if (glCreateBuffers == NULL || glNamedBufferData == NULL ||
		glGetNamedBufferParameteriv == NULL || glDeleteBuffers == NULL ||
		glCreateTextures == NULL || glTextureParameteri == NULL ||
		glTextureStorage2D == NULL || glDeleteTextures == NULL) {
		return 1;
	}

	glcp_test_clear_gl_errors();
	glCreateBuffers(1, &buffer);
	TEST_ASSERT(buffer != 0);
	glNamedBufferData(buffer, (GLsizeiptr)sizeof(data), data, GL_STATIC_DRAW);
	glGetNamedBufferParameteriv(buffer, GL_BUFFER_SIZE, &size);

	glCreateTextures(GL_TEXTURE_2D, 1, &texture);
	TEST_ASSERT(texture != 0);
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureStorage2D(texture, 1, GL_RGBA8, 1, 1);

	glDeleteTextures(1, &texture);
	glDeleteBuffers(1, &buffer);

	TEST_ASSERT(size >= (GLint)sizeof(data));
	return glcp_test_expect_gl_no_error("dsa coverage");
}
#endif

static int glcp_test_run_common_runtime_calls(void)
{
	TEST_ASSERT(glcp_test_run_basic_state_calls());
	TEST_ASSERT(glcp_test_run_texture_calls());
#if defined(GLCP_GL_VERSION_1_5) || defined(___GL_COMPAT_PROFILE_H___)
	TEST_ASSERT(glcp_test_run_buffer_calls());
#endif
#if defined(GLCP_GL_VERSION_2_0) || defined(___GL_COMPAT_PROFILE_H___)
	TEST_ASSERT(glcp_test_run_shader_program_calls());
#endif
#if defined(GLCP_GL_VERSION_3_0) && !GLCP_TEST_COMPAT_MACOS_LEGACY
	TEST_ASSERT(glcp_test_run_vertex_array_calls());
	TEST_ASSERT(glcp_test_run_framebuffer_calls());
#endif
#if defined(GLCP_GL_VERSION_1_5)
	TEST_ASSERT(glcp_test_run_query_calls());
#endif
#if defined(GLCP_GL_VERSION_3_2) && !GLCP_TEST_COMPAT_MACOS_LEGACY
	TEST_ASSERT(glcp_test_run_sync_calls());
#endif
#if defined(GLCP_GL_VERSION_3_3) && !GLCP_TEST_COMPAT_MACOS_LEGACY
	TEST_ASSERT(glcp_test_run_sampler_calls());
#endif
#if defined(GLCP_GL_VERSION_4_5) && !GLCP_TEST_COMPAT_MACOS_LEGACY
	TEST_ASSERT(glcp_test_run_dsa_calls());
#endif
	return 1;
}

#if defined(___GL_COMPAT_PROFILE_H___)
static int glcp_test_run_compat_fixed_function_calls(void)
{
	glcp_test_clear_gl_errors();

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glBegin(GL_POINTS);
	glVertex2f(0.0f, 0.0f);
	glEnd();
	glPopAttrib();

	return glcp_test_expect_gl_no_error("compat fixed-function coverage");
}
#endif

#endif
