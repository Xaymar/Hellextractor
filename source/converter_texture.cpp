// Copyright 2024 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "converter_texture.hpp"
#include <fstream>
#include <string_view>
#include "converter.hpp"
#include "endian.h"
#include "hd2_texture.hpp"

static constexpr uint64_t         type            = 0x329ec6a0c63842cdull; // texture
static constexpr std::string_view section_texture = "texture";

static auto instance = hellextractor::converter::registry::do_register(
	std::list<uint64_t>{
		type,
	},
	[](helldivers2::data::meta_t meta) { return std::make_shared<hellextractor::converter::texture>(meta); });

hellextractor::converter::texture::~texture() {}

hellextractor::converter::texture::texture(helldivers2::data::meta_t meta) : base(meta), _texture(meta) {}

std::map<std::string, std::pair<size_t, std::string>> hellextractor::converter::texture::outputs()
{
	return {
		{std::string{section_texture},
		 {
			 _texture.size(),
			 _texture.extension(),
		 }},
	};
}

void hellextractor::converter::texture::extract(std::string section, std::filesystem::path path)
{
	if (section_texture == section) { // Extract "texture" section.
		std::ofstream stream{path, std::ios::trunc | std::ios::binary | std::ios::out};
		if (!stream || stream.bad() || !stream.is_open()) {
			throw std::runtime_error("Unable to open output file");
		}

		for (auto const& section : _texture.sections()) {
			stream.write(reinterpret_cast<char const*>(section.first), section.second);
		}

		stream.close();
	}
}
