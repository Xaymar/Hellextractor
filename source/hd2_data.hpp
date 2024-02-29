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
#include <filesystem>
#include <optional>
#include <string_view>

#include "mapped_file.hpp"

namespace helldivers2 {
	typedef uint64_t file_id_t;
	typedef uint64_t file_type_t;

	class data {
		// header_t header;
		// type_t type[header.types];
		// file_t file[header.files];
		// struct { char data[file.size] } data[header.files];

		std::filesystem::path _main_path;
		mapped_file           _main;
		size_t                _main_size;

		std::filesystem::path      _stream_path;
		std::optional<mapped_file> _stream;
		size_t                     _stream_size;

		std::filesystem::path      _gpu_path;
		std::optional<mapped_file> _gpu;
		size_t                     _gpu_size;

		public:
		struct header_t {
			uint32_t magic_number; // Should always be 0x110000F0
			uint32_t types;
			uint32_t files;

			uint32_t __unk00;
			uint32_t __unk01; // Big Endian, seems to stay the same if the data file contains the same files.
			uint32_t __unk02;
			uint32_t __unk03;
			uint32_t __unk04;
			uint32_t __unk05;
			uint32_t __unk06;
			uint32_t gpu_resources_size_aligned_to_256b; // Size of _gpu aligned to the next higher 256b boundary.

			uint32_t __unk07;
			uint32_t __unk08;
			uint32_t __unk09;
			uint32_t __unk0A;
			uint32_t __unk0B;
			uint32_t __unk0C;
			uint32_t __unk0D;
		} const* _ptr;

		struct type_t {
			uint32_t    __unk00;
			uint32_t    __unk01;
			file_type_t id; // Big Endian
			uint32_t    count;
			uint32_t    __unk02;
			uint32_t    __unk03_always_0x10;
			uint32_t    __unk04_always_0x40;
		} const* _ptr_type;

		struct file_t {
			file_id_t   id; // Big Endian
			file_type_t type; // Big Endian
			uint32_t    offset;
			uint32_t    __unk00;
			uint32_t    stream_offset;
			uint32_t    __unk01;
			uint32_t    gpu_offset;
			uint32_t    __unk02;
			uint32_t    __unk03;
			uint32_t    __unk04;
			uint32_t    __unk05;
			uint32_t    __unk06;
			uint32_t    size;
			uint32_t    stream_size;
			uint32_t    gpu_size;
			uint32_t    __unk07;
			uint32_t    __unk08;
			uint32_t    index;
		} const* _ptr_file;

		uint8_t const* _ptr_data;

		struct meta_t {
			file_t file;

			size_t      main_size;
			void const* main;
			size_t      stream_size;
			void const* stream;
			size_t      gpu_size;
			void const* gpu;
		};

		public:
		data(std::filesystem::path path);

		size_t types() const;

		type_t const& type(size_t idx) const;

		size_t files() const;

		file_t const& file(size_t idx) const;

		meta_t meta(size_t idx) const;

		bool has_main(size_t idx) const;

		uint8_t const* main_data(size_t idx) const;

		size_t main_size(size_t idx) const;

		bool has_stream(size_t idx) const;

		uint8_t const* stream_data(size_t idx) const;

		size_t stream_size(size_t idx) const;

		bool has_gpu(size_t idx) const;

		uint8_t const* gpu_data(size_t idx) const;

		size_t gpu_size(size_t idx) const;
	};
} // namespace helldivers2

/*
namespace hd2 {

	// Models are very tiny, so this can be used to scale them up.
	constexpr float mesh_scale = 1.;

	enum class vertex_element_type : uint32_t {
		_00_position = 0x00, // format = <float_type>[2..4], rest seems to be ignored
		_01_color    = 0x01, // format = 0x1A
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
				throw std::out_of_range("idx >= edx");
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
		}
	};

	struct meshinfo_datatype_list_t {
		uint8_t const* _ptr;
		struct data {
			uint32_t count;
			//uint32_t offsets[count];
			//uint32_t crcs[count];
			//uint32_t __unk00;
		} const* _data;

		public:
		meshinfo_datatype_list_t(uint8_t const* ptr) : _ptr(ptr), _data(reinterpret_cast<decltype(_data)>(_ptr)) {}

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
				throw std::out_of_range("idx >= edx");
			}

			return {reinterpret_cast<uint8_t const*>(_ptr + offsets()[idx])};
		}
	};

	struct meshinfo_mesh_modeldata_t {
		uint8_t const* _ptr;
		struct data_t {
			uint32_t vertices_offset;
			uint32_t vertices;
			uint32_t indices_offset;
			uint32_t indices;
		} const* _data;

		public:
		meshinfo_mesh_modeldata_t(uint8_t const* ptr) : _ptr(ptr), _data(reinterpret_cast<decltype(_data)>(_ptr)) {}

		void const* operator&() const
		{
			return _ptr;
		}

		size_t size() const
		{
			return sizeof(data_t);
		}

		uint32_t indices_offset() const
		{
			return _data->indices_offset;
		}

		uint32_t indices() const
		{
			return _data->indices;
		}

		uint32_t vertices_offset() const
		{
			return _data->vertices_offset;
		}

		uint32_t vertices() const
		{
			return _data->vertices;
		}
	};

	struct meshinfo_mesh_t {
		uint8_t const* _ptr;
		struct data_t {
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
		meshinfo_mesh_t(uint8_t const* ptr) : _ptr(ptr), _data(reinterpret_cast<decltype(_data)>(_ptr)) {}

		void const* operator&() const
		{
			return _ptr;
		}

		size_t size() const
		{
			return _data->modeldata_offset + sizeof(uint32_t) * _data->material_count + sizeof(meshinfo_mesh_modeldata_t);
		}

		uint32_t datatype_index() const
		{
			return _data->datatype_index;
		}

		uint32_t materials() const
		{
			return _data->material_count;
		}

		uint32_t const* raw_materials() const
		{
			return reinterpret_cast<uint32_t const*>(_ptr + offsetof(data_t, modeldata_offset) + sizeof(uint32_t));
		}

		uint32_t material(size_t idx) const
		{
			auto edx = materials();
			if (idx >= edx) {
				throw std::out_of_range("idx >= edx");
			}

			return raw_materials()[idx];
		}

		meshinfo_mesh_modeldata_t modeldata() const
		{
			return {_ptr + _data->modeldata_offset};
		}
	};

	struct meshinfo_mesh_list_t {
		uint8_t const* _ptr;
		struct data_t {
			uint32_t count;
			//uint32_t offsets[count];
			//uint32_t crcs[count];
		} const* _data;

		public:
		meshinfo_mesh_list_t(uint8_t const* ptr) : _ptr(ptr), _data(reinterpret_cast<decltype(_data)>(_ptr)) {}

		uint32_t count() const
		{
			return _data->count;
		}

		uint32_t const* offsets() const
		{
			return reinterpret_cast<uint32_t const*>(_ptr + sizeof(data_t));
		}

		uint32_t const* not_crcs() const
		{
			return reinterpret_cast<uint32_t const*>(_ptr + sizeof(data_t) + sizeof(uint32_t) * count());
		}

		meshinfo_mesh_t at(size_t idx) const
		{
			auto edx = count();
			if (idx >= edx) {
				throw std::out_of_range("idx >= edx");
			}

			return {reinterpret_cast<uint8_t const*>(_ptr + sizeof(data_t) + offsets()[idx])};
		}
	};

	struct meshinfo_material_list_t {
		uint8_t const* _ptr;
		struct data_t {
			uint32_t count;
			//uint32_t keys[count];
			//uint64_t values[count];
		} const* _data;

		public:
		meshinfo_material_list_t(uint8_t const* ptr) : _ptr(ptr), _data(reinterpret_cast<decltype(_data)>(_ptr)) {}

		void const* operator&() const
		{
			return _ptr;
		}

		size_t size() const
		{
			return sizeof(data_t) + sizeof(uint32_t);
		}

		uint32_t count()
		{
			return _data->count;
		}

		uint32_t const* keys()
		{
			return reinterpret_cast<uint32_t const*>(_ptr + sizeof(data_t));
		}

		uint64_t const* values()
		{
			return reinterpret_cast<uint64_t const*>(_ptr + sizeof(data_t) + sizeof(uint32_t) * count());
		}

		std::pair<uint32_t, uint64_t> at(size_t idx)
		{
			auto edx = count();
			if (idx >= edx) {
				throw std::out_of_range("idx >= edx");
			}

			return {keys()[idx], values()[idx]};
		}
	};

	struct meshinfo_t {
		uint8_t const* _ptr;
		struct data_t {
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
		meshinfo_t(uint8_t const* mi) : _ptr(mi), _data(reinterpret_cast<decltype(_data)>(_ptr)) {}

		void const* operator&() const
		{
			return _ptr;
		}

		size_t size() const
		{
			return sizeof(data_t);
		}

		meshinfo_datatype_list_t datatypes()
		{
			return {_ptr + _data->datatype_offset};
		}

		meshinfo_mesh_list_t meshes()
		{
			return {_ptr + _data->mesh_offset};
		}

		meshinfo_material_list_t materials()
		{
			return {_ptr + _data->material_offset};
		}
	};

} // namespace hd2
*/
