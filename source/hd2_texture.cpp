// Copyright 2024 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "hd2_texture.hpp"

// Known information
// - gpu_resources only has mip level 2 and up again, which we won't need.
// - File appear to be DDS, and there is a reference to TGA as well.

helldivers2::texture::~texture() {}

helldivers2::texture::texture(helldivers2::data::meta_t meta) : _meta(meta)
{
	_header      = reinterpret_cast<decltype(_header)>(_meta.main);
	_data_header = reinterpret_cast<decltype(_data_header)>(reinterpret_cast<uint8_t const*>(_header) + sizeof(header_t));
	_data        = reinterpret_cast<decltype(_data)>(_meta.stream ? _meta.stream : _meta.gpu);
}

size_t helldivers2::texture::size()
{
	return _meta.main_size + _meta.stream_size;
}

std::string helldivers2::texture::extension()
{
#define TWOCC(a, b) ((a) | (b << 8))
#define FOURCC(a, b, c, d) ((a) | (b << 8) | (c << 16) | (d << 24))

	// Take a reasonable guess at what the actual file type is.
	if (_meta.main_size - sizeof(header_t) >= 4) {
		uint16_t magic16 = *reinterpret_cast<uint16_t const*>(_data_header);
		uint32_t magic32 = *reinterpret_cast<uint32_t const*>(_data_header);
		if (magic32 == FOURCC('D', 'D', 'S', ' ')) {
			return "dds";
		} else if (magic32 == FOURCC('.', 'P', 'N', 'G')) {
			return "png";
		}
	}

	// If we still don't know, return the default.
	return "texture";
}

std::list<std::pair<void const*, size_t>> helldivers2::texture::sections()
{
	return {
		{_data_header, _meta.main_size},
		{_data, _meta.stream_size ? _meta.stream_size : _meta.gpu_size},
	};
}
