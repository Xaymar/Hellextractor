// Copyright 2024 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "hd2_data.hpp"

helldivers2::data::data(std::filesystem::path path)
{
	_main_path = std::filesystem::absolute(path).replace_extension();
	_main_size = std::filesystem::file_size(_main_path);
	_main      = {_main_path};

	_stream_path = std::filesystem::absolute(path).replace_extension("stream");
	_stream_size = 0;
	if (std::filesystem::exists(_stream_path)) {
		_stream_size = std::filesystem::file_size(_stream_path);
		_stream      = {_stream_path};
	}

	_gpu_path = std::filesystem::absolute(path).replace_extension("gpu_resources");
	_gpu_size = 0;
	if (std::filesystem::exists(_gpu_path)) {
		_gpu_size = std::filesystem::file_size(_gpu_path);
		_gpu      = {_gpu_path};
	}

	_ptr      = reinterpret_cast<decltype(_ptr)>(&_main);
	_ptr_type = reinterpret_cast<decltype(_ptr_type)>(&_main + sizeof(header_t));
	_ptr_file = reinterpret_cast<decltype(_ptr_file)>(&_main + sizeof(header_t) + sizeof(type_t) * _ptr->types);
	_ptr_data = reinterpret_cast<decltype(_ptr_data)>(&_main + sizeof(header_t) + sizeof(type_t) * _ptr->types + sizeof(file_t) * _ptr->files);
}

size_t helldivers2::data::types() const
{
	return _ptr->types;
}

helldivers2::data::type_t const& helldivers2::data::type(size_t idx) const
{
	if (idx >= types()) {
		throw std::out_of_range("idx >= edx");
	}

	return _ptr_type[idx];
}

size_t helldivers2::data::files() const
{
	return _ptr->files;
}

helldivers2::data::file_t const& helldivers2::data::file(size_t idx) const
{
	if (idx >= files()) {
		throw std::out_of_range("idx >= edx");
	}

	return _ptr_file[idx];
}

helldivers2::data::meta_t helldivers2::data::meta(size_t idx) const
{
	if (idx >= files()) {
		throw std::out_of_range("idx >= edx");
	}

	return meta_t{
		.file        = file(idx),
		.main_size   = main_size(idx),
		.main        = main_data(idx),
		.stream_size = stream_size(idx),
		.stream      = stream_data(idx),
		.gpu_size    = gpu_size(idx),
		.gpu         = gpu_data(idx),
	};
}

bool helldivers2::data::has_main(size_t idx) const
{
	if (idx >= files()) {
		throw std::out_of_range("idx >= edx");
	}

	return (file(idx).size > 0);
}

uint8_t const* helldivers2::data::main_data(size_t idx) const
{
	if (idx >= files()) {
		throw std::out_of_range("idx >= edx");
	}

	if (!has_main(idx)) {
		return nullptr;
	}

	if (file(idx).offset >= _main_size) {
		throw std::overflow_error("offset >= size");
	} else if (file(idx).size + file(idx).offset > _main_size) {
		throw std::overflow_error("offset+size > size");
	}

	return &_main + file(idx).offset;
}

size_t helldivers2::data::main_size(size_t idx) const
{
	if (idx >= files()) {
		throw std::out_of_range("idx >= edx");
	}

	return file(idx).size;
}

bool helldivers2::data::has_stream(size_t idx) const
{
	if (idx >= files()) {
		throw std::out_of_range("idx >= edx");
	}

	return (_stream.has_value()) && (file(idx).stream_size > 0);
}

uint8_t const* helldivers2::data::stream_data(size_t idx) const
{
	if (idx >= files()) {
		throw std::out_of_range("idx >= edx");
	}

	if (!has_stream(idx)) {
		return nullptr;
	}

	if (file(idx).stream_offset >= _stream_size) {
		throw std::overflow_error("offset >= size");
	} else if (file(idx).stream_size + file(idx).stream_offset > _stream_size) {
		throw std::overflow_error("offset+size > size");
	}

	return (&(_stream.value()) + file(idx).stream_offset);
}

size_t helldivers2::data::stream_size(size_t idx) const
{
	if (idx >= files()) {
		throw std::out_of_range("idx >= edx");
	}

	if (!has_stream(idx)) {
		return 0;
	}

	return file(idx).stream_size;
}

bool helldivers2::data::has_gpu(size_t idx) const
{
	if (idx >= files()) {
		throw std::out_of_range("idx >= edx");
	}

	return (_gpu.has_value()) && (file(idx).gpu_size > 0);
}

uint8_t const* helldivers2::data::gpu_data(size_t idx) const
{
	if (idx >= files()) {
		throw std::out_of_range("idx >= edx");
	}

	if (!has_gpu(idx)) {
		return nullptr;
	}

	if (file(idx).gpu_offset >= _gpu_size) {
		throw std::overflow_error("offset >= size");
	} else if (file(idx).gpu_size + file(idx).gpu_offset > _gpu_size) {
		throw std::overflow_error("offset+size > size");
	}

	return (&(_gpu.value()) + file(idx).gpu_offset);
}

size_t helldivers2::data::gpu_size(size_t idx) const
{
	if (idx >= files()) {
		throw std::out_of_range("idx >= edx");
	}

	if (!has_gpu(idx)) {
		return 0;
	}

	return file(idx).gpu_size;
}
