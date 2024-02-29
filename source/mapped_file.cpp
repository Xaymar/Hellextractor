// Copyright 2024 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "mapped_file.hpp"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#endif

mapped_file::mapped_file() : _path(), _file(), _map(), _ptr() {}

mapped_file::mapped_file(std::filesystem::path path) : _path(path)
{
#ifdef WIN32
	_file.reset(CreateFileA(reinterpret_cast<LPCSTR>(path.generic_u8string().c_str()), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL), [](void* p) { CloseHandle(p); });
	if (_file.get() == INVALID_HANDLE_VALUE) {
		throw std::runtime_error("CreateFileA failed.");
	}

	_map.reset(CreateFileMapping(_file.get(), NULL, PAGE_READONLY, 0, 0, NULL), [](void* p) { CloseHandle(p); });
	if (_map.get() == INVALID_HANDLE_VALUE) {
		throw std::runtime_error("CreateFileMapping failed.");
	}

	_ptr = reinterpret_cast<decltype(_ptr)>(MapViewOfFile(_map.get(), FILE_MAP_READ, 0, 0, 0));
	if (!_ptr) {
		throw std::runtime_error("MapViewOfFile failed.");
	}
#else
	_file.reset(reinterpret_cast<void*>(open(path.generic_u8string().c_str(), O_NOATIME | O_RDONLY)), [](void* p) { close(reinterpret_cast<int>(p)); });
	if (reinterpret_cast<int>(_file.get()) == -1) {
		throw std::runtime_error("open failed.");
	}

	_map.reset(reinterpret_cast<void*>(mmap(nullptr, std::filesystem::file_size(path), PROT_READ, MAP_NORESERVE)), [](void* p) { munmap(p); });
	if (!_map) {
		throw std::runtime_error("mmap failed.");
	}

	_ptr = reinterpret_cast<decltype(_ptr)>(_map.get());
#endif
}

mapped_file::~mapped_file()
{
	_map.reset();
	_file.reset();
}

uint8_t const* mapped_file::operator&() const
{
	return _ptr;
}

uint8_t const* mapped_file::operator*() const
{
	return _ptr;
}

mapped_file::operator void const*() const
{
	return _ptr;
}

mapped_file::operator uint8_t const*() const
{
	return _ptr;
}
