# glcp.rb
#
# Generator for glcp Core and Compatibility loader outputs.
#
# Input:
#   - external/gl/glcorearb.h from the Khronos OpenGL Registry
#   - external/KHR/khrplatform.h from the Khronos EGL Registry
#
# Output:
#   - glcp/glcp.c
#   - glcp/glcp.h (self-contained Core Profile header)
#   - glcp/glcp_compat.c
#   - glcp/glcp_compat.h (Compatibility Profile oriented header)
#
# Purpose:
#   - Generate a lightweight loader for Desktop OpenGL Core Profile APIs
#   - Generate a Compatibility Profile oriented loader that relies on platform gl.h for OpenGL 1.0/1.1
#   - Target Windows, Linux, and macOS desktop OpenGL environments
#
# Notes:
#   - This script is mainly for maintainers and contributors
#   - Normal users are expected to consume released glcp.c / glcp.h packages
#   - iOS and Android are out of scope because they typically use OpenGL ES

DEFAULT_GLCP_RELEASE = 3
GLCP_GENERATE_TIME = Time.now
CURRENT_STAGE = { name: 'startup' }

def log_stage(message)
	CURRENT_STAGE[:name] = message
	puts "[glcp] #{message}"
	$stdout.flush
end

def run_command(command)
	puts "[glcp] running: #{command}"
	success = system(command)
	status = $?.exitstatus if $?
	puts "[glcp] command exit status: #{status.nil? ? 'unknown' : status}"
	success
end

def transform_glcorearb_text(text)
	version_stack = []

	text.each_line.map{|line|
		if /\A#ifndef (GL_VERSION_\d+_\d+)\s*$/ =~ line
			version = $1
			version_stack << version
			"#ifndef GLCP_DECL_#{version}\n"
		elsif /\A#define (GL_VERSION_\d+_\d+) 1\s*$/ =~ line
			version = $1
			"#define GLCP_DECL_#{version} 1\n"
		elsif /\A#endif \/\* (GL_VERSION_\d+_\d+) \*\/\s*$/ =~ line
			version = $1
			if version_stack.last == version
				version_stack.pop
				"#endif /* GLCP_DECL_#{version} */\n"
			else
				line
			end
		else
			line
		end
	}.join
end

def emit_version_macros(dest, functions, min_loader_version = nil)
	last_output_version = ''
	functions.each do |_function, version|
		next if min_loader_version && version < min_loader_version
		next if last_output_version == version
		dest.puts '#define GLCP_' + version + ' 1'
		last_output_version = version
	end
end

def compat_version_guard(version)
	if version < 'GL_VERSION_1_2'
		"#if 0 /* compat uses platform gl.h for #{version.sub('GL_VERSION_', '').tr('_', '.')} */"
	elsif version < 'GL_VERSION_1_5'
		"#if defined(_WIN32) && defined(GLCP_#{version})"
	else
		"#if !defined(__APPLE__) && defined(GLCP_#{version})"
	end
end

def emit_function_pointer_decls(dest, functions, prototypes, min_loader_version = nil)
	last_output_version = ''
	prototypes.each do |prototype, function|
		version = functions[function]
		next if min_loader_version && version < min_loader_version
		if last_output_version != version
			dest.puts '#endif' if last_output_version != ''
			dest.puts '#if defined(GLCP_' + version + ')'
			last_output_version = version
		end
		dest.puts 'extern ' + prototype + ' ' + function + ';'
	end
	dest.puts '#endif' if last_output_version != ''
end

def emit_compat_function_pointer_decls(dest, functions, prototypes, min_loader_version = nil)
	last_output_version = ''
	prototypes.each do |prototype, function|
		version = functions[function]
		next if min_loader_version && version < min_loader_version
		if last_output_version != version
			dest.puts '#endif' if last_output_version != ''
			dest.puts compat_version_guard(version)
			last_output_version = version
		end
		dest.puts 'extern ' + prototype + ' ' + function + ';'
	end
	dest.puts '#endif' if last_output_version != ''
end

def emit_compat_version_macros(dest, functions)
	last_output_version = ''
	functions.each do |_function, version|
		next if last_output_version == version
		if version < 'GL_VERSION_1_5'
			dest.puts "#if defined(_WIN32)"
			dest.puts "#define GLCP_#{version} 1"
			dest.puts '#endif'
		else
			dest.puts "#if !defined(__APPLE__)"
			dest.puts "#define GLCP_#{version} 1"
			dest.puts '#endif'
		end
		last_output_version = version
	end
end

def transform_compat_source(core_source)
	compat_source = core_source
		.sub('#include "glcp.h"', '#include "glcp_compat.h"')
		.gsub(/\bglcpInitialize\b/, 'glcpCompatInitialize')
		.gsub(/\bglcpFinalize\b/, 'glcpCompatFinalize')

	%w[GL_VERSION_1_0 GL_VERSION_1_1].each do |version|
		compat_source = compat_source.gsub(
			"#if defined(GLCP_#{version})",
			compat_version_guard(version)
		)
	end

	%w[GL_VERSION_1_2 GL_VERSION_1_3 GL_VERSION_1_4].each do |version|
		compat_source = compat_source.gsub(
			"#if defined(GLCP_#{version})",
			compat_version_guard(version)
		)
	end

	compat_source = compat_source.gsub(
		/#if defined\(GLCP_(GL_VERSION_[2-9]_\d+)\)/,
		'#if !defined(__APPLE__) && defined(GLCP_\1)'
	)
	compat_source = compat_source.gsub(
		/#if defined\(GLCP_(GL_VERSION_1_[5-9])\)/,
		'#if !defined(__APPLE__) && defined(GLCP_\1)'
	)

	compat_source
end

def validate_generated_header(path, final_endif)
	lines = File.readlines(path)
	raise "#{path} is empty" if lines.empty?
	raise "#{path} missing final #{final_endif}" unless lines.any?{|line| line.strip == final_endif }

	balance = 0
	lines.each do |line|
		stripped = line.strip
		if stripped.start_with?('#if')
			balance += 1
		elsif stripped.start_with?('#endif')
			balance -= 1
			raise "#{path} has mismatched #if/#endif" if balance < 0
		end
	end
	raise "#{path} has unterminated #if/#endif" unless balance == 0
end

def compatibility_header_preamble
	<<~'EOS'
		#if !defined(___GL_COMPAT_PROFILE_H___)
		#define ___GL_COMPAT_PROFILE_H___
		#if defined(__glext_h_)
		#error glext.h included before glcp_compat.h
		#endif
		#if defined(__wglext_h_)
		#error wglext.h included before glcp_compat.h
		#endif
		#if defined(__glxext_h_)
		#error glxext.h included before glcp_compat.h
		#endif
		#if defined(_WIN32)
		#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN 1
		#endif
		#include <windows.h>
		#include <GL/gl.h>
		#elif defined(__APPLE__)
		#ifndef GL_GLEXT_LEGACY
		#define GL_GLEXT_LEGACY 1
		#endif
		#include <TargetConditionals.h>
		#if TARGET_OS_OSX
		#include <OpenGL/gl.h>
		#endif
		#elif defined(__linux__) && !defined(__ANDROID__)
		#ifndef GL_GLEXT_LEGACY
		#define GL_GLEXT_LEGACY 1
		#endif
		#include <GL/gl.h>
		#endif
		#ifdef GL_GLEXT_PROTOTYPES
		#undef GL_GLEXT_PROTOTYPES
		#endif
		#define GLCP_GL_VERSION_1_2 1
		#define GLCP_GL_VERSION_1_3 1
		#define GLCP_GL_VERSION_1_4 1
		#define GLCP_GL_VERSION_1_5 1
		#define GLCP_GL_VERSION_2_0 1
		#define GLCP_GL_VERSION_2_1 1
		#define GLCP_GL_VERSION_3_0 1
		#define GLCP_GL_VERSION_3_1 1
		#define GLCP_GL_VERSION_3_2 1
		#define GLCP_GL_VERSION_3_3 1
		#define GLCP_GL_VERSION_4_0 1
		#define GLCP_GL_VERSION_4_1 1
		#define GLCP_GL_VERSION_4_2 1
		#define GLCP_GL_VERSION_4_3 1
		#define GLCP_GL_VERSION_4_4 1
		#define GLCP_GL_VERSION_4_5 1
		#define GLCP_GL_VERSION_4_6 1
		EOS
end

def strip_compat_legacy_decl_blocks(lines)
	skip_depth = 0
	lines.each_with_object([]) do |line, kept|
		if line =~ /\A#ifndef GLCP_DECL_GL_VERSION_1_[01]\s*$/
			skip_depth = 1
			next
		end

		if skip_depth > 0
			skip_depth += 1 if line.start_with?('#if')
			if line.start_with?('#endif')
				skip_depth -= 1
			end
			next
		end

		kept << line
	end
end

def extract_header_comment_lines(header)
	lines = header.each_line.to_a
	header_comment_end = lines.index{|line| line.strip == '*/' }
	raise 'header comment end not found' if header_comment_end.nil?
	lines[0..header_comment_end]
end

def write_core_header(path, header_comment, khrplatform_text, glcorearb_text, functions, prototypes)
	File.open(path, 'w') do |dest|
		dest.write(header_comment)
		dest.puts '#if !defined(___GL_CORE_PROFILE_H___)'
		dest.puts '#define ___GL_CORE_PROFILE_H___'
		dest.puts '#if defined(_WIN32)'
		dest.puts '#ifndef WIN32_LEAN_AND_MEAN'
		dest.puts '#define WIN32_LEAN_AND_MEAN 1'
		dest.puts '#endif'
		dest.puts '#include <windows.h>'
		dest.puts '#endif'
		dest.puts '#if defined(__glext_h_)'
		dest.puts '#error glext.h included before glcp.h'
		dest.puts '#endif'
		dest.puts '#if defined(__wglext_h_)'
		dest.puts '#error wglext.h included before glcp.h'
		dest.puts '#endif'
		dest.puts '#if defined(__glxext_h_)'
		dest.puts '#error glxext.h included before glcp.h'
		dest.puts '#endif'
		emit_version_macros(dest, functions)
		dest.puts '/* <-- khrplatform.h */'
		dest.write(khrplatform_text)
		dest.puts
		dest.puts '/* --> khrplatform.h */'
		dest.puts '/* <-- glcorearb.h */'
		dest.write(glcorearb_text)
		dest.puts '/* --> glcorearb.h */'
		dest.puts '#if defined(__cplusplus)'
		dest.puts 'extern "C" {'
		dest.puts '#endif'
		emit_function_pointer_decls(dest, functions, prototypes)
		dest.puts 'extern void glcpInitialize();'
		dest.puts 'extern void glcpFinalize();'
		dest.puts '#if defined(__cplusplus)'
		dest.puts '}'
		dest.puts '#endif'
		dest.puts '#endif /*___GL_CORE_PROFILE_H___*/'
	end
	validate_generated_header(path, '#endif /*___GL_CORE_PROFILE_H___*/')
end

def write_compat_outputs(header_comment_lines, khrplatform_text, glcorearb_text, functions, prototypes)
	log_stage('writing glcp/glcp_compat.h')
	compat_glcorearb_text = strip_compat_legacy_decl_blocks(glcorearb_text.each_line.to_a).join

	File.open('glcp/glcp_compat.h', 'w') do |dest|
		header_comment_lines.each{|line| dest.write(line) }
		dest.puts
		dest.write(compatibility_header_preamble)
		emit_compat_version_macros(dest, functions)
		dest.puts '/* <-- khrplatform.h */'
		dest.write(khrplatform_text)
		dest.puts
		dest.puts '/* --> khrplatform.h */'
		dest.puts '/* <-- glcorearb.h */'
		dest.write(compat_glcorearb_text)
		dest.puts '/* --> glcorearb.h */'
		dest.puts '#if defined(__cplusplus)'
		dest.puts 'extern "C" {'
		dest.puts '#endif'
		emit_compat_function_pointer_decls(dest, functions, prototypes, 'GL_VERSION_1_2')
		dest.puts 'extern void glcpCompatInitialize();'
		dest.puts 'extern void glcpCompatFinalize();'
		dest.puts '#if defined(__cplusplus)'
		dest.puts '}'
		dest.puts '#endif'
		dest.puts '#endif /*___GL_COMPAT_PROFILE_H___*/'
	end
	validate_generated_header('glcp/glcp_compat.h', '#endif /*___GL_COMPAT_PROFILE_H___*/')
	puts '[glcp] wrote glcp/glcp_compat.h'

	log_stage('writing glcp/glcp_compat.c')
	core_source = File.read('glcp/glcp.c')
	compat_source = transform_compat_source(core_source)
	File.write('glcp/glcp_compat.c', compat_source)
	puts '[glcp] wrote glcp/glcp_compat.c'
end

def detect_supported_gl_version(path)
	versions = []
	File.open(path){|file|
		file.each{|line|
			if /\#ifndef\sGL_VERSION_([0-9]+)_([0-9]+)/ =~ line then
				versions << [$1.to_i, $2.to_i]
			end
		}
	}
	return [0, 0] if versions.empty?
	versions.max
end

def detect_glcp_release
	value = ENV['GLCP_RELEASE']
	return DEFAULT_GLCP_RELEASE if value.nil? || value.strip.empty?

	Integer(value, 10)
rescue ArgumentError
	raise "invalid GLCP_RELEASE value: #{value.inspect}"
end

log_stage('detecting supported OpenGL version from external/gl/glcorearb.h')
SUPPORTED_GL_MAJOR, SUPPORTED_GL_MINOR = detect_supported_gl_version('external/gl/glcorearb.h')
SUPPORTED_GL_VERSION = "#{SUPPORTED_GL_MAJOR}.#{SUPPORTED_GL_MINOR}"
GLCP_RELEASE = detect_glcp_release
GLCP_VERSION = "#{SUPPORTED_GL_MAJOR}.#{SUPPORTED_GL_MINOR}.#{GLCP_RELEASE}"
puts "[glcp] detected OpenGL #{SUPPORTED_GL_VERSION}, generator version #{GLCP_VERSION}"

HEADER = <<"EOS"
/*
 * glcp
 * version #{GLCP_VERSION}
 * supported OpenGL version #{SUPPORTED_GL_VERSION}
 *
 * The zlib/libpng License
 * Copyright (C) 2013-#{GLCP_GENERATE_TIME.strftime("%Y")} Shun Moriya
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * generate from glcp.rb at #{GLCP_GENERATE_TIME.strftime("%Y-%m-%d %T")}
 */

EOS

class Function
	def initialize(type, name, argument)
		@type = type
		@name = name
		@argument = argument
	end
	def getType
		return @type
	end
	def getName
		return @name
	end
	def getArgument
		return @argument
	end
end

functions = {}
prototypes = {}
hoge = {}

begin
	log_stage('parsing external/gl/glcorearb.h for functions and prototypes')
	File.open('external/gl/glcorearb.h'){|file|
		version = 'GL_VERSION_0_0'
		file.each{|line|
			if /\#ifndef\s(GL_VERSION_[0-9|_]+)/ =~ line then
				version = $1
			end

			#if /GLAPI\s([0-9|a-z|A-Z|_]+)\sAPIENTRY\s([0-9|a-z|A-Z|_]+)\s(\([0-9|a-z|A-Z|_*,.\s]+\));/ =~ line then
			if /GLAPI\s([0-9|a-z|A-Z|_]+)\sAPIENTRY\s([0-9|a-z|A-Z|_]+)/ =~ line then
				functions[$2] = version
=begin
				position = line.rindex(';')
				puts $1 + ' ' + $2 + ' ' + $3
				if position != nil then
					hoge[$2] = line.slice(0, position)
				end
				hoge[$2] = Function.new($1, $2, $3)
=end
			end

			if /typedef\s[0-9|a-z|A-Z|_]+\s\(APIENTRYP\s([0-9|a-z|A-Z|_]+)/ =~ line then
				prototypes[$1] = 'NOT FOUND'
			end
		}
	}
	puts "[glcp] parsed #{functions.size} functions and #{prototypes.size} prototypes"

	log_stage('matching functions to PFN prototypes')
	functions.each{|function,version|
		name = 'PFN' + function.upcase + 'PROC';
		raise 'function type not found ' + function if prototypes[name] == nil
		prototypes[name] = function
	}
	puts "[glcp] matched #{functions.size} functions to prototypes"

	log_stage('writing glcp/glcp.c')
	File.open('glcp/glcp.c', 'w'){|file|
		file.puts HEADER
		file.puts '#include "glcp.h"'
		file.puts <<~'EOS'
			#include <stddef.h>
			#if defined(__linux__) && !defined(__ANDROID__)
			#include <dlfcn.h>
			typedef void (*glcpGLXProc)(void);
			extern glcpGLXProc glXGetProcAddressARB(const GLubyte* name);
			#elif defined(__APPLE__)
			#include <TargetConditionals.h>
			#include <dlfcn.h>
			#endif

			#if defined(_WIN32)
			static void* glcpGetProcAddress(const char* name)
			{
				PROC proc = wglGetProcAddress(name);
				if (proc != NULL && proc != (PROC)0x1 && proc != (PROC)0x2 && proc != (PROC)0x3 && proc != (PROC)-1) {
					return (void*)proc;
				}

				{
					HMODULE module = GetModuleHandleA("opengl32.dll");
					if (module == NULL) {
						module = LoadLibraryA("opengl32.dll");
					}
					return module == NULL ? NULL : (void*)GetProcAddress(module, name);
				}
			}
			#elif defined(__linux__) && !defined(__ANDROID__)
			static void* glcpGetProcAddress(const char* name)
			{
				void* proc = (void*)glXGetProcAddressARB((const GLubyte*)name);
				if (proc != NULL) {
					return proc;
				}

				{
					static void* module = NULL;
					if (module == NULL) {
						module = dlopen("libGL.so.1", RTLD_LAZY | RTLD_LOCAL);
					}
					return module == NULL ? NULL : dlsym(module, name);
				}
			}
			#elif defined(__APPLE__) && TARGET_OS_OSX
			static void* glcpGetProcAddress(const char* name)
			{
				static void* module = NULL;
				if (module == NULL) {
					module = dlopen("/System/Library/Frameworks/OpenGL.framework/OpenGL", RTLD_LAZY | RTLD_LOCAL);
				}
				return module == NULL ? NULL : dlsym(module, name);
			}
			#else
			static void* glcpGetProcAddress(const char* name)
			{
				(void)name;
				return NULL;
			}
			#endif
		EOS

	################################################################################
	# Output function pointer variables
	################################################################################
	last_output_version = ''
	prototypes.each{|prototype, function|
		version = functions[function]
		if last_output_version != version then
			file.puts '#endif' if last_output_version != ''
			file.puts '#if defined(GLCP_' + version + ')'
			last_output_version = version
		end
		file.puts prototype + ' ' + function + ' = NULL;'
	}
	if last_output_version != '' then
		file.puts '#endif'
	end
=begin
	################################################################################
	# Output function wrappers
	################################################################################
	last_output_version = ''
	prototypes.each{|prototype, function|
		version = functions[function]
		if last_output_version != version then
			file.puts '#endif' if last_output_version != ''
			file.puts '#if defined(GLCP_' + version + ')'
			last_output_version = version
		end

		header = hoge[function]
		if header then
			file.puts header.getType() + ' ' + header.getName() + header.getArgument()
			file.puts '{'
			if header.getType() != 'void' then
				file.puts '	return _' + header.getName() + header.getArgument() + ';'
			else
				file.puts '	_' + header.getName() + header.getArgument() + ';'
			end
			file.puts '}'
		end
	}
	if last_output_version != '' then
		file.puts '#endif'
	end
=end
	################################################################################
	# Output initialization function
	################################################################################
	file.puts 'void glcpInitialize()'
	file.puts '{'

	last_output_version = ''
	prototypes.each{|prototype, function|
		version = functions[function]
		if last_output_version != version then
			file.puts '#endif' if last_output_version != ''
			file.puts '#if defined(GLCP_' + version + ')'
			last_output_version = version
		end
		file.puts '	' + function + ' = (' + prototype + ')glcpGetProcAddress("' + function + '");'
	}
	if last_output_version != '' then
		file.puts '#endif'
	end

	file.puts '}'

	################################################################################
	# Output finalization function
	################################################################################
	file.puts 'void glcpFinalize()'
	file.puts '{'

	last_output_version = ''
	prototypes.each{|prototype, function|
		version = functions[function]
		if last_output_version != version then
			file.puts '#endif' if last_output_version != ''
			file.puts '#if defined(GLCP_' + version + ')'
			last_output_version = version
		end
		file.puts '	' + function + ' = NULL;'
	}
	if last_output_version != '' then
		file.puts '#endif'
	end

	file.puts '}'
	}
	puts '[glcp] wrote glcp/glcp.c'

	log_stage('writing glcp/glcp.h')
	khrplatform_text = File.read('external/KHR/khrplatform.h')
	glcorearb_text = transform_glcorearb_text(File.read('external/gl/glcorearb.h').sub(
		'#include <KHR/khrplatform.h>',
		'/* glcp: inlined KHR/khrplatform.h */'
	))
	header_comment_lines = extract_header_comment_lines(HEADER)
	write_core_header('glcp/glcp.h', HEADER, khrplatform_text, glcorearb_text, functions, prototypes)
	puts '[glcp] wrote glcp/glcp.h'

	write_compat_outputs(header_comment_lines, khrplatform_text, glcorearb_text, functions, prototypes)

	log_stage('completed successfully')
	puts 'done.'
rescue => e
	puts "[glcp] failed during #{CURRENT_STAGE[:name]}: #{e.class}: #{e.message}"
	puts e.backtrace.map{|line| "[glcp] #{line}"}
	raise
end
