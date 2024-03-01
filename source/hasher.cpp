// Copyright 2024 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "hasher.hpp"
#include <cstring>
#include <functional>
#include <map>
#include <memory>
#include <stdexcept>

typedef std::function<std::shared_ptr<hellextractor::hash::instance>()> hash_fn_t;
typedef std::map<hellextractor::hash::type, hash_fn_t>                  hash_list_t;

struct hash_list {
	hash_list(hellextractor::hash::type type, hash_fn_t fn)
	{
		hash_list::get().emplace(type, fn);
	}

	static hash_list_t& get()
	{
		static hash_list_t list;
		return list;
	}
};

hellextractor::hash::instance::instance() {}

hellextractor::hash::instance::~instance() {}

std::shared_ptr<hellextractor::hash::instance> hellextractor::hash::instance::create(hellextractor::hash::type type)
{
	if (auto kv = hash_list::get().find(type); kv != hash_list::get().end()) {
		return kv->second();
	}
	return nullptr;
}

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <bcrypt.h>

template<class T, class D = std::default_delete<T>>
struct shared_ptr_with_deleter : public std::shared_ptr<T> {
	explicit shared_ptr_with_deleter(T* t = nullptr) : std::shared_ptr<T>(t, D()) {}

	// Reset function, as it also needs to properly set the deleter.
	void reset(T* t = nullptr)
	{
		std::shared_ptr<T>::reset(t, D());
	}
};

struct BCRYPT_ALG_HANDLE_deleter {
	void operator()(BCRYPT_ALG_HANDLE p) const
	{
		BCryptCloseAlgorithmProvider(p, 0);
	}
};
using bcrypt_alg_handle_t = shared_ptr_with_deleter<void, BCRYPT_ALG_HANDLE_deleter>;

struct BCRYPT_HASH_HANDLE_deleter {
	void operator()(BCRYPT_HASH_HANDLE p) const
	{
		BCryptDestroyHash(p);
	}
};
using bcrypt_hash_handle_t = shared_ptr_with_deleter<void, BCRYPT_HASH_HANDLE_deleter>;

class hash_bcrypt : public hellextractor::hash::instance {
	std::shared_ptr<void> _alg;
	std::vector<char>     _objbuf;
	std::vector<char>     _buf;

	public:
	hash_bcrypt(hellextractor::hash::type type)
	{
		const wchar_t* algorithm = BCRYPT_MD5_ALGORITHM;

		switch (type) {
		case hellextractor::hash::type::SHA128:
			algorithm = BCRYPT_SHA1_ALGORITHM;
			break;
		case hellextractor::hash::type::SHA256:
			algorithm = BCRYPT_SHA256_ALGORITHM;
			break;
		case hellextractor::hash::type::SHA512:
			algorithm = BCRYPT_SHA512_ALGORITHM;
			break;
		case hellextractor::hash::type::MD2:
			algorithm = BCRYPT_MD2_ALGORITHM;
			break;
		case hellextractor::hash::type::MD4:
			algorithm = BCRYPT_MD4_ALGORITHM;
			break;
		case hellextractor::hash::type::MD5:
			algorithm = BCRYPT_MD5_ALGORITHM;
			break;
		default:
			throw std::runtime_error("Don't know how we got here, but now we have a problem.");
		}

		BCRYPT_ALG_HANDLE alghandle;
		if (auto status = BCryptOpenAlgorithmProvider(&alghandle, BCRYPT_MD5_ALGORITHM, NULL, 0); status) {
			throw std::runtime_error("BCryptOpenAlgorithmProvider");
		}
		_alg.reset(alghandle, [](void* p) { BCryptCloseAlgorithmProvider(p, 0); });

		ULONG bytes_copied;
		DWORD hashobj_length;
		if (auto status = BCryptGetProperty(_alg.get(), BCRYPT_OBJECT_LENGTH, (PUCHAR)&hashobj_length, sizeof(DWORD), &bytes_copied, 0); status) {
			throw std::runtime_error("BCryptGetProperty BCRYPT_OBJECT_LENGTH");
		}
		_objbuf.resize(hashobj_length, 0);

		DWORD hash_length;
		if (auto status = BCryptGetProperty(_alg.get(), BCRYPT_HASH_LENGTH, (PUCHAR)&hash_length, sizeof(DWORD), &bytes_copied, 0); status) {
			throw std::runtime_error("BCryptGetProperty BCRYPT_HASH_LENGTH");
		}
		_buf.resize(hash_length, 0);
	}

	std::vector<char> hash(void const* data, size_t length) override
	{
		BCRYPT_HASH_HANDLE hashhandle;
		if (auto status = BCryptCreateHash(_alg.get(), &hashhandle, (PUCHAR)_objbuf.data(), _objbuf.size(), NULL, 0, 0); status) {
			throw std::runtime_error("BCryptCreateHash");
		}
		std::shared_ptr<void> hasher(hashhandle, [](void* p) { BCryptDestroyHash(p); });

		if (auto status = BCryptHashData(hasher.get(), (PBYTE)data, length, 0); status) {
			throw std::runtime_error("BCryptHashData");
		}
		if (auto status = BCryptFinishHash(hasher.get(), (PBYTE)_buf.data(), _buf.size(), 0); status) {
			throw std::runtime_error("BCryptFinishHash");
		}

		return _buf;
	}
};

class hash_sha128 : public hash_bcrypt {
	public:
	hash_sha128() : hash_bcrypt(hellextractor::hash::type::SHA128){};
};
static auto hash_sha128_factory = hash_list(hellextractor::hash::type::SHA128, []() { return std::make_shared<hash_sha128>(); });

class hash_sha256 : public hash_bcrypt {
	public:
	hash_sha256() : hash_bcrypt(hellextractor::hash::type::SHA256){};
};
static auto hash_sha256_factory = hash_list(hellextractor::hash::type::SHA256, []() { return std::make_shared<hash_sha256>(); });

class hash_sha512 : public hash_bcrypt {
	public:
	hash_sha512() : hash_bcrypt(hellextractor::hash::type::SHA512){};
};
static auto hash_sha512_factory = hash_list(hellextractor::hash::type::SHA512, []() { return std::make_shared<hash_sha512>(); });

class hash_md2 : public hash_bcrypt {
	public:
	hash_md2() : hash_bcrypt(hellextractor::hash::type::MD2){};
};
static auto hash_md2_factory = hash_list(hellextractor::hash::type::MD2, []() { return std::make_shared<hash_md2>(); });

class hash_md4 : public hash_bcrypt {
	public:
	hash_md4() : hash_bcrypt(hellextractor::hash::type::MD4){};
};
static auto hash_md4_factory = hash_list(hellextractor::hash::type::MD4, []() { return std::make_shared<hash_md4>(); });

class hash_md5 : public hash_bcrypt {
	public:
	hash_md5() : hash_bcrypt(hellextractor::hash::type::MD5){};
};
static auto hash_md5_factory = hash_list(hellextractor::hash::type::MD5, []() { return std::make_shared<hash_md5>(); });
#endif

class hash_murmur_64a : public hellextractor::hash::instance {
	std::vector<char> _buf;

	public:
	hash_murmur_64a() : hellextractor::hash::instance()
	{
		_buf.resize(sizeof(uint64_t), 0);
	}

	std::vector<char> hash(void const* ptr, size_t length) override
	{
		constexpr uint64_t seed   = 0;
		constexpr uint64_t mix    = 0xC6A4A7935BD1E995llu;
		const int          shifts = 47;

		uint64_t hash = seed ^ (length * mix);

		uint64_t const* data = reinterpret_cast<uint64_t const*>(ptr);
		uint64_t const* end  = (length / sizeof(uint64_t)) + data;

		while (data != end) {
			// Get value at data, then increment data. Neat little C/C++ trick that really just looks confusing.
			uint64_t key = *data++;

			key *= mix;
			key ^= key >> shifts;
			key *= mix;

			hash ^= key;
			hash *= mix;
		}
		const unsigned char* data2 = (const unsigned char*)data;

		switch (length & 7) {
		case 7:
			hash ^= (static_cast<uint64_t>(data2[6]) << 48);
		case 6:
			hash ^= (static_cast<uint64_t>(data2[5]) << 40);
		case 5:
			hash ^= (static_cast<uint64_t>(data2[4]) << 32);
		case 4:
			hash ^= (static_cast<uint64_t>(data2[3]) << 24);
		case 3:
			hash ^= (static_cast<uint64_t>(data2[2]) << 16);
		case 2:
			hash ^= (static_cast<uint64_t>(data2[1]) << 8);
		case 1:
			hash ^= (static_cast<uint64_t>(data2[0]));
		};

		hash *= mix;
		hash ^= hash >> shifts;

		hash *= mix;
		hash ^= hash >> shifts;

		memcpy(_buf.data(), &hash, sizeof(uint64_t));
		return _buf;
	}
};
static auto hash_murmur_64a_fac = hash_list(hellextractor::hash::type::MURMUR_64A, []() { return std::make_shared<hash_murmur_64a>(); });

class hash_murmur_stingray32 : public hash_murmur_64a {
	std::vector<char> _buf;

	public:
	hash_murmur_stingray32() : hash_murmur_64a() {}

	std::vector<char> hash(void const* ptr, size_t length) override
	{
		// Stingray's 32bit hashes are the upper 32 bits of the hash.
		auto     buf  = hash_murmur_64a::hash(ptr, length);
		uint64_t hash = *reinterpret_cast<uint64_t*>(buf.data());
		buf.resize(4);
		*reinterpret_cast<uint32_t*> = hash >> 32;
		return _buf;
	}
};
static auto hash_murmur_stingray32_fac = hash_list(hellextractor::hash::type::MURMUR_STINGRAY32, []() { return std::make_shared<hash_murmur_stingray32>(); });
