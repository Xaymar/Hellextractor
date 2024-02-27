// Copyright 2024 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// ToDo:
// - Look into ICU for Unicode handling: https://icu.unicode.org/

#include "main.hpp"
#include <cinttypes>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <list>
#include <map>

namespace hellextractor {
	struct mode_info {
		std::string     name;
		std::string     help;
		mode_function_t fn;

		typedef std::map<std::string, mode_info> mode_list_t;

		static mode_list_t& get()
		{
			static mode_list_t list;
			return list;
		}
	};

	mode::mode(std::string name, std::string help, mode_function_t fn)
	{
		mode_info::get().try_emplace(name, mode_info{name, help, fn});
	}
} // namespace hellextractor

int main(int argc, const char** argv)
try {
	bool show_help = false;

	if (argc == 1) {
		show_help = true;
	} else {
		std::vector<std::string> args;
		args.push_back(argv[0]);
		for (size_t argn = 2; argn < argc; ++argn) { // Skip the first argument, which is always the mode itself.
			args.push_back(argv[argn]);
		}

		if (auto kv = hellextractor::mode_info::get().find(argv[1]); kv != hellextractor::mode_info::get().end()) {
			return kv->second.fn(args);
		} else {
			show_help = true;
		}
	}

	if (show_help) {
		auto self = std::filesystem::path(argv[0]).filename();
		std::cout << self.generic_string() << " <mode> ..." << std::endl;
		std::cout << std::endl;

		for (auto const& kv : hellextractor::mode_info::get()) {
			std::cout << self.generic_string() << " " << kv.first << " ..." << std::endl;
			std::cout << "    " << kv.second.help << std::endl;
		}

		std::cout << std::endl;
		std::cout << "For help with each mode, call the mode with -h as further arguments." << std::endl;
		return 1;
	}
	return 0;

	//// Parse arguments
	//for (size_t argn = 1; argn < argc; ++argn) {
	//	std::string_view arg = argv[argn];
	//	if (("-m" == arg) || ("--mode" == arg)) {
	//	} else if (("-o" == arg) || ("--output" == arg)) {
	//		if ((argn + 1) < argc) {
	//			std::string_view val = argv[++argn];
	//			output               = std::filesystem::absolute(val);
	//		} else {
	//			throw std::runtime_error("String expected, got EOL instead");
	//		}
	//	} else if ('-' == arg[0]) {
	//		throw std::runtime_error(string_printf("Unrecognized format: %s", arg.data()));
	//	} else {
	//		unparsed.push_back(std::string(arg));
	//	}
	//}


	return 0;
} catch (const std::exception& ex) {
	std::cerr << "Exception: " << ex.what() << std::endl;
	return 1;
} catch (...) {
	std::cerr << "Encountered unknown exception." << std::endl;
	return 1;
}

/*{ // least efficient brute force search I could come up with. Just to verify the hash function works.
	std::set<uint64_t> hashes = {
		0x05106B81DCD58A13, 0x18DEAD01056B72E9, 0x1D59BD6687DB6B33, 0x250E0A11AC8E26F8, 0x27862FE24795319C, 0x2A690FD348FE9AC5, 0x504B55235D21440E, 0x535A7BD3E650D799, 0x57A13425279979D7, 0x5F7203C8F280DAB8, 0x9199BB50B6896F02, 0x9831CA893B0D087D, 0x931E336D7646CC26, 0x9EFE0A916AAE7880, 0xA14E8DFA2CD117E2, 0xA486D4045106165C, 0xA8193123526FAD64, 0xAA5965F03029FA18, 0xAB2F78E885F513C6, 0xAF32095C82F2B070, 0xC4F0F4BE7FB0C8D6, 0xCD4238C6A0C69E32, 0xD7014A50477953E0, 0xE0A48D0BE9A7453F, 0xE985C5F61C169997, 0xEAC0B497876ADEDF, 0xF7505933166D6755, 0xFE73C7DCFF8A7CA5, 0xac2b738a374cf583, 0xdcfb9e18fff13984, 0x44957ef5a3a368e4, 0x3eed05ba83af5090, 0x7ffdb779b04e4ed1, 0xe301e8af94e3b5a3, 0xb7893adf7567506a, 0xf7b361bde8b4dd03, 0xfe9754bd19814a47, 0x0001a6d201e87086, 0x82645835e6b73232, 0x69108ded1e3e634b, 0x8fd0d44d20650b68, 0xed74b77c6d447d84, 0x92d3ee038eeb610d, 0x6a00deed7d3b235a, 0xd526a27da14f1dc5, 0xfa4a8e091a91201e, 0xa62f9297dc969e85, 0x48ff313713a997a1, 0x3fcdd69156a46417, 0xb277b11fe4a61d37, 0x169de9566953d264, 0x3b1fa9e8f6bac374, 0xad9c6d9ed1e5e77a, 0x5f7203c8f280dab8, 0x22bd082459e868d8, 0xbf21403a3ab0bbb1, 0x46cab355bd8d4d37, 0xd37e11a77baafc01, 0x9d0a795bfe818d19, 0xcce8d5b5f5ae333f, 0xe5ee32a477239a93, 0x9e5c3cc74575aeb5, 0xa27b4d04a9ba6f9e, 0xe9fc9ea7042e5ec0, 0x90641b51c98b7aac, 0xd8b27864a97ffdd7, 0xf97af9983c05b950, 0xbb0279e548747a0a, 0x0d972bab10b40fd3, 0xad2d3fa30d9ab394, 0x91f98cecac930de8, 0x94616b1760e1c463, 0x99736be1fff739a4, 0x00a3e6c59a2b9c6c, 0x19c792357c99f49b, 0xe0a48d0be9a7453f, 0x8c074c8f95e7841b, 0x6ad9817aa72d9533, 0x786f65c00a816b19, 0x76015845a6003765, 0x712d6e3dd1024c9c, 0xa99510c6e86dd3c2, 0x2bbcabe5074ade9e, 0x52014e20a7fdccef, 0x7b82d55316f98c9b, 0xde8c9d00247f8fc6, 0x0605775c0481181b, 0x293bb3b1982c6d5a, 0xb6b3099f78886ff9, 0xd596cd750e4e279f,
	};

	// can convert this to a depth search?
	// Steps would be:
	// 1. Loop through valid for single characters
	// 2. Loop through valid for two characters
	//   1. Loop through the 2nd character until at max.
	//   2. Increment 1st chracter, repeat 1 until at max.
	// etc.
	// May end up significantly faster.
	constexpr size_t     max_depth = 16;
	std::vector<char>    buffer(max_depth, 0);
	std::vector<uint8_t> counter(max_depth, 0);
	std::string_view     valid = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";

	for (size_t depth = 1; depth < max_depth; ++depth) {
		printf("Depth: %zu\n", depth);
		bool breakout = false;
		while (!breakout) {
			// Update buffer and counter.
			// - Assign the proper value to the buffer at the location.
			for (size_t idx = 0; idx < depth; idx++) {
				buffer[idx] = valid[counter[idx]];
			}
			// - Update the counter and propagate upward if there is space.
			for (size_t idx = (depth - 1); idx < depth; idx--) {
				if (++counter[idx] < valid.size()) {
					break;
				} else {
					counter[idx] = 0; // Let it propagate upwards
					if (idx == 0) {
						breakout = true;
					}
				}
			}
			auto hash = murmurhash64A(buffer.data(), depth, 0);
			if (hashes.find(hash) != hashes.end()) {
				printf("  %018" PRIX64 " = %.*s\n", hash, (int)depth, buffer.data());
			}
		}
	}

	return 0;
}*/
/*{
std::string arg = "";
do {
	std::getline(std::cin, arg);
	printf("%016" PRIx64 "\n", murmurhash64A(arg.c_str(), arg.size(), 0));
} while (!arg.empty());
return 0;
}*/
