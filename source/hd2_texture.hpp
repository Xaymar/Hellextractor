// Copyright 2024 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once
#include <cinttypes>
#include <cstddef>
#include <list>
#include "hd2_data.hpp"

namespace helldivers2 {
	class texture {
		public:
		struct streamable_section_t {
			uint32_t offset;
			uint32_t size;
			uint16_t width;
			uint16_t height;
		};

		struct header_t {
			uint32_t             __unk0;
			uint32_t             __unk1;
			uint32_t             __unk2;
			streamable_section_t sections[15];
		};

		private:
		helldivers2::data_110000F0::meta_t _meta;
		header_t const*                    _header;
		uint8_t const*                     _data_header;
		size_t                             _data_header_sz;
		uint8_t const*                     _data;
		size_t                             _data_sz;

		public:
		~texture();
		texture(helldivers2::data_110000F0::meta_t meta);

		size_t size();

		std::string extension();

		std::list<std::pair<void const*, size_t>> sections();
	};
} // namespace helldivers2
