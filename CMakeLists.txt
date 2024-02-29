# Copyright 2024 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# CMake Setup
cmake_minimum_required(VERSION 3.26)
project(
	Hellextractor
	DESCRIPTION "Extract content from Helldivers 2 (Autodesk Stingray)"
	HOMEPAGE_URL "https://github.com/Xaymar/Hellextractor"
)
list(APPEND CMAKE_MESSAGE_INDENT "[${PROJECT_NAME}] ")

################################################################################
# Modules
################################################################################

set(CMAKE_MODULE_PATH
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/clang"
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/version"
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake"
)

include("CheckIPOSupported")
if(EXISTS "cmake/version/version.cmake")
	include("cmake/version/version.cmake")
endif()
if(EXISTS "cmake/clang/Clang.cmake")
	include("cmake/clang/Clang.cmake")
endif()

################################################################################
# Versioning
################################################################################

# if(EXISTS "cmake/version/version.cmake")
# 	version(GENERATE _VERSION COMPRESSED MAJOR 0 MINOR 0 PATCH 0 TWEAK 0 REQUIRE "PATCH;")
# 	version(PARSE _VERSION "${_VERSION}" REQUIRE "PATCH;TWEAK")

# 	# If possible, automatically generate versions from git.
# 	if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/.git")
# 		find_program(GIT
# 			NAMES
# 				git
# 				git.exe
# 		)

# 		if(EXISTS "${GIT}")
# 			# Try and calculate the exist version using git.
# 			execute_process(COMMAND "${GIT}" describe --tags --long --abbrev=8 HEAD WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} RESULT_VARIABLE GIT_RESULT OUTPUT_VARIABLE GIT_OUTPUT ERROR_VARIABLE GIT_ERROR OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
# 			if((GIT_RESULT EQUAL 0) AND (NOT "${GIT_OUTPUT}" STREQUAL ""))
# 				# Result will be MAJOR.MINOR.PATCH-TWEAK-gHASH
# 				string(REPLACE "-" ";" GIT_OUTPUT "${GIT_OUTPUT}")
# 				string(REPLACE "." ";" GIT_OUTPUT "${GIT_OUTPUT}")

# 				# Split into components
# 				list(GET GIT_OUTPUT 0 GIT_OUTPUT_MAJOR)
# 				list(GET GIT_OUTPUT 1 GIT_OUTPUT_MINOR)
# 				list(GET GIT_OUTPUT 2 GIT_OUTPUT_PATCH)
# 				list(GET GIT_OUTPUT 3 GIT_OUTPUT_TWEAK)
# 				list(GET GIT_OUTPUT 4 GIT_OUTPUT_BUILD)

# 				# Special case: Tag contains prerelease
# 				if(GIT_OUTPUT_PATCH MATCHES "([0-9]+)([a-zA-Z]+)([0-9]*)")
# 					# Patch requires special parsing.
# 					set(GIT_OUTPUT_PATCH "${CMAKE_MATCH_1}")
# 					if(CMAKE_MATCH_3 GREATER 0)
# 						set(GIT_OUTPUT_PRERELEASE "${CMAKE_MATCH_2}")
# 					else()
# 						set(GIT_OUTPUT_PRERELEASE "a")
# 					endif()
# 					MATH(EXPR GIT_OUTPUT_TWEAK "${GIT_OUTPUT_TWEAK} + ${CMAKE_MATCH_3}")

# 					# Modify the global version
# 					version(MODIFY _VERSION "${_VERSION}" COMPRESS
# 						MAJOR "${GIT_OUTPUT_MAJOR}"
# 						MINOR "${GIT_OUTPUT_MINOR}"
# 						PATCH "${GIT_OUTPUT_PATCH}"
# 						TWEAK "${GIT_OUTPUT_TWEAK}"
# 						BUILD "${GIT_OUTPUT_BUILD}"
# 						PRERELEASE "${GIT_OUTPUT_PRERELEASE}"
# 						REQUIRE "PATCH;TWEAK"
# 					)
# 				else()
# 					# Modify the global version
# 					version(MODIFY _VERSION "${_VERSION}" COMPRESS
# 						MAJOR "${GIT_OUTPUT_MAJOR}"
# 						MINOR "${GIT_OUTPUT_MINOR}"
# 						PATCH "${GIT_OUTPUT_PATCH}"
# 						TWEAK "${GIT_OUTPUT_TWEAK}"
# 						BUILD "${GIT_OUTPUT_BUILD}"
# 						PRERELEASE "a"
# 						REQUIRE "PATCH;TWEAK"
# 					)
# 				endif()
# 			else()
# 				execute_process(COMMAND "${GIT}" rev-list --count HEAD WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} RESULT_VARIABLE GIT_RESULT OUTPUT_VARIABLE GIT_OUTPUT ERROR_VARIABLE GIT_ERROR OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
# 				if((GIT_RESULT EQUAL 0) AND (NOT "${GIT_OUTPUT}" STREQUAL ""))
# 					version(MODIFY _VERSION "${_VERSION}" COMPRESS
# 						TWEAK "${GIT_OUTPUT}"
# 						PRERELEASE "a"
# 						REQUIRE "PATCH;TWEAK"
# 					)

# 					# Determine the build using git.
# 					execute_process(COMMAND "${GIT}" log -1 "--pretty=format:g%h" --abbrev=8 HEAD WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} RESULT_VARIABLE GIT_RESULT OUTPUT_VARIABLE GIT_OUTPUT ERROR_VARIABLE GIT_ERROR OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
# 					if((GIT_RESULT EQUAL 0) AND (NOT "${GIT_OUTPUT}" STREQUAL ""))
# 						version(MODIFY _VERSION "${_VERSION}" COMPRESS
# 							BUILD "${GIT_OUTPUT}"
# 							REQUIRE "PATCH;TWEAK"
# 						)
# 					else()
# 						message(WARNING "Failed to detect build version with 'git'.")
# 					endif()
# 				else()
# 					message(WARNING "Failed to automatically detect version with 'git'.")
# 				endif()
# 			endif()
# 		else()
# 			message(WARNING "'git' not found, automatic version detection disabled.")
# 		endif()
# 	else()
# 		message(STATUS "Not a git repository, automatic version detection disabled.")
# 	endif()

# 	# Allow manual overrides of the detected version.
# 	if(${PREFIX}VERSION)
# 		version(PARSE _VERSION_CFG "${${PREFIX}VERSION}" REQUIRE "PATCH;TWEAK")
# 		if("${_VERSION_CFG_BUILD}" STREQUAL "")
# 			set(_VERSION_CFG_BUILD "${_VERSION_BUILD}")
# 		endif()
# 		version(GENERATE _VERSION COMPRESS
# 			MAJOR "${_VERSION_CFG_MAJOR}"
# 			MINOR "${_VERSION_CFG_MINOR}"
# 			PATCH "${_VERSION_CFG_PATCH}"
# 			TWEAK "${_VERSION_CFG_TWEAK}"
# 			PRERELEASE "${_VERSION_CFG_PRERELEASE}"
# 			BUILD "${_VERSION_CFG_BUILD}"
# 		)
# 	endif()

# 	# Fix up missing parts of the Version
# 	version(PARSE _VERSION "${_VERSION}" REQUIRE "PATCH;TWEAK")

# 	set(_VERSION_THIN "${_VERSION_MAJOR}.${_VERSION_MINOR}.${_VERSION_PATCH}")
# 	if(NOT (_VERSION_PRERELEASE STREQUAL ""))
# 		set(_VERSION_THIN "${_VERSION_THIN}${_VERSION_PRERELEASE}${_VERSION_TWEAK}")
# 	endif()
# 	if(NOT (VERSION_COMMIT STREQUAL ""))
# 		set(_VERSION_THIN "${_VERSION_THIN}-${_VERSION_BUILD}")
# 	endif()

# 	# Parse & Log the detected version.
# 	message(STATUS "Version ${_VERSION_THIN}")
# else()
# 	message(WARNING "Building without versioning support...")
# endif()

################################################################################
# Project
################################################################################

# if(EXISTS "cmake/version/version.cmake")
# 	version(GENERATE PROJECT_VERSION
# 		MAJOR "${_VERSION_MAJOR}"
# 		MINOR "${_VERSION_MINOR}"
# 		PATCH "${_VERSION_PATCH}"
# 		TWEAK "${_VERSION_TWEAK}"
# 		REQUIRE "PATCH;TWEAK"
# 	)
# else()
	set(PROJECT_VERSION "0.0.0.0")
# endif()
project(
	${PROJECT_NAME}
	VERSION ${PROJECT_VERSION}
)

check_ipo_supported(RESULT HAS_IPO)
add_executable(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES
	# Always generate position independent code.
	POSITION_INDEPENDENT_CODE ON

	# Enable IPO/LTO if available
	INTERPROCEDURAL_OPTIMIZATION ${HAS_IPO}

	# Set C & C++ Standard and Extensions
	C_STANDARD 17
	C_STANDARD_REQUIRED ON
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF

	# Remove prefix from generated files.
	PREFIX ""
	IMPORT_PREFIX ""

	# Never treat warnings as errors.
	COMPILE_WARNING_AS_ERROR OFF
)
if(MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE
		"/EHa"
		"/Zc:__cplusplus"
	)
endif()

if(WIN32)
	target_link_libraries(${PROJECT_NAME} PRIVATE "bcrypt")
endif()

file(GLOB_RECURSE PRIVATE_FILES FOLLOW_SYMLINKS CONFIGURE_DEPENDS "source/*")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/source" PREFIX "Private" FILES ${PRIVATE_FILES})

file(GLOB_RECURSE PUBLIC_FILES FOLLOW_SYMLINKS CONFIGURE_DEPENDS "include/*")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "Public" FILES ${PUBLIC_FILES})

target_sources(${PROJECT_NAME}
    PRIVATE
        ${PRIVATE_FILES}
		${PUBLIC_FILES}
)

if(EXISTS "cmake/clang/Clang.cmake")
	generate_compile_commands_json(
		TARGETS ${PROJECT_NAME}
	)
	clang_tidy(
		TARGETS ${PROJECT_NAME}
		VERSION 15.0.0
		GLOBAL
	)
	clang_format(
		TARGETS ${PROJECT_NAME}
		VERSION 15.0.0
		DEPENDENCY
		GLOBAL
	)
endif()
