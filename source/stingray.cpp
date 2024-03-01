// Copyright 2024 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "stingray.hpp"
#include "endian.h"

stingray::hash_t& stingray::hash_t::operator=(stingray::hash_t const& other)
{
	_value = other._value;
	return *this;
}

stingray::hash_t& stingray::hash_t::operator=(stingray::hash_t&& other) noexcept
{
	_value = other._value;
	return *this;
}

stingray::hash_t& stingray::hash_t::operator=(uint64_t other) noexcept
{
	_value = other;
	return *this;
}

bool stingray::hash_t::operator==(stingray::hash_t const& rhs) const noexcept
{
	return _value == rhs._value;
}

bool stingray::hash_t::operator<=(stingray::hash_t const& rhs) const noexcept
{
	return _value <= rhs._value;
}

bool stingray::hash_t::operator>=(stingray::hash_t const& rhs) const noexcept
{
	return _value >= rhs._value;
}

bool stingray::hash_t::operator<(stingray::hash_t const& rhs) const noexcept
{
	return _value < rhs._value;
}

bool stingray::hash_t::operator>(stingray::hash_t const& rhs) const noexcept
{
	return _value > rhs._value;
}

stingray::hash_t::operator uint64_t() const noexcept
{
	return htobe64(_value);
}

stingray::hash_t::operator stingray::thin_hash_t() const noexcept
{
	return {htobe64(_value) >> 32};
}

stingray::hash_t::operator uint32_t() const noexcept
{
	return htobe64(_value) >> 32;
}

stingray::thin_hash_t& stingray::thin_hash_t::operator=(stingray::thin_hash_t const& other)
{
	_value = other._value;
	return *this;
}

stingray::thin_hash_t& stingray::thin_hash_t::operator=(stingray::thin_hash_t&& other) noexcept
{
	_value = other._value;
	return *this;
}

stingray::thin_hash_t& stingray::thin_hash_t::operator=(uint32_t other) noexcept
{
	_value = other;
	return *this;
}

bool stingray::thin_hash_t::operator==(thin_hash_t const& rhs) const noexcept
{
	return _value == rhs._value;
}

bool stingray::thin_hash_t::operator<=(thin_hash_t const& rhs) const noexcept
{
	return _value <= rhs._value;
}

bool stingray::thin_hash_t::operator<(thin_hash_t const& rhs) const noexcept
{
	return _value < rhs._value;
}

bool stingray::thin_hash_t::operator>=(thin_hash_t const& rhs) const noexcept
{
	return _value >= rhs._value;
}

bool stingray::thin_hash_t::operator>(thin_hash_t const& rhs) const noexcept
{
	return _value > rhs._value;
}

stingray::thin_hash_t::operator uint32_t() const noexcept
{
	return htobe32(_value);
}
