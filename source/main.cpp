// Copyright 2024 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// ToDo:
// - Look into ICU for Unicode handling: https://icu.unicode.org/

#include <cinttypes>
#include <cstddef>
#include <cstdio>
#include <cstring>

#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <utility>

#include "hasher.hpp"
#include "mapped_file.hpp"
#include "string_printf.hpp"

#include "hd2_data.hpp"

#if __BIG_ENDIAN__
#define htole16(x) ((uint16_t)(((x) >> 8) & 0xFF) | ((uint16_t)((x)&0xFF) << 8))
#define htole32(x) (((uint32_t)htobe16(((x) >> 16) & 0xFFFF)) | ((uint32_t)htobe16((x)&0xFFFF) << 16))
#define htole64(x) (((uint64_t)htobe32(((x) >> 32) & 0xFFFFFFFF)) | ((uint64_t)htobe32((x)&0xFFFFFFFF) << 32))
#define htobe16(x) (x)
#define htobe32(x) (x)
#define htobe64(x) (x)
#else
#define htobe16(x) ((uint16_t)(((x) >> 8) & 0xFF) | ((uint16_t)((x)&0xFF) << 8))
#define htobe32(x) (((uint32_t)htobe16(((x) >> 16) & 0xFFFF)) | ((uint32_t)htobe16((x)&0xFFFF) << 16))
#define htobe64(x) (((uint64_t)htobe32(((x) >> 32) & 0xFFFFFFFF)) | ((uint64_t)htobe32((x)&0xFFFFFFFF) << 32))
#define htole16(x) (x)
#define htole32(x) (x)
#define htole64(x) (x)
#endif

#define le16toh(x) htole16(x)
#define le32toh(x) htole32(x)
#define le64toh(x) htole64(x)
#define be16toh(x) htobe16(x)
#define be32toh(x) htobe32(x)
#define be64toh(x) htobe64(x)

int main(int argc, const char** argv)
try {
	enum class mode {
		EXTRACT,
		MESH,
		HASH,
	} currentmode                 = mode::EXTRACT;
	std::filesystem::path  output = std::filesystem::absolute(argv[0]).parent_path();
	std::list<std::string> unparsed;

	if (argc <= 1) {
		std::cout << std::filesystem::path(argv[0]).filename() << " [options] file [file]" << std::endl;
		std::cout << "Options" << std::endl;
		std::cout << "  -m, --mode" << std::endl;
		std::cout << "    extract: Extract data from the data files" << std::endl;
		std::cout << "    mesh: Convert .meshinfo (and its accompanying .mesh file) into model data" << std::endl;
		std::cout << "    hash: Hash the provided arguments" << std::endl;
		return 1;
	}

	// Parse arguments
	for (size_t argn = 1; argn < argc; ++argn) {
		std::string_view arg = argv[argn];
		if (("-m" == arg) || ("--mode" == arg)) {
			if ((argn + 1) < argc) {
				std::string_view val = argv[++argn];
				if ("extract" == val) {
					currentmode = mode::EXTRACT;
				} else if ("mesh" == val) {
					currentmode = mode::MESH;
				} else if ("hash" == val) {
					currentmode = mode::HASH;
				} else {
					throw std::runtime_error("Unknown mode");
				}
			} else {
				throw std::runtime_error("String expected, got EOL instead");
			}
		} else if (("-o" == arg) || ("--output" == arg)) {
			if ((argn + 1) < argc) {
				std::string_view val = argv[++argn];
				output               = std::filesystem::absolute(val);
			} else {
				throw std::runtime_error("String expected, got EOL instead");
			}
		} else if ('-' == arg[0]) {
			throw std::runtime_error(string_printf("Unrecognized format: %s", arg.data()));
		} else {
			unparsed.push_back(std::string(arg));
		}
	}

	auto enumerate_files = [](std::filesystem::path const& path, std::function<bool(std::filesystem::path const&)> filter) {
		if (!std::filesystem::is_directory(path)) {
			return std::set<std::filesystem::path>{path};
		}

		std::set<std::filesystem::path> paths;
		for (auto const& file : std::filesystem::recursive_directory_iterator(path)) {
			if (!file.is_regular_file()) {
				continue;
			}

			if (filter(file.path())) {
				paths.insert(std::filesystem::absolute(file.path()));
			}
		}

		return paths;
	};

	std::cout << string_printf("Output Path: %s", output.generic_u8string().c_str()) << std::endl;

	if (currentmode == mode::EXTRACT) {
		// Figure out all the files we need to process.
		auto filter = [](std::filesystem::path const& path) {
			if (path.has_extension()) {
				return false;
			}
			return true;
		};
		std::set<std::filesystem::path> real_paths;
		for (auto const& path : unparsed) {
			real_paths.merge(enumerate_files(path, filter));
		}

		std::cout << string_printf("%zu Input Files: ", real_paths.size()) << std::endl;
		for (auto const& path : real_paths) {
			std::cout << string_printf("    %s", path.generic_u8string().c_str()) << std::endl;
		}

		// Load all the containers into memory. This should ideally only happen with virtual memory.
		std::list<hd2::data> containers;
		for (auto const& path : real_paths) {
			containers.emplace_back(path);
		}

		// Create a full tree of {type, id}, {container, index}, so we don't waste time extracting things multiple times.
		std::map<std::pair<hd2::file_id_t, hd2::file_type>, std::pair<hd2::data const&, size_t>> files;
		for (auto const& cont : containers) {
			for (size_t i = 0; i < cont.files(); i++) {
				auto const& file = cont.file(i);
				files.try_emplace(std::pair<hd2::file_id_t, hd2::file_type>{file.id, file.type}, std::pair<hd2::data const&, size_t>{cont, i});
				// Files may exist multiple times, but they are usually exactly identical.
			}
		}
		std::cout << string_printf("%zu Output Files: ", files.size()) << std::endl;
		for (auto const& file : files) {
			std::cout << string_printf("    %016" PRIx64 " %016" PRIx64, htobe64((uint64_t)file.first.first), htobe64((uint64_t)file.first.second)) << std::endl;
		}

		// Export all discovered files.
		std::cout << "Exporting: " << std::endl;
		std::filesystem::create_directories(output);
		for (auto const& kv : files) {
			auto const& cont  = kv.second.first;
			auto const& file  = kv.second.first.file(kv.second.second);
			auto        index = kv.second.second;

			std::string ext = std::string{hd2::file_extension_from_type(file.type)};
			if (ext.length() == 0) {
				ext = string_printf("%016" PRIx64, htobe64((uint64_t)file.type));
			}

			std::filesystem::path name = output / string_printf("%016" PRIx64 "%s", htobe64((uint64_t)file.id), ext.c_str());

			if (!std::filesystem::exists(name)) {
				std::cout << string_printf("    [%08" PRIX32 ":%08" PRIX32 "] [%08" PRIX32 ":%08" PRIX32 "] [%08" PRIX32 ":%08" PRIX32 "] %s ...", file.offset, file.size, file.stream_offset, file.stream_size, file.gpu_offset, file.gpu_size, std::filesystem::relative(name, output).generic_string().c_str());

				std::ofstream filestream{name, std::ios::trunc | std::ios::binary | std::ios::out};
				if (!filestream || filestream.bad() || !filestream.is_open()) {
					throw std::runtime_error(string_printf("Failed to export '%s'.", name.generic_u8string().c_str()));
				}
				filestream.flush();

				if (cont.has_meta(index)) {
					filestream.write(reinterpret_cast<char const*>(cont.meta_data(index)), cont.meta_size(index));
				}
				filestream.flush();

				if (cont.has_stream(index)) {
					filestream.write(reinterpret_cast<char const*>(cont.stream(index)), cont.stream_size(index));
				}
				filestream.flush();

				if (cont.has_gpu(index)) {
					filestream.write(reinterpret_cast<char const*>(cont.gpu(index)), cont.gpu_size(index));
				}
				filestream.flush();

				filestream.close();

				std::cout << " Done." << std::endl;
			} else {
				std::cout << string_printf("    Skipped %s.", std::filesystem::relative(name, output).generic_string().c_str()) << std::endl;
			}
		}
	} else if (currentmode == mode::HASH) {
		auto inst = hellextractor::hash::instance::create(hellextractor::hash::type::MURMURHASH64A);

		for (auto const& key : unparsed) {
			auto hash = inst->hash(key.data(), key.size());
			uint64_t rhash = *reinterpret_cast<uint64_t const*>(hash.data());
			std::cout << string_printf("%016" PRIx64 "/%016" PRIx64 " = %s", htole64(rhash), htobe64(rhash), key.c_str());
		}
	} else if (currentmode == mode::MESH) {
		auto filter = [](std::filesystem::path const& path) {
			if (path.extension() != ".meshinfo") {
				return false;
			}
			return true;
		};

		std::set<std::filesystem::path> real_paths;
		for (auto const& path : unparsed) {
			real_paths.merge(enumerate_files(path, filter));
		}

		throw std::runtime_error("Not yet implemented");
		/*

	auto path          = std::filesystem::path{argv[1]};
	auto meshinfo_path = path.replace_extension(".meshinfo");
	auto mesh_path     = path.replace_extension(".mesh");
	auto output_path   = path.replace_extension();

#ifdef WIN32
	win32_handle_t mesh_info_file(CreateFileA(reinterpret_cast<LPCSTR>(meshinfo_path.generic_u8string().c_str()), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL));
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

	win32_handle_t mesh_file(CreateFileA(reinterpret_cast<LPCSTR>(mesh_path.generic_u8string().c_str()), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL));
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

	hd2::meshinfo_t meshinfo{reinterpret_cast<uint8_t const*>(mesh_info_ptr)};
	auto            materials = meshinfo.materials();
	auto            datatypes = meshinfo.datatypes();
	auto            meshes    = meshinfo.meshes();

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

			printf("  - %" PRIu32 " Vertices, Stride: %" PRIu32 ", ", mesh.modeldata().vertices(), datatype.vertices_stride());
			printf("Offset: %08" PRIx32 ", ", mesh.modeldata().vertices_offset());
			printf("From: %08" PRIx32 ", ", mesh.modeldata().vertices_offset() + datatype.vertices_offset());
			printf("To: %08" PRIx32 "\n", mesh.modeldata().vertices_offset() + datatype.vertices_offset() + mesh.modeldata().vertices() * datatype.vertices_stride());

			printf("  - %" PRIu32 " Indices, Stride: %" PRIu32 ", ", mesh.modeldata().indices(), datatype.indices_stride());
			printf("Offset: %08" PRIx32 ", ", mesh.modeldata().indices_offset());
			printf("From: %08" PRIx32 ", ", mesh.modeldata().indices_offset() + datatype.indices_offset());
			printf("To: %08" PRIx32 "\n", mesh.modeldata().indices_offset() + datatype.indices_offset() + mesh.modeldata().indices() * datatype.indices_stride());

			printf("  - %" PRIu32 " Materials: ", mesh.materials());
			for (size_t jdx = 0; jdx < mesh.materials(); jdx++) {
				printf("%" PRIx32 ", ", mesh.material(jdx));
			}
			printf("\n");
		}
	}

	//auto dt_path = std::filesystem::absolute(output_path.parent_path() / "../datatypes");
	//{ // Export raw datatypes
	//	std::filesystem::create_directories(dt_path);

	//	for (size_t idx = 0; idx < datatypes.count(); idx++) {
	//		auto        datatype = datatypes.at(idx);
	//		std::string dt_hash  = datatype.hash();

	//		// Generate name.
	//		std::string filename = std::format("%s/%02" PRIu32 "_%s.dt", dt_path.generic_string().c_str(), datatype.vertices_stride(), dt_hash.c_str());

	//		if (!std::filesystem::exists(filename)) {
	//			auto   dtfilehandle = fopen(filename.c_str(), "w+b");
	//			file_t dtfile{dtfilehandle};
	//			fwrite(&datatype.unique(), 1, sizeof(hd2::meshinfo_datatype_t::data_t), dtfile.get());
	//		}
	//	}
	//}

	{ // Export meshes.
		std::filesystem::create_directories(output_path);

		printf("Exporting Meshes...\n");
		for (size_t idx = 0; idx < meshes.count(); idx++) {
			printf("- [%zu] Exporting...\n", idx);
			auto mesh     = meshes.at(idx);
			auto datatype = datatypes.at(mesh.datatype_index());

			//{ // Write sample file for datatype.
			//	std::string filename = std::format("%s/%02" PRIu32 "_%s.vtx", dt_path.generic_string().c_str(), datatype.vertices_stride(), datatype.hash().c_str());

			//	if (!std::filesystem::exists(filename)) {
			//		auto   dtfilehandle = fopen(filename.c_str(), "w+b");
			//		file_t dtfile{dtfilehandle};

			//		std::ptrdiff_t ptr = datatype.vertices_offset() + mesh.modeldata().vertices_offset() * datatype.vertices_stride();

			//		fwrite(reinterpret_cast<void const*>(mesh_ptr + ptr), datatype.vertices_stride(), std::min<size_t>(mesh.modeldata().vertices_count(), 128), dtfile.get());
			//	}
			//}

			{ // Write raw file.
				std::string file_name;
				file_t      file;
				{
					{
						const char*       format = "%s/%08" PRIu32 ".raw";
						std::vector<char> buf;
						size_t            bufsz = snprintf(nullptr, 0, format, output_path.u8string().c_str(), idx);
						buf.resize(bufsz + 1);
						snprintf(buf.data(), buf.size(), format, output_path.u8string().c_str(), idx);
						file_name = {buf.data(), buf.data() + buf.size() - 1};
					}
					auto file_handle = fopen(file_name.c_str(), "w+b");
					file.reset(file_handle);
				}

				struct offsets_t {
					size_t dt;
					size_t mesh;
					size_t vtx;
					size_t idx;
				};

				offsets_t offsets;
				offsets.dt   = sizeof(offsets_t);
				offsets.mesh = offsets.dt + sizeof(hd2::meshinfo_datatype_t::data_t);
				offsets.vtx  = offsets.mesh + mesh.size();
				offsets.idx  = offsets.vtx + (datatype.vertices_stride() * mesh.modeldata().vertices());

				fwrite(&offsets, sizeof(offsets_t), 1, file.get());

				// Write datatype information.
				fwrite(datatype._ptr, sizeof(hd2::meshinfo_datatype_t::data_t), 1, file.get());

				// Write mesh information.
				fwrite(&mesh, mesh.size(), 1, file.get());

				// Write vertex information.
				fwrite(mesh_ptr + datatype.vertices_offset() + mesh.modeldata().vertices_offset() * datatype.vertices_stride(), datatype.vertices_stride(), mesh.modeldata().vertices(), file.get());

				// Write index information.
				fwrite(mesh_ptr + datatype.indices_offset() + mesh.modeldata().indices_offset() * datatype.indices_stride(), datatype.indices_stride(), mesh.modeldata().indices(), file.get());
			}

			{ // Write model file.
				std::string file_name;
				file_t      file;
				{
					{
						const char*       format = "%s/%08" PRIu32 ".obj";
						std::vector<char> buf;
						size_t            bufsz = snprintf(nullptr, 0, format, output_path.u8string().c_str(), idx);
						buf.resize(bufsz + 1);
						snprintf(buf.data(), buf.size(), format, output_path.u8string().c_str(), idx);
						file_name = {buf.data(), buf.data() + buf.size() - 1};
					}
					auto file_handle = fopen(file_name.c_str(), "w+b");
					file.reset(file_handle);
					;
				}

				// Write overall data.
				fprintf(file.get(), "o %s\n", file_name.c_str());
				fprintf(file.get(), "g %s\n", file_name.c_str());
				fprintf(file.get(), "s 0\n");
				fflush(file.get());

				{ // Write all vertices.
					std::ptrdiff_t ptr         = datatype.vertices_offset() + mesh.modeldata().vertices_offset() * datatype.vertices_stride();
					std::ptrdiff_t max_ptr     = ptr + mesh.modeldata().vertices() * datatype.vertices_stride();
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

					for (size_t vtx = 0; vtx < mesh.modeldata().vertices(); vtx++) {
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

							case hd2::vertex_element_type::_01_color: {
								switch (element.format) {
								case hd2::vertex_element_format::_1E_implied_half3: {
									half const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
									fprintf(file.get(), "# vc %#16.8g %#16.8g %#16.8g\n", (float)vec_ptr[0], (float)vec_ptr[1], (float)vec_ptr[2]);
									break;
								}
								case hd2::vertex_element_format::f32vec3: {
									float const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
									fprintf(file.get(), "# vc %#16.8g %#16.8g %#16.8g\n", vec_ptr[0], vec_ptr[1], vec_ptr[2]);
									break;
								}

								case hd2::vertex_element_format::f16vec4: {
									half const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
									fprintf(file.get(), "# vc %#16.8g %#16.8g %#16.8g %#16.8g\n", (float)vec_ptr[0], (float)vec_ptr[1], (float)vec_ptr[2], (float)vec_ptr[3]);
									break;
								}
								case hd2::vertex_element_format::_03_implied_float4: {
									float const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
									fprintf(file.get(), "# vc %#16.8g %#16.8g %#16.8g %#16.8g\n", vec_ptr[0], vec_ptr[1], vec_ptr[2], vec_ptr[3]);
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
					std::ptrdiff_t max_ptr     = ptr + mesh.modeldata().indices() * datatype.indices_stride();
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

					for (size_t idx = 0; idx < mesh.modeldata().indices(); idx += 3) {
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
			}
		}
	}

	std::cout << std::endl;
	*/
	} else {
		throw std::runtime_error("Programmer messed up and didn't actually implement this.");
	}

	return 0;
} catch (const std::exception& ex) {
	std::cerr << "Exception: " << ex.what() << std::endl;
	return 1;
} catch (...) {
	std::cerr << "Encountered unknown exception." << std::endl;
	return 1;
}

/*{ // least efficient brute force search I could come up with. Just to verify the hash function works.
	std::set<uint64_t> hashes = {
		0x05106B81DCD58A13, 0x18DEAD01056B72E9, 0x1D59BD6687DB6B33, 0x250E0A11AC8E26F8, 0x27862FE24795319C, 0x2A690FD348FE9AC5, 0x504B55235D21440E, 0x535A7BD3E650D799, 0x57A13425279979D7, 0x5F7203C8F280DAB8, 0x9199BB50B6896F02, 0x9831CA893B0D087D, 0x931E336D7646CC26, 0x9EFE0A916AAE7880, 0xA14E8DFA2CD117E2, 0xA486D4045106165C, 0xA8193123526FAD64, 0xAA5965F03029FA18, 0xAB2F78E885F513C6, 0xAF32095C82F2B070, 0xC4F0F4BE7FB0C8D6, 0xCD4238C6A0C69E32, 0xD7014A50477953E0, 0xE0A48D0BE9A7453F, 0xE985C5F61C169997, 0xEAC0B497876ADEDF, 0xF7505933166D6755, 0xFE73C7DCFF8A7CA5, 0xac2b738a374cf583, 0xdcfb9e18fff13984, 0x44957ef5a3a368e4, 0x3eed05ba83af5090, 0x7ffdb779b04e4ed1, 0xe301e8af94e3b5a3, 0xb7893adf7567506a, 0xf7b361bde8b4dd03, 0xfe9754bd19814a47, 0x0001a6d201e87086, 0x82645835e6b73232, 0x69108ded1e3e634b, 0x8fd0d44d20650b68, 0xed74b77c6d447d84, 0x92d3ee038eeb610d, 0x6a00deed7d3b235a, 0xd526a27da14f1dc5, 0xfa4a8e091a91201e, 0xa62f9297dc969e85, 0x48ff313713a997a1, 0x3fcdd69156a46417, 0xb277b11fe4a61d37, 0x169de9566953d264, 0x3b1fa9e8f6bac374, 0xad9c6d9ed1e5e77a, 0x5f7203c8f280dab8, 0x22bd082459e868d8, 0xbf21403a3ab0bbb1, 0x46cab355bd8d4d37, 0xd37e11a77baafc01, 0x9d0a795bfe818d19, 0xcce8d5b5f5ae333f, 0xe5ee32a477239a93, 0x9e5c3cc74575aeb5, 0xa27b4d04a9ba6f9e, 0xe9fc9ea7042e5ec0, 0x90641b51c98b7aac, 0xd8b27864a97ffdd7, 0xf97af9983c05b950, 0xbb0279e548747a0a, 0x0d972bab10b40fd3, 0xad2d3fa30d9ab394, 0x91f98cecac930de8, 0x94616b1760e1c463, 0x99736be1fff739a4, 0x00a3e6c59a2b9c6c, 0x19c792357c99f49b, 0xe0a48d0be9a7453f, 0x8c074c8f95e7841b, 0x6ad9817aa72d9533, 0x786f65c00a816b19, 0x76015845a6003765, 0x712d6e3dd1024c9c, 0xa99510c6e86dd3c2, 0x2bbcabe5074ade9e, 0x52014e20a7fdccef, 0x7b82d55316f98c9b, 0xde8c9d00247f8fc6, 0x0605775c0481181b, 0x293bb3b1982c6d5a, 0xb6b3099f78886ff9, 0xd596cd750e4e279f,
	};

	// can convert this to a depth search?
	// Steps would be:
	// 1. Loop through valid for single characters
	// 2. Loop through valid for two characters
	//   1. Loop through the 2nd character until at max.
	//   2. Increment 1st chracter, repeat 1 until at max.
	// etc.
	// May end up significantly faster.
	constexpr size_t     max_depth = 16;
	std::vector<char>    buffer(max_depth, 0);
	std::vector<uint8_t> counter(max_depth, 0);
	std::string_view     valid = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";

	for (size_t depth = 1; depth < max_depth; ++depth) {
		printf("Depth: %zu\n", depth);
		bool breakout = false;
		while (!breakout) {
			// Update buffer and counter.
			// - Assign the proper value to the buffer at the location.
			for (size_t idx = 0; idx < depth; idx++) {
				buffer[idx] = valid[counter[idx]];
			}
			// - Update the counter and propagate upward if there is space.
			for (size_t idx = (depth - 1); idx < depth; idx--) {
				if (++counter[idx] < valid.size()) {
					break;
				} else {
					counter[idx] = 0; // Let it propagate upwards
					if (idx == 0) {
						breakout = true;
					}
				}
			}
			auto hash = murmurhash64A(buffer.data(), depth, 0);
			if (hashes.find(hash) != hashes.end()) {
				printf("  %018" PRIX64 " = %.*s\n", hash, (int)depth, buffer.data());
			}
		}
	}

	return 0;
}*/
/*{
std::string arg = "";
do {
	std::getline(std::cin, arg);
	printf("%016" PRIx64 "\n", murmurhash64A(arg.c_str(), arg.size(), 0));
} while (!arg.empty());
return 0;
}*/
