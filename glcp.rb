# http://www.opengl.org/registry/#headers
#
# PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB;
# glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress("glMultiTexCoord2fARB");
# if(!glMultiTexCoord2fARB)return 1;

GLCP_VERSION = '0.0.2'
GLCP_GENERATE_TIME = Time.now

HEADER = <<"EOS"
/*
 * glcp
 * version #{GLCP_VERSION}
 *
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

#
functions.each{|function,version|
	name = 'PFN' + function.upcase + 'PROC';
	raise 'function type not found ' + function if prototypes[name] == nil
	prototypes[name] = function
}

File.open('glcp/glcp.c', 'w'){|file|
	file.puts HEADER
	file.puts '#include "glcp.h"'

	################################################################################
	# 関数ポインター変数を出力
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
	# 関数を出力
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
	# 初期化関数を出力
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
	# 終了関数を出力
	################################################################################
	file.puts 'void glcpFinalize()'
	file.puts '{'
	file.puts '}'
}

open("gl/glcorearb.h") {|source|
	open("glcp/glcp.h", "w") {|dest|
		dest.puts HEADER
		dest.puts '#if !defined(___GL_CORE_PROFILE_H___)'
		dest.puts '#define ___GL_CORE_PROFILE_H___'
		dest.puts '#include <windows.h>'
		dest.puts '#include <gl/gl.h>'
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
		# バージョンシンボルを出力
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
		# glcorearb.hを出力
		################################################################################
		dest.puts '/* <-- glcorearb.h */'
		dest.write(source.read)
		dest.puts '/* --> glcorearb.h */'
		dest.puts '#if defined(__cplusplus)'
		dest.puts 'extern "C" {'
		dest.puts '#endif'
#=begin
		################################################################################
		# 関数ポインター変数を出力
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
		# プロトタイプ宣言を出力
		################################################################################
		dest.puts 'extern void glcpInitialize();'
		dest.puts 'extern void glcpFinalize();'
		dest.puts '#if defined(__cplusplus)'
		dest.puts '}'
		dest.puts '#endif'
		dest.puts '#endif /*___GL_CORE_PROFILE_H___*/'
	}
}

puts 'done.'
