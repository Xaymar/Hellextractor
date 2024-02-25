// Copyright 2024 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <algorithm>
#include <cinttypes>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

template<class T, class D = std::default_delete<T>>
struct shared_ptr_with_deleter : public std::shared_ptr<T> {
	explicit shared_ptr_with_deleter(T* t = nullptr) : std::shared_ptr<T>(t, D()) {}

	// Reset function, as it also needs to properly set the deleter.
	void reset(T* t = nullptr)
	{
		std::shared_ptr<T>::reset(t, D());
	}
};

struct half {
	uint16_t value;

	half()
	{
		operator=(0.0f);
	}
	half(float v)
	{
		operator=(v);
	}
	half(uint16_t v)
	{
		operator=(v);
	}

	half& operator=(float v)
	{
		auto as_uint = [](const float x) { return *(uint32_t*)&x; };

		// IEEE-754 16-bit floating-point format (without infinity): 1-5-10, exp-15, +-131008.0, +-6.1035156E-5, +-5.9604645E-8, 3.311 digits
		const uint32_t b = as_uint(v) + 0x00001000; // round-to-nearest-even: add last bit after truncated mantissa
		const uint32_t e = (b & 0x7F800000) >> 23; // exponent
		const uint32_t m = b & 0x007FFFFF; // mantissa; in line below: 0x007FF000 = 0x00800000-0x00001000 = decimal indicator flag - initial rounding
		value            = (b & 0x80000000) >> 16 | (e > 112) * ((((e - 112) << 10) & 0x7C00) | m >> 13) | ((e < 113) & (e > 101)) * ((((0x007FF000 + m) >> (125 - e)) + 1) >> 1) | (e > 143) * 0x7FFF; // sign : normalized : denormalized : saturate
	}

	half& operator=(uint16_t v)
	{
		value = v;
	}

	operator float() const
	{
		auto as_float = [](const uint32_t x) { return *(float*)&x; };
		auto as_uint  = [](const float x) { return *(uint32_t*)&x; };

		// IEEE-754 16-bit floating-point format (without infinity): 1-5-10, exp-15, +-131008.0, +-6.1035156E-5, +-5.9604645E-8, 3.311 digits
		const uint32_t e = (value & 0x7C00) >> 10; // exponent
		const uint32_t m = (value & 0x03FF) << 13; // mantissa
		const uint32_t v = as_uint((float)m) >> 23; // evil log2 bit hack to count leading zeros in denormalized format
		return as_float((value & 0x8000) << 16 | (e != 0) * ((e + 112) << 23 | m) | ((e == 0) & (m != 0)) * ((v - 37) << 23 | ((m << (150 - v)) & 0x007FE000))); // sign : normalized : denormalized
	}

	operator uint16_t() const
	{
		return value;
	}
};

#ifdef WIN32
#define NOMINMAX
#include <Windows.h>
#include <bcrypt.h>

struct HANDLE_deleter {
	void operator()(HANDLE p) const
	{
		CloseHandle(p);
	}
};
using win32_handle_t = shared_ptr_with_deleter<void, HANDLE_deleter>;

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

#else
#pragma error("Not supported yet, do some work to make it work.")
#endif

struct FILE_deleter {
	void operator()(FILE* p) const
	{
		if (p)
			fclose(p);
	}
};
using file_t = shared_ptr_with_deleter<FILE, FILE_deleter>;

namespace std {
	std::string format(std::string_view format, ...)
	{
		va_list args_size, args_data;
		va_start(args_size, format);
		va_copy(args_data, args_size);

		std::vector<char> buffer;
		// Resize buffer
		buffer.resize(vsnprintf(nullptr, 0, format.data(), args_size) + 1);
		va_end(args_size);

		// Format data.
		vsnprintf(buffer.data(), buffer.size(), format.data(), args_data);
		va_end(args_data);

		return {buffer.data(), buffer.data() + buffer.size() - 1}; // Ignore null terminator.
	}
} // namespace std

namespace hd2 {
	// Models are very tiny, so this can be used to scale them up.
	constexpr float mesh_scale = 1.;

	enum class vertex_element_type : uint32_t {
		_00_position = 0x00, // format = <float_type>[2..4], rest seems to be ignored
		_01_unknown  = 0x01, // format = 0x1A
		_04_texcoord = 0x04, // has index at [2]
		_05_unknown  = 0x05, // ??
		_06_unknown  = 0x06, // has index at [2]
		_07_normal   = 0x07, // Any of the (float_type)[3] formats.
	};

	enum class vertex_element_format : uint32_t {
		_00_implied_float  = 0x00, // 4 bytes, implied
		_01_implied_float2 = 0x01, // 8 bytes, implied
		_02_float3         = 0x02, // 12 bytes
		f32vec3            = _02_float3,
		_03_implied_float4 = 0x03, // 16 bytes, implied
		_04_unknown        = 0x04, // 4 bytes, ??
		_14_long4          = 0x14, // 32 bytes, (u)int32_t[4]
		_18_byte4          = 0x18, // 4 bytes, (u)int8_t[4]
		_19_unknown        = 0x19, // 4 bytes, ??
		_1A_unknown        = 0x1A, // 4 bytes, ??
		_1C_implied_half   = 0x1C, // 2 bytes, implied
		_1D_half2          = 0x1D, // 4 bytes
		f16vec2            = _1D_half2,
		_1E_implied_half3  = 0x1E, // 12 bytes, implied
		_1F_half4          = 0x1F, // 16 bytes
		f16vec4            = _1F_half4,
	};
	static size_t vertex_element_format_size(vertex_element_format format)
	{
		switch (format) {
		case hd2::vertex_element_format::_00_implied_float:
			return 4;

		case hd2::vertex_element_format::_01_implied_float2:
			return 8;

		case hd2::vertex_element_format::f32vec3:
			return 12;

		case hd2::vertex_element_format::_03_implied_float4:
			return 16;

		case hd2::vertex_element_format::_04_unknown:
			return 4;

		case hd2::vertex_element_format::_14_long4:
			return 16;

		case hd2::vertex_element_format::_18_byte4:
			return 4;

		case hd2::vertex_element_format::_19_unknown:
			return 4;

		case hd2::vertex_element_format::_1A_unknown:
			return 4;

		case hd2::vertex_element_format::f16vec2:
			return 4;

		case hd2::vertex_element_format::f16vec4:
			return 8;
		}
		throw std::runtime_error("UNKNOWN");
	}

	struct meshinfo_datatype_t {
		uint8_t const* _ptr;
		struct data_t {
			uint32_t magic_number;
			uint32_t __unk00;
			struct element_t {
				vertex_element_type   type;
				vertex_element_format format;
				uint32_t              layer;
				uint32_t              __unk00;
				uint32_t              __unk01;
			} elements[16];
			uint32_t num_elements;
			uint32_t __unk01;
			struct unk0_t {
				uint32_t magic_number;
				uint32_t __unk00[3];
				uint32_t vertex_count;
				uint32_t vertex_stride;
				uint32_t __unk01[4];
			} __unk02;
			struct unk1_t {
				uint32_t magic_number;
				uint32_t __unk09[3];
				uint32_t index_count;
				uint32_t __unk10[5];
				uint32_t vertex_offset;
				uint32_t vertex_size;
				uint32_t index_offset;
				uint32_t index_size;
				uint32_t __unk11[4];
			} __unk03;
		} const* _data;

		public:
		meshinfo_datatype_t(uint8_t const* ptr) : _ptr(ptr), _data(reinterpret_cast<decltype(_data)>(_ptr)) {}

		uint32_t elements()
		{
			return _data->num_elements;
		}

		data_t::element_t element(size_t idx)
		{
			auto edx = elements();
			if (idx >= edx) {
				std::out_of_range("idx >= edx");
			}

			return _data->elements[idx];
		}

		uint32_t vertices()
		{
			return _data->__unk02.vertex_count;
		}

		uint32_t vertices_offset()
		{
			return _data->__unk03.vertex_offset;
		}

		uint32_t vertices_size()
		{
			return _data->__unk03.vertex_size;
		}

		uint32_t vertices_stride()
		{
			//return (_data->vertex_size / _data->vertex_count);
			return _data->__unk02.vertex_stride;
		}

		uint32_t indices_offset()
		{
			return _data->__unk03.index_offset;
		}

		uint32_t indices()
		{
			return _data->__unk03.index_count;
		}

		uint32_t indices_size()
		{
			return _data->__unk03.index_size;
		}

		uint32_t indices_stride()
		{
			return (_data->__unk03.index_size / _data->__unk03.index_count);
		}

		data_t unique()
		{
			data_t udata = *_data;
			// Zero out non-unique information.
			udata.__unk02.vertex_count = udata.__unk02.vertex_stride = udata.__unk03.index_count = udata.__unk03.vertex_offset = udata.__unk03.vertex_size = udata.__unk03.index_offset = udata.__unk03.index_size = 0;

			return udata;
		}

		std::string hash()
		{
			data_t udata = unique();

#ifdef WIN32
			{
				BCRYPT_ALG_HANDLE  alghandle;
				BCRYPT_HASH_HANDLE hashhandle;
				ULONG              bytes_copied;
				DWORD              hashobj_length;
				DWORD              hash_length;

				if (auto status = BCryptOpenAlgorithmProvider(&alghandle, BCRYPT_MD5_ALGORITHM, NULL, 0); status) {
					throw std::runtime_error("BCryptOpenAlgorithmProvider");
				}
				bcrypt_alg_handle_t alg(alghandle);

				if (auto status = BCryptGetProperty(alg.get(), BCRYPT_OBJECT_LENGTH, (PUCHAR)&hashobj_length, sizeof(DWORD), &bytes_copied, 0); status) {
					throw std::runtime_error("BCryptGetProperty BCRYPT_OBJECT_LENGTH");
				}
				std::vector<char> hashobjbuf(hashobj_length, 0);

				if (auto status = BCryptGetProperty(alg.get(), BCRYPT_HASH_LENGTH, (PUCHAR)&hash_length, sizeof(DWORD), &bytes_copied, 0); status) {
					throw std::runtime_error("BCryptGetProperty BCRYPT_HASH_LENGTH");
				}
				std::vector<char> hashbuf(hash_length, 0);

				if (auto status = BCryptCreateHash(alg.get(), &hashhandle, (PUCHAR)hashobjbuf.data(), hashobjbuf.size(), NULL, 0, 0); status) {
					throw std::runtime_error("BCryptCreateHash");
				}
				bcrypt_hash_handle_t hash(hashhandle);

				if (auto status = BCryptHashData(hash.get(), (PBYTE)&udata, sizeof(hd2::meshinfo_datatype_t::data_t), 0); status) {
					throw std::runtime_error("BCryptHashData");
				}
				if (auto status = BCryptFinishHash(hash.get(), (PBYTE)hashbuf.data(), hashbuf.size(), 0); status) {
					throw std::runtime_error("BCryptFinishHash");
				}

				// There should now be a hash in hashbuf.
				std::stringstream sstr;
				for (size_t p = 0; p < hashbuf.size(); p++) {
					sstr << std::format("%02" PRIX8, static_cast<uint32_t>(hashbuf[p]));
				}

				return sstr.str();
			}
#else
#pragma error("Not yet implemented")
#endif
		}
	};

	struct meshinfo_datatype_list {
		uint8_t const* _ptr;
		struct data {
			uint32_t count;
			//uint32_t offsets[count];
			//uint32_t crcs[count];
			//uint32_t __unk00;
		} const* _data;

		public:
		meshinfo_datatype_list(uint8_t const* ptr) : _ptr(ptr), _data(reinterpret_cast<decltype(_data)>(_ptr)) {}

		uint32_t count() const
		{
			return _data->count;
		}

		uint32_t const* offsets() const
		{
			return reinterpret_cast<uint32_t const*>(_ptr + sizeof(data));
		}

		uint32_t const* not_crcs() const
		{
			return reinterpret_cast<uint32_t const*>(_ptr + sizeof(data) + sizeof(uint32_t) * count());
		}

		meshinfo_datatype_t at(size_t idx) const
		{
			auto edx = count();
			if (idx >= edx) {
				std::out_of_range("idx >= edx");
			}

			return {reinterpret_cast<uint8_t const*>(_ptr + offsets()[idx])};
		}
	};

	struct meshinfo_mesh_modeldata {
		uint8_t const* _ptr;
		struct {
			uint32_t vertices_offset;
			uint32_t vertices_count;
			uint32_t indices_offset;
			uint32_t indices_count;
		} const* _data;

		public:
		meshinfo_mesh_modeldata(uint8_t const* ptr) : _ptr(ptr), _data(reinterpret_cast<decltype(_data)>(_ptr)) {}

		uint32_t indices_offset() const
		{
			return _data->indices_offset;
		}

		uint32_t indices_count() const
		{
			return _data->indices_count;
		}

		uint32_t vertices_offset() const
		{
			return _data->vertices_offset;
		}

		uint32_t vertices_count() const
		{
			return _data->vertices_count;
		}
	};

	struct meshinfo_mesh {
		uint8_t const* _ptr;
		struct data {
			uint32_t __unk00;
			uint32_t __varies00[7];
			uint32_t __unk01;
			uint32_t __identicalToNotCRC;
			uint32_t __varies01[2];
			uint32_t __unk02;
			uint32_t __varies02;
			uint32_t datatype_index;
			uint32_t __unk03[10];
			uint32_t material_count2;
			uint32_t __unk04[3];
			uint32_t material_count;
			uint32_t modeldata_offset;
			//uint32_t materials[material_count];
		} const* _data;

		public:
		meshinfo_mesh(uint8_t const* ptr) : _ptr(ptr), _data(reinterpret_cast<decltype(_data)>(_ptr)) {}

		uint32_t datatype_index() const
		{
			return _data->datatype_index;
		}

		uint32_t material_count() const
		{
			return _data->material_count;
		}

		uint32_t const* materials() const
		{
			return reinterpret_cast<uint32_t const*>(_ptr + offsetof(data, modeldata_offset) + sizeof(uint32_t));
		}

		uint32_t material_at(size_t idx) const
		{
			auto edx = material_count();
			if (idx >= edx) {
				std::out_of_range("idx >= edx");
			}

			return materials()[idx];
		}

		meshinfo_mesh_modeldata modeldata() const
		{
			return {_ptr + _data->modeldata_offset};
		}
	};

	struct meshinfo_mesh_list {
		uint8_t const* _ptr;
		struct data {
			uint32_t count;
			//uint32_t offsets[count];
			//uint32_t crcs[count];
		} const* _data;

		public:
		meshinfo_mesh_list(uint8_t const* ptr) : _ptr(ptr), _data(reinterpret_cast<decltype(_data)>(_ptr)) {}

		uint32_t count() const
		{
			return _data->count;
		}

		uint32_t const* offsets() const
		{
			return reinterpret_cast<uint32_t const*>(_ptr + sizeof(data));
		}

		uint32_t const* not_crcs() const
		{
			return reinterpret_cast<uint32_t const*>(_ptr + sizeof(data) + sizeof(uint32_t) * count());
		}

		meshinfo_mesh at(size_t idx) const
		{
			auto edx = count();
			if (idx >= edx) {
				std::out_of_range("idx >= edx");
			}

			return {reinterpret_cast<uint8_t const*>(_ptr + sizeof(data) + offsets()[idx])};
		}
	};

	struct meshinfo_material_list {
		uint8_t const* _ptr;
		struct data {
			uint32_t count;
			//uint64_t keys[count];
			//uint64_t values[count];
		} const* _data;

		public:
		meshinfo_material_list(uint8_t const* ptr) : _ptr(ptr), _data(reinterpret_cast<decltype(_data)>(_ptr)) {}

		uint32_t count()
		{
			return _data->count;
		}

		uint32_t const* keys()
		{
			return reinterpret_cast<uint32_t const*>(_ptr + sizeof(data));
		}

		uint64_t const* values()
		{
			return reinterpret_cast<uint64_t const*>(_ptr + sizeof(data) + sizeof(uint32_t) * count());
		}

		std::pair<uint32_t, uint64_t> at(size_t idx)
		{
			auto edx = count();
			if (idx >= edx) {
				std::out_of_range("idx >= edx");
			}

			return {keys()[idx], values()[idx]};
		}
	};

	struct meshinfo {
		uint8_t const* _ptr;
		struct data {
			uint32_t __unk00[12];
			uint32_t __unk01;
			uint32_t __unk02;
			uint32_t __unk03;
			uint32_t __unk04;
			uint32_t __unk05[3];
			uint32_t __unk06;
			uint32_t __unk07;
			uint32_t __unk08;
			uint32_t __unk09;
			uint32_t datatype_offset;
			uint32_t __unk10;
			uint32_t mesh_offset;
			uint32_t __unk11[2];
			uint32_t material_offset;
		} const* _data;

		public:
		meshinfo(uint8_t const* mi) : _ptr(mi), _data(reinterpret_cast<decltype(_data)>(_ptr)) {}

		meshinfo_datatype_list datatypes()
		{
			return {_ptr + _data->datatype_offset};
		}

		meshinfo_mesh_list meshes()
		{
			return {_ptr + _data->mesh_offset};
		}

		meshinfo_material_list materials()
		{
			return {_ptr + _data->material_offset};
		}
	};
} // namespace hd2
;

int main(int argc, const char** argv)
{
	if (argc < 2) {
		return 1;
	}

	auto path          = std::filesystem::path{argv[1]};
	auto meshinfo_path = path.replace_extension(".meshinfo");
	auto mesh_path     = path.replace_extension(".mesh");
	auto output_path   = path.replace_extension();

#ifdef WIN32
	win32_handle_t mesh_info_file(CreateFileA(meshinfo_path.u8string().c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL));
	if (mesh_info_file.get() == INVALID_HANDLE_VALUE) {
		return 1;
	}

	win32_handle_t mesh_info_fmap(CreateFileMapping(mesh_info_file.get(), NULL, PAGE_READONLY, 0, 0, NULL));
	if (mesh_info_fmap.get() == INVALID_HANDLE_VALUE) {
		return 1;
	}

	uint8_t const* mesh_info_ptr = reinterpret_cast<uint8_t const*>(MapViewOfFile(mesh_info_fmap.get(), FILE_MAP_READ, 0, 0, 0));
	if (!mesh_info_ptr) {
		return 1;
	}

	win32_handle_t mesh_file(CreateFileA(mesh_path.u8string().c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL));
	if (mesh_file.get() == INVALID_HANDLE_VALUE) {
		return 1;
	}

	win32_handle_t mesh_fmap(CreateFileMapping(mesh_file.get(), NULL, PAGE_READONLY, 0, 0, NULL));
	if (mesh_fmap.get() == INVALID_HANDLE_VALUE) {
		return 1;
	}

	uint8_t const* mesh_ptr = reinterpret_cast<uint8_t const*>(MapViewOfFile(mesh_fmap.get(), FILE_MAP_READ, 0, 0, 0));
	if (!mesh_ptr) {
		return 1;
	}
#else
#pragma error("Not supported yet, do some work to make it work.")
#endif

	hd2::meshinfo meshinfo{reinterpret_cast<uint8_t const*>(mesh_info_ptr)};
	auto          materials = meshinfo.materials();
	auto          datatypes = meshinfo.datatypes();
	auto          meshes    = meshinfo.meshes();

	{ // Enumerate Materials
		printf("%" PRIu32 " Materials: \n", materials.count());
		for (size_t idx = 0; idx < materials.count(); idx++) {
			auto material = materials.at(idx);
			printf("- [%zu] %08" PRIx32 " = %" PRIx64 "\n", idx, material.first, material.second);
		}
	}

	{ // Enumerate Data Types
		printf("%" PRIu32 " Data Types: \n", datatypes.count());
		for (size_t idx = 0; idx < datatypes.count(); idx++) {
			auto datatype = datatypes.at(idx);
			printf("- [%zu] \n", idx);
			printf("  - %" PRIu32 " Vertices, ", datatype.vertices());
			printf("From: %08" PRIx32 ", ", datatype.vertices_offset());
			printf("To: %08" PRIx32 ", ", datatype.vertices_offset() + datatype.vertices_size());
			printf("Stride: %" PRId32 "\n", datatype.vertices_stride());
			printf("  - %" PRIu32 " Indices, ", datatype.indices());
			printf("From: %08" PRIx32 ", ", datatype.indices_offset());
			printf("To: %08" PRIx32 ", ", datatype.indices_offset() + datatype.indices_size());
			printf("Stride: %" PRId32 "\n", datatype.indices_stride());
		}
	}

	{ // Enumerate Meshes
		printf("%" PRIu32 " Meshes: \n", meshes.count());
		for (size_t idx = 0; idx < meshes.count(); idx++) {
			auto mesh     = meshes.at(idx);
			auto datatype = datatypes.at(mesh.datatype_index());
			printf("- [%zu] Data Type: %" PRIu32 ", %s\n", idx, mesh.datatype_index(), datatype.hash().c_str());

			printf("  - %" PRIu32 " Vertices, Stride: %" PRIu32 ", ", mesh.modeldata().vertices_count(), datatype.vertices_stride());
			printf("Offset: %08" PRIx32 ", ", mesh.modeldata().vertices_offset());
			printf("From: %08" PRIx32 ", ", mesh.modeldata().vertices_offset() + datatype.vertices_offset());
			printf("To: %08" PRIx32 "\n", mesh.modeldata().vertices_offset() + datatype.vertices_offset() + mesh.modeldata().vertices_count() * datatype.vertices_stride());

			printf("  - %" PRIu32 " Indices, Stride: %" PRIu32 ", ", mesh.modeldata().indices_count(), datatype.indices_stride());
			printf("Offset: %08" PRIx32 ", ", mesh.modeldata().indices_offset());
			printf("From: %08" PRIx32 ", ", mesh.modeldata().indices_offset() + datatype.indices_offset());
			printf("To: %08" PRIx32 "\n", mesh.modeldata().indices_offset() + datatype.indices_offset() + mesh.modeldata().indices_count() * datatype.indices_stride());

			printf("  - %" PRIu32 " Materials: ", mesh.material_count());
			for (size_t jdx = 0; jdx < mesh.material_count(); jdx++) {
				printf("%" PRIx32 ", ", mesh.material_at(jdx));
			}
			printf("\n");
		}
	}

	auto dt_path = std::filesystem::absolute(output_path.parent_path() / "../datatypes");
	{ // Export raw datatypes
		std::filesystem::create_directories(dt_path);

		for (size_t idx = 0; idx < datatypes.count(); idx++) {
			auto        datatype = datatypes.at(idx);
			std::string dt_hash  = datatype.hash();

			// Generate name.
			std::string filename = std::format("%s/%02" PRIu32 "_%s.dt", dt_path.generic_string().c_str(), datatype.vertices_stride(), dt_hash.c_str());

			if (!std::filesystem::exists(filename)) {
				auto   dtfilehandle = fopen(filename.c_str(), "w+b");
				file_t dtfile{dtfilehandle};
				fwrite(&datatype.unique(), 1, sizeof(hd2::meshinfo_datatype_t::data_t), dtfile.get());
			}
		}
	}

	{ // Export meshes.
		std::filesystem::create_directories(output_path);

		printf("Exporting Meshes...\n");
		for (size_t idx = 0; idx < meshes.count(); idx++) {
			printf("- [%zu] Exporting...\n", idx);
			auto mesh     = meshes.at(idx);
			auto datatype = datatypes.at(mesh.datatype_index());

			/*{ // Write sample file for datatype.
				std::string filename = std::format("%s/%02" PRIu32 "_%s.vtx", dt_path.generic_string().c_str(), datatype.vertices_stride(), datatype.hash().c_str());

				if (!std::filesystem::exists(filename)) {
					auto   dtfilehandle = fopen(filename.c_str(), "w+b");
					file_t dtfile{dtfilehandle};

					std::ptrdiff_t ptr = datatype.vertices_offset() + mesh.modeldata().vertices_offset() * datatype.vertices_stride();

					fwrite(reinterpret_cast<void const*>(mesh_ptr + ptr), datatype.vertices_stride(), std::min<size_t>(mesh.modeldata().vertices_count(), 128), dtfile.get());
				}
			}*/

			//* Don't do any of that for now.
			// Generate name.
			std::string filename;
			{
				const char*       format = "%s/%08" PRIu32 ".obj";
				std::vector<char> buf;
				size_t            bufsz = snprintf(nullptr, 0, format, output_path.u8string().c_str(), idx);
				buf.resize(bufsz + 1);
				snprintf(buf.data(), buf.size(), format, output_path.u8string().c_str(), idx);
				filename = {buf.data(), buf.data() + buf.size() - 1};
			}

			std::string vtxname;
			{
				const char*       format = "%s/%08" PRIu32 ".vtx";
				std::vector<char> buf;
				size_t            bufsz = snprintf(nullptr, 0, format, output_path.u8string().c_str(), idx);
				buf.resize(bufsz + 1);
				snprintf(buf.data(), buf.size(), format, output_path.u8string().c_str(), idx);
				vtxname = {buf.data(), buf.data() + buf.size() - 1};
			}

			std::string idxname;
			{
				const char*       format = "%s/%08" PRIu32 ".idx";
				std::vector<char> buf;
				size_t            bufsz = snprintf(nullptr, 0, format, output_path.u8string().c_str(), idx);
				buf.resize(bufsz + 1);
				snprintf(buf.data(), buf.size(), format, output_path.u8string().c_str(), idx);
				idxname = {buf.data(), buf.data() + buf.size() - 1};
			}

			// Open output files.
			auto   filehandle = fopen(filename.c_str(), "w+b");
			file_t file{filehandle};
			//auto   idxfilehandle = fopen(idxname.c_str(), "w+b");
			//file_t idxfile{idxfilehandle};
			//auto   vtxfilehandle = fopen(vtxname.c_str(), "w+b");
			//file_t vtxfile{vtxfilehandle};

			// Write overall data.
			fprintf(file.get(), "o %s\n", filename.c_str());
			fprintf(file.get(), "g %s\n", filename.c_str());
			fprintf(file.get(), "s 0\n");
			fflush(file.get());

			{ // Write all vertices.
				std::ptrdiff_t ptr         = datatype.vertices_offset() + mesh.modeldata().vertices_offset() * datatype.vertices_stride();
				std::ptrdiff_t max_ptr     = ptr + mesh.modeldata().vertices_count() * datatype.vertices_stride();
				std::ptrdiff_t abs_max_ptr = datatype.vertices_offset() + datatype.vertices_size();

				bool is_error_ptr = false;
				if (ptr > abs_max_ptr) {
					printf("  ptr > abs_max_ptr\n");
					is_error_ptr = true;
				} else if (max_ptr > abs_max_ptr) {
					printf("  max_ptr > abs_max_ptr\n");
					is_error_ptr = true;
				}
				if (is_error_ptr) {
					printf("  ptr =         %08zx", ptr);
					printf("  max_ptr =     %08zx", max_ptr);
					printf("  abs_max_ptr = %08zx", abs_max_ptr);
					continue;
				}

				//fwrite(reinterpret_cast<uint8_t const*>(mesh_ptr + ptr), 1, max_ptr - ptr, vtxfile.get());

				for (size_t vtx = 0; vtx < mesh.modeldata().vertices_count(); vtx++) {
					auto vtx_ptr = reinterpret_cast<uint8_t const*>(mesh_ptr + ptr + datatype.vertices_stride() * vtx);
					fprintf(file.get(), "# %zu\n", vtx);

					for (size_t el = 0; el < datatype.elements(); el++) {
						auto element = datatype.element(el);

						std::string format_str;
						switch (element.type) {
						case hd2::vertex_element_type::_00_position: {
							switch (element.format) {
							case hd2::vertex_element_format::f16vec2: {
								half const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
								fprintf(file.get(), "v %#16.8g %#16.8g %#16.8g\n", (float)vec_ptr[0] * hd2::mesh_scale, (float)vec_ptr[1] * hd2::mesh_scale, 0.f);
								break;
							}
							case hd2::vertex_element_format::_01_implied_float2: {
								float const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
								fprintf(file.get(), "v %#16.8g %#16.8g %#16.8g\n", vec_ptr[0] * hd2::mesh_scale, vec_ptr[1] * hd2::mesh_scale, 0.f);
								break;
							}

							case hd2::vertex_element_format::_1E_implied_half3: {
								half const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
								fprintf(file.get(), "v %#16.8g %#16.8g %#16.8g\n", (float)vec_ptr[0] * hd2::mesh_scale, (float)vec_ptr[1] * hd2::mesh_scale, (float)vec_ptr[2] * hd2::mesh_scale);
								break;
							}
							case hd2::vertex_element_format::f32vec3: {
								float const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
								fprintf(file.get(), "v %#16.8g %#16.8g %#16.8g\n", vec_ptr[0] * hd2::mesh_scale, vec_ptr[1] * hd2::mesh_scale, vec_ptr[2] * hd2::mesh_scale);
								break;
							}

							default:
								fprintf(file.get(), "# ERROR: type+format %08x, %08x, %08x, %08x, %08x is unknown\n", element.type, element.format, element.layer, element.__unk00, element.__unk01);
								fprintf(file.get(), "#  DUMP: ");
								for (size_t n = 0; n < hd2::vertex_element_format_size(element.format); n++) {
									fprintf(file.get(), "%02X", *(vtx_ptr + n));
								}
								fprintf(file.get(), "\n");
							}
							break;
						}

						case hd2::vertex_element_type::_04_texcoord: {
							if (element.layer != 0) {
								fprintf(file.get(), "# ");
							}

							switch (element.format) {
							case hd2::vertex_element_format::f16vec2: {
								half const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
								fprintf(file.get(), "vt %#16.8g %#16.8g\n", (float)vec_ptr[0], (float)vec_ptr[1]);
								break;
							}
							case hd2::vertex_element_format::_01_implied_float2: {
								float const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
								fprintf(file.get(), "vt %#16.8g %#16.8g\n", (float)vec_ptr[0], (float)vec_ptr[1]);
								break;
							}

							case hd2::vertex_element_format::_1E_implied_half3: {
								half const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
								fprintf(file.get(), "vt %#16.8g %#16.8g %#16.8g\n", (float)vec_ptr[0], (float)vec_ptr[1], (float)vec_ptr[2]);
								break;
							}
							case hd2::vertex_element_format::f32vec3: {
								float const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
								fprintf(file.get(), "vt %#16.8g %#16.8g %#16.8g\n", (float)vec_ptr[0], (float)vec_ptr[1], (float)vec_ptr[2]);
								break;
							}

							default:
								fprintf(file.get(), "# ERROR: type+format %08x, %08x, %08x, %08x, %08x is unknown\n", element.type, element.format, element.layer, element.__unk00, element.__unk01);
								fprintf(file.get(), "#  DUMP: ");
								for (size_t n = 0; n < hd2::vertex_element_format_size(element.format); n++) {
									fprintf(file.get(), "%02X", *(vtx_ptr + n));
								}
								fprintf(file.get(), "\n");
							}
							break;
						}

						case hd2::vertex_element_type::_07_normal: {
							if (element.layer != 0) {
								fprintf(file.get(), "# ");
							}

							switch (element.format) {
							case hd2::vertex_element_format::_1E_implied_half3: {
								half const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
								fprintf(file.get(), "vn %#16.8g %#16.8g %#16.8g\n", (float)vec_ptr[0], (float)vec_ptr[1], (float)vec_ptr[2]);
								break;
							}
							case hd2::vertex_element_format::f32vec3: {
								float const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
								fprintf(file.get(), "vn %#16.8g %#16.8g %#16.8g\n", (float)vec_ptr[0], (float)vec_ptr[1], (float)vec_ptr[2]);
								break;
							}

							default:
								fprintf(file.get(), "# ERROR: type+format %08x, %08x, %08x, %08x, %08x is unknown\n", element.type, element.format, element.layer, element.__unk00, element.__unk01);
								fprintf(file.get(), "#  DUMP: ");
								for (size_t n = 0; n < hd2::vertex_element_format_size(element.format); n++) {
									fprintf(file.get(), "%02X", *(vtx_ptr + n));
								}
								fprintf(file.get(), "\n");
							}
							break;
						}

						default:
							fprintf(file.get(), "# ERROR: type %08x, %08x, %08x, %08x, %08x is unknown\n", element.type, element.format, element.layer, element.__unk00, element.__unk01);
							fprintf(file.get(), "#  DUMP: ");
							for (size_t n = 0; n < hd2::vertex_element_format_size(element.format); n++) {
								fprintf(file.get(), "%02X", *(vtx_ptr + n));
							}
							fprintf(file.get(), "\n");
						}

						vtx_ptr += hd2::vertex_element_format_size(element.format);
					}
				}
			}
			fflush(file.get());

			{ // Write all faces.
				std::ptrdiff_t ptr         = datatype.indices_offset() + mesh.modeldata().indices_offset() * datatype.indices_stride();
				std::ptrdiff_t max_ptr     = ptr + mesh.modeldata().indices_count() * datatype.indices_stride();
				std::ptrdiff_t abs_max_ptr = datatype.indices_offset() + datatype.indices_size();

				bool is_error_ptr = false;
				if (ptr > abs_max_ptr) {
					printf("  ptr > abs_max_ptr\n");
					is_error_ptr = true;
				} else if (max_ptr > abs_max_ptr) {
					printf("  max_ptr > abs_max_ptr\n");
					is_error_ptr = true;
				}
				if (is_error_ptr) {
					printf("  ptr =         %08zx", ptr);
					printf("  max_ptr =     %08zx", max_ptr);
					printf("  abs_max_ptr = %08zx", abs_max_ptr);
					continue;
				}

				//fwrite(reinterpret_cast<uint8_t const*>(mesh_ptr + ptr), 1, max_ptr - ptr, idxfile.get());

				for (size_t idx = 0; idx < mesh.modeldata().indices_count(); idx += 3) {
					auto idx_ptr = reinterpret_cast<uint8_t const*>(mesh_ptr + ptr + datatype.indices_stride() * idx);
					fprintf(file.get(), "# %zu\n", idx);

					switch (datatype.indices_stride()) {
					case 1: {
						auto v0 = *reinterpret_cast<uint8_t const*>(idx_ptr + datatype.indices_stride() * 0) + 1;
						auto v1 = *reinterpret_cast<uint8_t const*>(idx_ptr + datatype.indices_stride() * 1) + 1;
						auto v2 = *reinterpret_cast<uint8_t const*>(idx_ptr + datatype.indices_stride() * 2) + 1;
						fprintf(file.get(), "f %" PRIu8 "/%" PRIu8 "/%" PRIu8 " %" PRIu8 "/%" PRIu8 "/%" PRIu8 " %" PRIu8 "/%" PRIu8 "/%" PRIu8 "\n", v0, v0, v0, v1, v1, v1, v2, v2, v2);
						break;
					}
					case 2: {
						auto v0 = *reinterpret_cast<uint16_t const*>(idx_ptr + datatype.indices_stride() * 0) + 1;
						auto v1 = *reinterpret_cast<uint16_t const*>(idx_ptr + datatype.indices_stride() * 1) + 1;
						auto v2 = *reinterpret_cast<uint16_t const*>(idx_ptr + datatype.indices_stride() * 2) + 1;
						fprintf(file.get(), "f %" PRIu16 "/%" PRIu16 "/%" PRIu16 " %" PRIu16 "/%" PRIu16 "/%" PRIu16 " %" PRIu16 "/%" PRIu16 "/%" PRIu16 "\n", v0, v0, v0, v1, v1, v1, v2, v2, v2);
						break;
					}
					case 4: {
						auto v0 = *reinterpret_cast<uint32_t const*>(idx_ptr + datatype.indices_stride() * 0) + 1;
						auto v1 = *reinterpret_cast<uint32_t const*>(idx_ptr + datatype.indices_stride() * 1) + 1;
						auto v2 = *reinterpret_cast<uint32_t const*>(idx_ptr + datatype.indices_stride() * 2) + 1;
						fprintf(file.get(), "f %" PRIu32 "/%" PRIu32 "/%" PRIu32 " %" PRIu32 "/%" PRIu32 "/%" PRIu32 " %" PRIu32 "/%" PRIu32 "/%" PRIu32 "\n", v0, v0, v0, v1, v1, v1, v2, v2, v2);
						break;
					}
					case 8: {
						auto v0 = *reinterpret_cast<uint64_t const*>(idx_ptr + datatype.indices_stride() * 0) + 1;
						auto v1 = *reinterpret_cast<uint64_t const*>(idx_ptr + datatype.indices_stride() * 1) + 1;
						auto v2 = *reinterpret_cast<uint64_t const*>(idx_ptr + datatype.indices_stride() * 2) + 1;
						fprintf(file.get(), "f %" PRIu64 "/%" PRIu64 "/%" PRIu64 " %" PRIu64 "/%" PRIu64 "/%" PRIu64 " %" PRIu64 "/%" PRIu64 "/%" PRIu64 "\n", v0, v0, v0, v1, v1, v1, v2, v2, v2);
						break;
					}
					}
				}
			}
			fflush(file.get());
			//*/
		}
	}

	std::cout << std::endl;

	return 0;
}
