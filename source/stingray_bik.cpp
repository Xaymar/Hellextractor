// Copyright 2024 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "stingray_bik.hpp"

stingray::bik::~bik() {}

stingray::bik::bik(stingray::data_110000F0::meta_t meta) : _meta(meta)
{
	_header         = reinterpret_cast<decltype(_header)>(_meta.main);
	_data_header    = reinterpret_cast<decltype(_data_header)>(reinterpret_cast<uint8_t const*>(_header) + sizeof(header_t));
	_data_header_sz = _meta.main_size - sizeof(header_t);
	_data           = reinterpret_cast<decltype(_data)>(_meta.stream ? _meta.stream : _meta.gpu);
	_data_sz        = _meta.stream_size ? _meta.stream_size : _meta.gpu_size;
}

size_t stingray::bik::size()
{
	return _data_header_sz + _data_sz;
}

std::string stingray::bik::extension()
{
	return "bik";
}

std::list<std::pair<void const*, size_t>> stingray::bik::sections()
{
	return {
		{_data_header, _data_header_sz},
		{_data, _data_sz},
	};
}
