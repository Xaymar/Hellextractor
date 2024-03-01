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

namespace stingray {
	class thin_hash_t;

	class hash_t {
		uint64_t _value;

		public:
		constexpr hash_t(uint64_t value) : _value(value){};

		constexpr hash_t(stingray::hash_t const& other) : _value(other._value){};

		constexpr hash_t(stingray::hash_t&& other) noexcept : _value(other._value){};

		stingray::hash_t& operator=(stingray::hash_t const& other);

		stingray::hash_t& operator=(stingray::hash_t&& other) noexcept;

		stingray::hash_t& operator=(uint64_t other) noexcept;

		bool operator==(stingray::hash_t const& rhs) const noexcept;

		bool operator<=(stingray::hash_t const& rhs) const noexcept;

		bool operator<(stingray::hash_t const& rhs) const noexcept;

		bool operator>=(stingray::hash_t const& rhs) const noexcept;

		bool operator>(stingray::hash_t const& rhs) const noexcept;

		operator uint64_t() const noexcept;

		operator stingray::thin_hash_t() const noexcept;

		operator uint32_t() const noexcept;
	};

	class thin_hash_t {
		uint32_t _value;

		public:
		constexpr thin_hash_t(uint32_t value) : _value(value){};

		constexpr thin_hash_t(stingray::thin_hash_t const& other) : _value(other._value){};

		constexpr thin_hash_t(stingray::thin_hash_t&& other) noexcept : _value(other._value){};

		stingray::thin_hash_t& operator=(stingray::thin_hash_t const& other);

		stingray::thin_hash_t& operator=(stingray::thin_hash_t&& other) noexcept;

		stingray::thin_hash_t& operator=(uint32_t other) noexcept;

		bool operator==(stingray::thin_hash_t const& rhs) const noexcept;

		bool operator<=(stingray::thin_hash_t const& rhs) const noexcept;

		bool operator<(stingray::thin_hash_t const& rhs) const noexcept;

		bool operator>=(stingray::thin_hash_t const& rhs) const noexcept;

		bool operator>(stingray::thin_hash_t const& rhs) const noexcept;

		operator uint32_t() const noexcept;
	};

} // namespace stingray
