# http://www.opengl.org/registry/#headers
#
# PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB;
# glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress("glMultiTexCoord2fARB");
# if(!glMultiTexCoord2fARB)return 1;

require 'mkmf'

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

def detect_build_check_command
	commands = [
		['x86_64-w64-mingw32-gcc', 'x86_64-w64-mingw32-gcc -c -I. -o /dev/null glcp/glcp.c'],
		['i686-w64-mingw32-gcc', 'i686-w64-mingw32-gcc -c -I. -o /dev/null glcp/glcp.c'],
		['clang', 'clang --target=x86_64-w64-windows-gnu -c -I. -o /dev/null glcp/glcp.c'],
		['gcc', 'gcc -c -I. -o /dev/null glcp/glcp.c'],
		['cc', 'cc -c -I. -o /dev/null glcp/glcp.c']
	]

	commands.each do |executable, command|
		return [executable, command] if find_executable(executable)
	end

	return ['cl', 'cl /nologo /c /I. /TC /FoNUL glcp\\glcp.c'] if find_executable('cl')

	nil
end

def run_build_check
	log_stage('building generated glcp/glcp.c')
	detected = detect_build_check_command
	if detected.nil?
		message = 'no supported C compiler found for build check'
		if ENV['GLCP_BUILD_CHECK_REQUIRED'] == '1'
			raise message
		end
		puts "[glcp] skipped build check: #{message}"
		return
	end

	compiler, command = detected
	puts "[glcp] build check compiler: #{compiler}"
	success = run_command(command)
	raise "build check failed with #{compiler}" unless success
	puts '[glcp] build check passed'
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

log_stage('detecting supported OpenGL version from gl/glcorearb.h')
SUPPORTED_GL_MAJOR, SUPPORTED_GL_MINOR = detect_supported_gl_version('gl/glcorearb.h')
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
	log_stage('parsing gl/glcorearb.h for functions and prototypes')
	File.open('gl/glcorearb.h'){|file|
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
		file.puts '	' + function + ' = (' + prototype + ')wglGetProcAddress("' + function + '");'
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
		file.puts '}'
	}
	puts '[glcp] wrote glcp/glcp.c'

	log_stage('writing glcp/glcp.h')
	open("gl/glcorearb.h") {|source|
		open("glcp/glcp.h", "w") {|dest|
			dest.puts HEADER
			dest.puts '#if !defined(___GL_CORE_PROFILE_H___)'
			dest.puts '#define ___GL_CORE_PROFILE_H___'
			dest.puts '#include <KHR/khrplatform.h>'
			dest.puts '#include <windows.h>'
			dest.puts '#include <GL/gl.h>'
			dest.puts '#if defined(GL_VERSION_1_1) && !defined(GL_VERSION_1_0)'
			dest.puts '#define GL_VERSION_1_0'
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

		################################################################################
		# Output version symbols
		################################################################################
		last_output_version = ''
		functions.each{|function, version|
			if last_output_version != version then
				dest.puts '#endif' if last_output_version != ''
				dest.puts '#if !defined(' + version + ')'
				dest.puts '#define GLCP_' + version + ' ' + version
				last_output_version = version
			end
		}
		if last_output_version != '' then
			dest.puts '#endif'
		end

		################################################################################
		# Embed glcorearb.h contents
		################################################################################
		dest.puts '/* <-- glcorearb.h */'
		dest.write(source.read)
		dest.puts '/* --> glcorearb.h */'
		dest.puts '#if defined(__cplusplus)'
		dest.puts 'extern "C" {'
		dest.puts '#endif'
#=begin
		################################################################################
		# Output function pointer variables
		################################################################################
		last_output_version = ''
		prototypes.each{|prototype, function|
			version = functions[function]
			if last_output_version != version then
				dest.puts '#endif' if last_output_version != ''
				dest.puts '#if defined(GLCP_' + version + ')'
				last_output_version = version
			end
			dest.puts 'extern ' + prototype + ' ' + function + ';'
		}
		if last_output_version != '' then
			dest.puts '#endif'
		end
#=end
		################################################################################
		# Output prototype declarations
		################################################################################
		dest.puts 'extern void glcpInitialize();'
		dest.puts 'extern void glcpFinalize();'
		dest.puts '#if defined(__cplusplus)'
		dest.puts '}'
		dest.puts '#endif'
			dest.puts '#endif /*___GL_CORE_PROFILE_H___*/'
		}
	}
	puts '[glcp] wrote glcp/glcp.h'

	run_build_check

	log_stage('completed successfully')
	puts 'done.'
rescue => e
	puts "[glcp] failed during #{CURRENT_STAGE[:name]}: #{e.class}: #{e.message}"
	puts e.backtrace.map{|line| "[glcp] #{line}"}
	raise
end
