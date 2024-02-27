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

std::string_view hd2::file_extension_from_type(hd2::file_type type)
{
	switch (type) {
	case hd2::file_type::TRUETYPE_FONT:
		return ".ttf";
	case hd2::file_type::BONES:
		return ".bones";
	//case hd2::file_type::_HAVOK_1D59BD6687DB6B33:
	//	return " References Havok content.";
	case hd2::file_type::SHADING_ENVIRONMENT_MAPPING:
		return ".shading_environment_mapping";
	case hd2::file_type::RENDER_CONFIG:
		return ".render_config";
	case hd2::file_type::LEVEL:
		return ".level";
	case hd2::file_type::WWISE_STREAM:
		return ".wwise_stream";
	case hd2::file_type::WWISE_BANK:
		return ".wwise_bank";
	//case hd2::file_type::HAVOK_57A13425279979D7:
	//	return " References Havok content.";
	//case hd2::file_type::HAVOK_5F7203C8F280DAB8:
	//	return " References Havok content.";
	//case hd2::file_type::_9199BB50B6896F02:
	//	return "";
	case hd2::file_type::ENTITY:
		return ".entity";
	case hd2::file_type::ANIMATION:
		return ".animation";
	case hd2::file_type::FONT:
		return ".font";
	case hd2::file_type::LUA:
		return ".lua";
	case hd2::file_type::STATE_MACHINE:
		return ".state_machine";
	case hd2::file_type::PARTICLES:
		return ".particles";
	case hd2::file_type::BINK_VIDEO:
		return ".bik";
	//case hd2::file_type::_AB2F78E885F513C6:
	//	return "";
	case hd2::file_type::WWISDE_DEPENDENCIES:
		return ".wwise_dep";
	//case hd2::file_type::_C4F0F4BE7FB0C8D6:
	//	return "";
	case hd2::file_type::TEXTURE:
		return ".texture";
	//case hd2::file_type::_HAVOK_CLOTH:
	//	return " References Havok content. Cloth Physics for capes?";
	case hd2::file_type::UNIT:
		return ".unit";
	//case hd2::file_type::SPEEDTREE_E985C5F61C169997:
	//	return " SpeedTree SDK format, possibly trees and foliage.";
	case hd2::file_type::MATERIAL:
		return ".material";
	case hd2::file_type::VECTOR_FIELD:
		return ".vector_field";
	case hd2::file_type::SHADING_ENVIRONMENT:
		return ".shading_environment";

	// From https://reshax.com/topic/507-helldivers-2-model-extraction-help/?do=findComment&comment=2009&_rid=1957
	case hd2::file_type::VT2_ac2b738a374cf583:
		return ".actor";
	//case hd2::file_type::VT2_931e336d7646cc26: return " animation";
	case hd2::file_type::VT2_dcfb9e18fff13984:
		return ".animation_curves";
	case hd2::file_type::VT2_44957ef5a3a368e4:
		return ".animation_set";
	case hd2::file_type::VT2_3eed05ba83af5090:
		return ".apb";
	case hd2::file_type::VT2_7ffdb779b04e4ed1:
		return ".baked_lighting";
	//case hd2::file_type::VT2_aa5965f03029fa18: return " bik";
	case hd2::file_type::VT2_e301e8af94e3b5a3:
		return ".blend_set";
	//case hd2::file_type::VT2_18dead01056b72e9: return " bones";
	case hd2::file_type::VT2_b7893adf7567506a:
		return ".chroma";
	case hd2::file_type::VT2_f7b361bde8b4dd03:
		return ".common";
	case hd2::file_type::VT2_fe9754bd19814a47:
		return ".common_package";
	case hd2::file_type::VT2_0001a6d201e87086:
		return ".compiled_shader";
	case hd2::file_type::VT2_82645835e6b73232:
		return ".config";
	case hd2::file_type::VT2_69108ded1e3e634b:
		return ".crypto";
	case hd2::file_type::VT2_8fd0d44d20650b68:
		return ".data";
	//case hd2::file_type::VT2_64298613551f58d4: return " dds";
	case hd2::file_type::VT2_ed74b77c6d447d84:
		return ".disabled";
	//case hd2::file_type::VT2_9831ca893b0d087d: return " entity";
	case hd2::file_type::VT2_92d3ee038eeb610d:
		return ".flow";
	case hd2::file_type::VT2_6a00deed7d3b235a:
		return ".flow_editor";
	//case hd2::file_type::VT2_9efe0a916aae7880: return " font";
	case hd2::file_type::VT2_d526a27da14f1dc5:
		return ".ini";
	case hd2::file_type::VT2_fa4a8e091a91201e:
		return ".ivf";
	case hd2::file_type::VT2_a62f9297dc969e85:
		return ".keys";
	//case hd2::file_type::VT2_2a690fd348fe9ac5: return " level";
	//case hd2::file_type::VT2_a14e8dfa2cd117e2: return " lua";
	//case hd2::file_type::VT2_eac0b497876adedf: return " material";
	case hd2::file_type::VT2_48ff313713a997a1:
		return ".mesh";
	case hd2::file_type::VT2_3fcdd69156a46417:
		return ".mod";
	case hd2::file_type::VT2_b277b11fe4a61d37:
		return ".mouse_cursor";
	case hd2::file_type::VT2_169de9566953d264:
		return ".navdata";
	case hd2::file_type::VT2_3b1fa9e8f6bac374:
		return ".network_config";
	case hd2::file_type::VT2_ad9c6d9ed1e5e77a:
		return ".package";
	//case hd2::file_type::VT2_a8193123526fad64: return " particles";
	case hd2::file_type::VT2_5f7203c8f280dab8:
		return ".physics";
	case hd2::file_type::VT2_22bd082459e868d8:
		return ".physics_data";
	case hd2::file_type::VT2_bf21403a3ab0bbb1:
		return ".physics_properties";
	case hd2::file_type::VT2_46cab355bd8d4d37:
		return ".prototype";
	//case hd2::file_type::VT2_27862fe24795319c: return " render_config";
	case hd2::file_type::VT2_d37e11a77baafc01:
		return ".resource";
	case hd2::file_type::VT2_9d0a795bfe818d19:
		return ".scene";
	case hd2::file_type::VT2_cce8d5b5f5ae333f:
		return ".shader";
	case hd2::file_type::VT2_e5ee32a477239a93:
		return ".shader_library";
	case hd2::file_type::VT2_9e5c3cc74575aeb5:
		return ".shader_library_group";
	//case hd2::file_type::VT2_fe73c7dcff8a7ca5: return " shading_environment";
	//case hd2::file_type::VT2_250e0a11ac8e26f8: return " shading_environment_mapping";
	case hd2::file_type::VT2_a27b4d04a9ba6f9e:
		return ".slug";
	case hd2::file_type::VT2_e9fc9ea7042e5ec0:
		return ".slug_album";
	case hd2::file_type::VT2_90641b51c98b7aac:
		return ".sound";
	case hd2::file_type::VT2_d8b27864a97ffdd7:
		return ".sound_environment";
	case hd2::file_type::VT2_f97af9983c05b950:
		return ".spu_job";
	//case hd2::file_type::VT2_a486d4045106165c: return " state_machine";
	case hd2::file_type::VT2_bb0279e548747a0a:
		return ".statemachine_editor";
	case hd2::file_type::VT2_0d972bab10b40fd3:
		return ".strings";
	case hd2::file_type::VT2_ad2d3fa30d9ab394:
		return ".surface_properties";
	//case hd2::file_type::VT2_cd4238c6a0c69e32: return " texture";
	case hd2::file_type::VT2_91f98cecac930de8:
		return ".timestamp";
	case hd2::file_type::VT2_94616b1760e1c463:
		return ".timpani";
	case hd2::file_type::VT2_99736be1fff739a4:
		return ".timpani_bank";
	case hd2::file_type::VT2_00a3e6c59a2b9c6c:
		return ".timpani_master";
	case hd2::file_type::VT2_19c792357c99f49b:
		return ".tome";
	//case hd2::file_type::VT2_e0a48d0be9a7453f:
	//	return ".unit";
	case hd2::file_type::VT2_8c074c8f95e7841b:
		return ".user_settings";
	//case hd2::file_type::VT2_f7505933166d6755: return " vector_field";
	case hd2::file_type::VT2_6ad9817aa72d9533:
		return ".volume";
	case hd2::file_type::VT2_786f65c00a816b19:
		return ".wav";
	//case hd2::file_type::VT2_535a7bd3e650d799: return " wwise_bank";
	//case hd2::file_type::VT2_af32095c82f2b070: return " wwise_dep";
	//case hd2::file_type::VT2_d50a8b7e1c82b110: return " wwise_metadata";
	//case hd2::file_type::VT2_504b55235d21440e: return " wwise_stream";
	case hd2::file_type::VT2_76015845a6003765:
		return ".xml";
	case hd2::file_type::VT2_712d6e3dd1024c9c:
		return ".ugg";
	case hd2::file_type::VT2_a99510c6e86dd3c2:
		return ".upb";
	case hd2::file_type::VT2_2bbcabe5074ade9e:
		return ".input";
	case hd2::file_type::VT2_52014e20a7fdccef:
		return ".xp";
	case hd2::file_type::VT2_7b82d55316f98c9b:
		return ".vista";
	case hd2::file_type::VT2_de8c9d00247f8fc6:
		return ".w7";
	case hd2::file_type::VT2_0605775c0481181b:
		return ".w8";
	case hd2::file_type::VT2_293bb3b1982c6d5a:
		return ".main";
	case hd2::file_type::VT2_b6b3099f78886ff9:
		return ".win32";
	case hd2::file_type::VT2_d596cd750e4e279f:
		return ".zD";

	default:
		return "";
	}
}

hd2::data::data(std::filesystem::path path)
{
	_meta_path = std::filesystem::absolute(path).replace_extension();
	_meta_size = std::filesystem::file_size(_meta_path);
	_meta      = {_meta_path};

	_stream_path = std::filesystem::absolute(path).replace_extension("stream");
	if (std::filesystem::exists(_stream_path)) {
		_stream_size = std::filesystem::file_size(_stream_path);
		_stream      = {_stream_path};
	}

	_gpu_path = std::filesystem::absolute(path).replace_extension("gpu");
	if (std::filesystem::exists(_gpu_path)) {
		_gpu_size = std::filesystem::file_size(_gpu_path);
		_gpu      = {_gpu_path};
	}

	_ptr      = reinterpret_cast<decltype(_ptr)>(&_meta);
	_ptr_type = reinterpret_cast<decltype(_ptr_type)>(&_meta + sizeof(header_t));
	_ptr_file = reinterpret_cast<decltype(_ptr_file)>(&_meta + sizeof(header_t) + sizeof(type_t) * _ptr->types);
	_ptr_data = reinterpret_cast<decltype(_ptr_data)>(&_meta + sizeof(header_t) + sizeof(type_t) * _ptr->types + sizeof(file_t) * _ptr->files);
}

size_t hd2::data::types() const
{
	return _ptr->types;
}

hd2::data::type_t const& hd2::data::type(size_t idx) const
{
	if (idx >= types()) {
		throw std::out_of_range("idx >= edx");
	}

	return _ptr_type[idx];
}

size_t hd2::data::files() const
{
	return _ptr->files;
}

hd2::data::file_t const& hd2::data::file(size_t idx) const
{
	if (idx >= files()) {
		throw std::out_of_range("idx >= edx");
	}

	return _ptr_file[idx];
}

bool hd2::data::has_meta(size_t idx) const
{
	if (idx >= files()) {
		throw std::out_of_range("idx >= edx");
	}

	return (file(idx).size > 0);
}

uint8_t const* hd2::data::meta_data(size_t idx) const
{
	if (idx >= files()) {
		throw std::out_of_range("idx >= edx");
	}

	if (!has_meta(idx)) {
		return nullptr;
	}

	if (file(idx).offset >= _meta_size) {
		throw std::overflow_error("offset >= size");
	} else if (file(idx).size + file(idx).offset > _meta_size) {
		throw std::overflow_error("offset+size > size");
	}

	return &_meta + file(idx).offset;
}

size_t hd2::data::meta_size(size_t idx) const
{
	if (idx >= files()) {
		throw std::out_of_range("idx >= edx");
	}

	return file(idx).size;
}

bool hd2::data::has_stream(size_t idx) const
{
	if (idx >= files()) {
		throw std::out_of_range("idx >= edx");
	}

	return (_stream.has_value()) && (file(idx).stream_size > 0);
}

uint8_t const* hd2::data::stream(size_t idx) const
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

size_t hd2::data::stream_size(size_t idx) const
{
	if (idx >= files()) {
		throw std::out_of_range("idx >= edx");
	}

	if (!has_stream(idx)) {
		return 0;
	}

	return file(idx).stream_size;
}

bool hd2::data::has_gpu(size_t idx) const
{
	if (idx >= files()) {
		throw std::out_of_range("idx >= edx");
	}

	return (_gpu.has_value()) && (file(idx).gpu_size > 0);
}

uint8_t const* hd2::data::gpu(size_t idx) const
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

size_t hd2::data::gpu_size(size_t idx) const
{
	if (idx >= files()) {
		throw std::out_of_range("idx >= edx");
	}

	if (!has_gpu(idx)) {
		return 0;
	}

	return file(idx).gpu_size;
}
