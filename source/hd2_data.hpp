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
#include <string_view>

#include "mapped_file.hpp"

namespace hd2 {
	enum class file_type : uint64_t {
		TRUETYPE_FONT               = 0x05106B81DCD58A13, // TrueType Font (.ttf) // 0x80 or 0x24 "Header" Size
		BONES                       = 0x18DEAD01056B72E9, // Had clear text that looked like a skeleton bone tree
		_HAVOK_1D59BD6687DB6B33     = 0x1D59BD6687DB6B33, // References Havok content.
		SHADING_ENVIRONMENT_MAPPING = 0x250E0A11AC8E26F8, // Shading Environment Mapping (.shading_environment_mapping)
		RENDER_CONFIG               = 0x27862FE24795319C, // Render Config (.render_config)
		LEVEL                       = 0x2A690FD348FE9AC5, // Level (.level)
		WWISE_STREAM                = 0x504B55235D21440E, // Wwise Encoded Media (.wwise_stream)
		WWISE_BANK                  = 0x535A7BD3E650D799, // Wwise Sound Bank (.wwise_bank), 0x16 "Header" Size
		HAVOK_57A13425279979D7      = 0x57A13425279979D7, // References Havok content.
		HAVOK_5F7203C8F280DAB8      = 0x5F7203C8F280DAB8, // References Havok content.
		_9199BB50B6896F02           = 0x9199BB50B6896F02, //
		ENTITY                      = 0x9831CA893B0D087D, // Entity (.entity)
		ANIMATION                   = 0x931E336D7646CC26, // Animation (.animation)
		FONT                        = 0x9EFE0A916AAE7880, // Font (.font)
		LUA                         = 0xA14E8DFA2CD117E2, // Lua (.lua), References Wwise Sound Banks
		STATE_MACHINE               = 0xA486D4045106165C, // State Machine (.state_machine)
		PARTICLES                   = 0xA8193123526FAD64, // Particles (.particles)
		BINK_VIDEO                  = 0xAA5965F03029FA18, // Bink Video (.bik)
		PREFAB                      = 0xAB2F78E885F513C6, // .prefab
		WWISDE_DEPENDENCIES         = 0xAF32095C82F2B070, // Wwise Dependencies (.wwise_dep), Contains text referencing audio
		_C4F0F4BE7FB0C8D6           = 0xC4F0F4BE7FB0C8D6, //
		TEXTURE                     = 0xCD4238C6A0C69E32, // Texture (.texture), (DDS = 0xC0 "Header" Size)
		CLOTH                       = 0xD7014A50477953E0, // Cloth (.cloth)
		UNIT                        = 0xE0A48D0BE9A7453F, // Unit (.unit), MeshInfo+Mesh
		SPEEDTREE                   = 0xE985C5F61C169997, // SpeedTree (.speedtree)
		MATERIAL                    = 0xEAC0B497876ADEDF, // Material (.material)
		VECTOR_FIELD                = 0xF7505933166D6755, // Vector Field (.vector_field)
		SHADING_ENVIRONMENT         = 0xFE73C7DCFF8A7CA5, // Shading Environement (.shading_environment)

		// From https://reshax.com/topic/507-helldivers-2-model-extraction-help/?do=findComment&comment=2009&_rid=1957
		VT2_ac2b738a374cf583 = 0xac2b738a374cf583, // actor
		//VT2_931e336d7646cc26 = 0x931e336d7646cc26, // animation
		VT2_dcfb9e18fff13984 = 0xdcfb9e18fff13984, // animation_curves
		VT2_44957ef5a3a368e4 = 0x44957ef5a3a368e4, // animation_set
		VT2_3eed05ba83af5090 = 0x3eed05ba83af5090, // apb
		VT2_7ffdb779b04e4ed1 = 0x7ffdb779b04e4ed1, // baked_lighting
		//VT2_aa5965f03029fa18 = 0xaa5965f03029fa18, // bik
		VT2_e301e8af94e3b5a3 = 0xe301e8af94e3b5a3, // blend_set
		//VT2_18dead01056b72e9 = 0x18dead01056b72e9, // bones
		VT2_b7893adf7567506a = 0xb7893adf7567506a, // chroma
		VT2_f7b361bde8b4dd03 = 0xf7b361bde8b4dd03, // common
		VT2_fe9754bd19814a47 = 0xfe9754bd19814a47, // common_package
		VT2_0001a6d201e87086 = 0x0001a6d201e87086, // compiled_shader
		VT2_82645835e6b73232 = 0x82645835e6b73232, // config
		VT2_69108ded1e3e634b = 0x69108ded1e3e634b, // crypto
		VT2_8fd0d44d20650b68 = 0x8fd0d44d20650b68, // data
		//VT2_64298613551f58d4 = 0x64298613551f58d4, // dds
		VT2_ed74b77c6d447d84 = 0xed74b77c6d447d84, // disabled
		//VT2_9831ca893b0d087d = 0x9831ca893b0d087d, // entity
		VT2_92d3ee038eeb610d = 0x92d3ee038eeb610d, // flow
		VT2_6a00deed7d3b235a = 0x6a00deed7d3b235a, // flow_editor
		//VT2_9efe0a916aae7880 = 0x9efe0a916aae7880, // font
		VT2_d526a27da14f1dc5 = 0xd526a27da14f1dc5, // ini
		VT2_fa4a8e091a91201e = 0xfa4a8e091a91201e, // ivf
		VT2_a62f9297dc969e85 = 0xa62f9297dc969e85, // keys
		//VT2_2a690fd348fe9ac5 = 0x2a690fd348fe9ac5, // level
		//VT2_a14e8dfa2cd117e2 = 0xa14e8dfa2cd117e2, // lua
		//VT2_eac0b497876adedf = 0xeac0b497876adedf, // material
		VT2_48ff313713a997a1 = 0x48ff313713a997a1, // mesh
		VT2_3fcdd69156a46417 = 0x3fcdd69156a46417, // mod
		VT2_b277b11fe4a61d37 = 0xb277b11fe4a61d37, // mouse_cursor
		VT2_169de9566953d264 = 0x169de9566953d264, // navdata
		VT2_3b1fa9e8f6bac374 = 0x3b1fa9e8f6bac374, // network_config
		VT2_ad9c6d9ed1e5e77a = 0xad9c6d9ed1e5e77a, // package
		//VT2_a8193123526fad64 = 0xa8193123526fad64, // particles
		VT2_5f7203c8f280dab8 = 0x5f7203c8f280dab8, // physics
		VT2_22bd082459e868d8 = 0x22bd082459e868d8, // physics_data
		VT2_bf21403a3ab0bbb1 = 0xbf21403a3ab0bbb1, // physics_properties
		VT2_46cab355bd8d4d37 = 0x46cab355bd8d4d37, // prototype
		//VT2_27862fe24795319c = 0x27862fe24795319c, // render_config
		VT2_d37e11a77baafc01 = 0xd37e11a77baafc01, // resource
		VT2_9d0a795bfe818d19 = 0x9d0a795bfe818d19, // scene
		VT2_cce8d5b5f5ae333f = 0xcce8d5b5f5ae333f, // shader
		VT2_e5ee32a477239a93 = 0xe5ee32a477239a93, // shader_library
		VT2_9e5c3cc74575aeb5 = 0x9e5c3cc74575aeb5, // shader_library_group
		//VT2_fe73c7dcff8a7ca5 = 0xfe73c7dcff8a7ca5, // shading_environment
		//VT2_250e0a11ac8e26f8 = 0x250e0a11ac8e26f8, // shading_environment_mapping
		VT2_a27b4d04a9ba6f9e = 0xa27b4d04a9ba6f9e, // slug
		VT2_e9fc9ea7042e5ec0 = 0xe9fc9ea7042e5ec0, // slug_album
		VT2_90641b51c98b7aac = 0x90641b51c98b7aac, // sound
		VT2_d8b27864a97ffdd7 = 0xd8b27864a97ffdd7, // sound_environment
		VT2_f97af9983c05b950 = 0xf97af9983c05b950, // spu_job
		//VT2_a486d4045106165c = 0xa486d4045106165c, // state_machine
		VT2_bb0279e548747a0a = 0xbb0279e548747a0a, // statemachine_editor
		VT2_0d972bab10b40fd3 = 0x0d972bab10b40fd3, // strings
		VT2_ad2d3fa30d9ab394 = 0xad2d3fa30d9ab394, // surface_properties
		//VT2_cd4238c6a0c69e32 = 0xcd4238c6a0c69e32, // texture
		VT2_91f98cecac930de8 = 0x91f98cecac930de8, // timestamp
		VT2_94616b1760e1c463 = 0x94616b1760e1c463, // timpani
		VT2_99736be1fff739a4 = 0x99736be1fff739a4, // timpani_bank
		VT2_00a3e6c59a2b9c6c = 0x00a3e6c59a2b9c6c, // timpani_master
		VT2_19c792357c99f49b = 0x19c792357c99f49b, // tome
		VT2_e0a48d0be9a7453f = 0xe0a48d0be9a7453f, // unit
		VT2_8c074c8f95e7841b = 0x8c074c8f95e7841b, // user_settings
		//VT2_f7505933166d6755 = 0xf7505933166d6755, // vector_field
		VT2_6ad9817aa72d9533 = 0x6ad9817aa72d9533, // volume
		VT2_786f65c00a816b19 = 0x786f65c00a816b19, // wav
		//VT2_535a7bd3e650d799 = 0x535a7bd3e650d799, // wwise_bank
		//VT2_af32095c82f2b070 = 0xaf32095c82f2b070, // wwise_dep
		//VT2_d50a8b7e1c82b110 = 0xd50a8b7e1c82b110, // wwise_metadata
		//VT2_504b55235d21440e = 0x504b55235d21440e, // wwise_stream
		VT2_76015845a6003765 = 0x76015845a6003765, // xml
		VT2_712d6e3dd1024c9c = 0x712d6e3dd1024c9c, // ugg
		VT2_a99510c6e86dd3c2 = 0xa99510c6e86dd3c2, // upb
		VT2_2bbcabe5074ade9e = 0x2bbcabe5074ade9e, // input
		VT2_52014e20a7fdccef = 0x52014e20a7fdccef, // xp
		VT2_7b82d55316f98c9b = 0x7b82d55316f98c9b, // vista
		VT2_de8c9d00247f8fc6 = 0xde8c9d00247f8fc6, // w7
		VT2_0605775c0481181b = 0x0605775c0481181b, // w8
		VT2_293bb3b1982c6d5a = 0x293bb3b1982c6d5a, // main
		VT2_b6b3099f78886ff9 = 0xb6b3099f78886ff9, // win32
		VT2_d596cd750e4e279f = 0xd596cd750e4e279f, // zD
	};

	std::string_view file_extension_from_type(file_type type);

	/** TTF Font
	 * 0x00 - 0x08: Own Name
	 * 0x08 - 0x18: No matches
	 * 0x18 - 0x20: Another font name?
	 */

	typedef uint64_t file_id_t;

	class data {
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
			uint32_t  __unk00;
			uint32_t  __unk01;
			file_type type; // Big Endian
			uint32_t  count;
			uint32_t  __unk02;
			uint32_t  __unk03_always_0x10;
			uint32_t  __unk04_always_0x40;
		} const* _ptr_type;

		struct file_t {
			uint64_t  id; // Big Endian
			file_type type; // Big Endian
			uint32_t  offset;
			uint32_t  __unk00;
			uint32_t  stream_offset;
			uint32_t  __unk01;
			uint32_t  gpu_offset;
			uint32_t  __unk02;
			uint32_t  __unk03;
			uint32_t  __unk04;
			uint32_t  __unk05;
			uint32_t  __unk06;
			uint32_t  size;
			uint32_t  stream_size;
			uint32_t  gpu_size;
			uint32_t  __unk07;
			uint32_t  __unk08;
			uint32_t  index;
		} const* _ptr_file;

		uint8_t const* _ptr_data;

		// header_t header;
		// type_t type[header.types];
		// file_t file[header.files];
		// struct { char data[file.size] } data[header.files];

		std::filesystem::path _meta_path;
		mapped_file           _meta;
		size_t                _meta_size;

		std::filesystem::path      _stream_path;
		std::optional<mapped_file> _stream;
		size_t                     _stream_size;

		std::filesystem::path      _gpu_path;
		std::optional<mapped_file> _gpu;
		size_t                     _gpu_size;

		public:
		data(std::filesystem::path path);

		size_t types() const;

		type_t const& type(size_t idx) const;

		size_t files() const;

		file_t const& file(size_t idx) const;

		bool has_meta(size_t idx) const;

		uint8_t const* meta_data(size_t idx) const;

		size_t meta_size(size_t idx) const;

		bool has_stream(size_t idx) const;

		uint8_t const* stream(size_t idx) const;

		size_t stream_size(size_t idx) const;

		bool has_gpu(size_t idx) const;

		uint8_t const* gpu(size_t idx) const;

		size_t gpu_size(size_t idx) const;
	};
} // namespace hd2

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
