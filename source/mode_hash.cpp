// Copyright 2024 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <cinttypes>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include "endian.h"
#include "hasher.hpp"
#include "main.hpp"
#include "string_printf.hpp"

static std::string_view constexpr name = "hash";
static std::string_view constexpr help = "Convert text into hash";

int32_t mode_hash(std::vector<std::string> const& args)
{
	auto inst = hellextractor::hash::instance::create(hellextractor::hash::type::MURMURHASH64A);

	bool show_help = false;
	if (args.size() == 1) {
		show_help = true;
	} else {
		if ((args[1] == "-h") || (args[1] == "--help")) {
			show_help = true;
		}
	}
	if (show_help) {
		auto self = std::filesystem::path(args[0]).filename();
		std::cout << self.generic_string() << " " << name << " [options] name [name [name [...]]]" << std::endl;
		std::cout << "Prints out the matching hash for every name provided on separate lines." << std::endl;
		std::cout << std::endl;
		std::cout << "Options" << std::endl;
		std::cout << "  -h, --help       Show this help" << std::endl;
		std::cout << std::endl;
		return 1;
	}

	for (size_t argn = 1; argn < args.size(); argn++) {
		auto     key   = args[argn];
		auto     hash  = inst->hash(key.data(), key.size());
		uint64_t rhash = *reinterpret_cast<uint64_t const*>(hash.data());
		std::cout << string_printf("%016" PRIx64 " %s", htobe64(rhash), key.c_str()) << std::endl;
	}

	return 0;
}
static auto instance = hellextractor::mode(std::string(name), std::string(help), mode_hash);
