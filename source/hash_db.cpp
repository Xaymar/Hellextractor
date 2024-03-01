// Copyright 2024 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "hash_db.hpp"
#include <algorithm>
#include <fstream>
#include <locale>

#include "hasher.hpp"
#include "string_printf.hpp"

hellextractor::hash_db::~hash_db() {}

hellextractor::hash_db::hash_db() : _strings(), _hashes() {}

hellextractor::hash_db::hash_db(std::filesystem::path db_file) : _strings(), _hashes()
{
	if (!std::filesystem::exists(db_file)) {
		throw std::runtime_error(string_printf("File '%s' does not exist", db_file.generic_u8string().c_str()));
	}

	std::ifstream file(db_file, std::ios::in);
	if (!file.is_open()) {
		throw std::runtime_error(string_printf("Failed to open file '%s'", db_file.generic_u8string().c_str()));
	}

	auto hasher = hellextractor::hash::instance::create(hash::type::MURMUR_64A);

	// Parse line by line.
	std::string line;
	while (!file.eof()) {
		std::getline(file, line);

		for (size_t idx = 0; (idx + 1) < line.length(); idx++) {
			auto c0 = line[idx];
			auto c1 = line[idx + 1];
			if (c0 == '/' && (c0 == c1)) {
				line = line.substr(0, idx);
				break;
			}
		}

		// Trim string.
		line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch) { return !std::isspace(ch); }));
		line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), line.end());

		// Skip completely empty lines.
		if (line.empty()) {
			continue;
		}

		// Everything else gets added.
		stingray::hash_t hash = *reinterpret_cast<stingray::hash_t const*>(hasher->hash(line.data(), line.length()).data());

		_hashes.emplace(line, hash);
		_strings.emplace(hash, line);
	}
}

std::map<stingray::hash_t, std::string> const& hellextractor::hash_db::strings()
{
	return _strings;
}

std::map<std::string, stingray::hash_t> const& hellextractor::hash_db::hashes()
{
	return _hashes;
}
