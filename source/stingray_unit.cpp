// Copyright 2024 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "stingray_unit.hpp"
#include <stdexcept>

stingray::unit::mesh::~mesh() {}

stingray::unit::mesh::mesh() : _ptr(), _material_ptr(), _group_ptr() {}

stingray::unit::mesh::mesh(uint8_t const* ptr)
{
	_ptr          = reinterpret_cast<decltype(_ptr)>(ptr);
	_material_ptr = reinterpret_cast<decltype(_material_ptr)>(ptr + _ptr->material_offset);
	_group_ptr    = reinterpret_cast<decltype(_group_ptr)>(ptr + _ptr->group_offset);
}

stingray::unit::mesh_list::~mesh_list() {}

stingray::unit::mesh_list::mesh_list() : _ptr(), _offset_ptr(), _name_ptr(), _mesh_ptr(), _meshes(), _map() {}

stingray::unit::mesh_list::mesh_list(uint8_t const* ptr)
{
	_ptr        = reinterpret_cast<decltype(_ptr)>(ptr);
	_offset_ptr = reinterpret_cast<decltype(_offset_ptr)>(ptr + sizeof(data_t));
	_name_ptr   = reinterpret_cast<decltype(_name_ptr)>(ptr + sizeof(data_t) + (sizeof(uint32_t) * _ptr->count));
	_mesh_ptr   = reinterpret_cast<decltype(_mesh_ptr)>(ptr + sizeof(data_t) + (sizeof(uint32_t) * _ptr->count) + (sizeof(stingray::thin_hash_t) * _ptr->count));

	_meshes.reserve(_ptr->count);
	for (size_t idx = 0; idx < _ptr->count; idx++) {
		_meshes.emplace_back(std::make_shared<stingray::unit::mesh>(_mesh_ptr));
	}
	for (size_t idx = 0; idx < _ptr->count; idx++) {
		_map.emplace(_name_ptr[idx], _meshes[idx]);
	}
}

size_t stingray::unit::mesh_list::size()
{
	return _meshes.size();
}

std::shared_ptr<stingray::unit::mesh> stingray::unit::mesh_list::at(size_t idx)
{
	return _meshes[idx];
}

std::map<stingray::thin_hash_t, std::shared_ptr<stingray::unit::mesh>> stingray::unit::mesh_list::get()
{
	return _map;
}

stingray::unit::node_list::~node_list() {}

stingray::unit::node_list::node_list() : _ptr(), _trss_ptr(), _name_ptr(), _metas(), _map() {}

stingray::unit::node_list::node_list(uint8_t const* ptr)
{
	_ptr      = reinterpret_cast<decltype(_ptr)>(ptr);
	_trss_ptr = reinterpret_cast<decltype(_trss_ptr)>(ptr + sizeof(header_t));
	_name_ptr = reinterpret_cast<decltype(_name_ptr)>(ptr + sizeof(header_t) + (sizeof(float) * 16 * _ptr->count));

	_metas.reserve(_ptr->count + 1);
	for (size_t idx = 0; idx < _ptr->count; idx++) {
		_metas.emplace_back(std::make_shared<stingray::unit::node_list::meta_t>(_name_ptr[idx], &_trss_ptr[idx], &_link_ptr[idx]));
	}
	for (size_t idx = 0; idx < _ptr->count; idx++) {
		_map.emplace(_name_ptr[idx], _metas[idx]);
	}
}

size_t stingray::unit::node_list::size()
{
	return _metas.size();
}

std::shared_ptr<stingray::unit::node_list::meta_t> stingray::unit::node_list::at(size_t idx)
{
	return _metas[idx];
}

std::map<stingray::thin_hash_t, std::shared_ptr<stingray::unit::node_list::meta_t>> const& stingray::unit::node_list::get()
{
	return _map;
}

stingray::unit::material_list::~material_list() {}

stingray::unit::material_list::material_list() : _ptr(), _key_ptr(), _value_ptr(), _map() {}

stingray::unit::material_list::material_list(uint8_t const* ptr)
{
	_ptr       = reinterpret_cast<decltype(_ptr)>(ptr);
	_key_ptr   = reinterpret_cast<decltype(_key_ptr)>(ptr + sizeof(header_t));
	_value_ptr = reinterpret_cast<decltype(_value_ptr)>(ptr + sizeof(header_t) + (sizeof(stingray::thin_hash_t) * _ptr->count));

	for (size_t idx = 0; idx < _ptr->count; idx++) {
		_map.emplace(_key_ptr[idx], _value_ptr[idx]);
	}
}

std::map<stingray::thin_hash_t, stingray::hash_t> const& stingray::unit::material_list::get()
{
	return _map;
}

stingray::unit::unit::~unit() {}

stingray::unit::unit::unit(stingray::data_110000F0::meta_t meta) : _meta(meta)
{
	_data    = reinterpret_cast<decltype(_data)>(_meta.main);
	_data_sz = _meta.main_size;

	_ptr           = reinterpret_cast<decltype(_ptr)>(_meta.main);
	_material_list = {reinterpret_cast<uint8_t const*>(_meta.main) + _ptr->materials_offset};
}

size_t stingray::unit::unit::size()
{
	return _data_sz;
}

std::string stingray::unit::unit::extension()
{
	return "unit";
}

std::list<std::pair<void const*, size_t>> stingray::unit::unit::sections()
{
	return {
		{_data, _data_sz},
	};
}
