//
//auto filter = [](std::filesystem::path const& path) {
//	if (path.extension() != ".meshinfo") {
//		return false;
//	}
//	return true;
//	};
//
//std::set<std::filesystem::path> real_paths;
//for (auto const& path : unparsed) {
//	real_paths.merge(enumerate_files(path, filter));
//}
//
//throw std::runtime_error("Not yet implemented");
///*
//
//auto path          = std::filesystem::path{argv[1]};
//auto meshinfo_path = path.replace_extension(".meshinfo");
//auto mesh_path     = path.replace_extension(".mesh");
//auto output_path   = path.replace_extension();
//
//#ifdef WIN32
//	win32_handle_t mesh_info_file(CreateFileA(reinterpret_cast<LPCSTR>(meshinfo_path.generic_u8string().c_str()), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL));
//	if (mesh_info_file.get() == INVALID_HANDLE_VALUE) {
//		return 1;
//	}
//
//	win32_handle_t mesh_info_fmap(CreateFileMapping(mesh_info_file.get(), NULL, PAGE_READONLY, 0, 0, NULL));
//	if (mesh_info_fmap.get() == INVALID_HANDLE_VALUE) {
//		return 1;
//	}
//
//	uint8_t const* mesh_info_ptr = reinterpret_cast<uint8_t const*>(MapViewOfFile(mesh_info_fmap.get(), FILE_MAP_READ, 0, 0, 0));
//	if (!mesh_info_ptr) {
//		return 1;
//	}
//
//	win32_handle_t mesh_file(CreateFileA(reinterpret_cast<LPCSTR>(mesh_path.generic_u8string().c_str()), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL));
//	if (mesh_file.get() == INVALID_HANDLE_VALUE) {
//		return 1;
//	}
//
//	win32_handle_t mesh_fmap(CreateFileMapping(mesh_file.get(), NULL, PAGE_READONLY, 0, 0, NULL));
//	if (mesh_fmap.get() == INVALID_HANDLE_VALUE) {
//		return 1;
//	}
//
//	uint8_t const* mesh_ptr = reinterpret_cast<uint8_t const*>(MapViewOfFile(mesh_fmap.get(), FILE_MAP_READ, 0, 0, 0));
//	if (!mesh_ptr) {
//		return 1;
//	}
//#else
//#pragma error("Not supported yet, do some work to make it work.")
//#endif
//
//	hd2::meshinfo_t meshinfo{reinterpret_cast<uint8_t const*>(mesh_info_ptr)};
//	auto            materials = meshinfo.materials();
//	auto            datatypes = meshinfo.datatypes();
//	auto            meshes    = meshinfo.meshes();
//
//	{ // Enumerate Materials
//		printf("%" PRIu32 " Materials: \n", materials.count());
//		for (size_t idx = 0; idx < materials.count(); idx++) {
//			auto material = materials.at(idx);
//			printf("- [%zu] %08" PRIx32 " = %" PRIx64 "\n", idx, material.first, material.second);
//		}
//	}
//
//	{ // Enumerate Data Types
//		printf("%" PRIu32 " Data Types: \n", datatypes.count());
//		for (size_t idx = 0; idx < datatypes.count(); idx++) {
//			auto datatype = datatypes.at(idx);
//			printf("- [%zu] \n", idx);
//			printf("  - %" PRIu32 " Vertices, ", datatype.vertices());
//			printf("From: %08" PRIx32 ", ", datatype.vertices_offset());
//			printf("To: %08" PRIx32 ", ", datatype.vertices_offset() + datatype.vertices_size());
//			printf("Stride: %" PRId32 "\n", datatype.vertices_stride());
//			printf("  - %" PRIu32 " Indices, ", datatype.indices());
//			printf("From: %08" PRIx32 ", ", datatype.indices_offset());
//			printf("To: %08" PRIx32 ", ", datatype.indices_offset() + datatype.indices_size());
//			printf("Stride: %" PRId32 "\n", datatype.indices_stride());
//		}
//	}
//
//	{ // Enumerate Meshes
//		printf("%" PRIu32 " Meshes: \n", meshes.count());
//		for (size_t idx = 0; idx < meshes.count(); idx++) {
//			auto mesh     = meshes.at(idx);
//			auto datatype = datatypes.at(mesh.datatype_index());
//			printf("- [%zu] Data Type: %" PRIu32 ", %s\n", idx, mesh.datatype_index(), datatype.hash().c_str());
//
//			printf("  - %" PRIu32 " Vertices, Stride: %" PRIu32 ", ", mesh.modeldata().vertices(), datatype.vertices_stride());
//			printf("Offset: %08" PRIx32 ", ", mesh.modeldata().vertices_offset());
//			printf("From: %08" PRIx32 ", ", mesh.modeldata().vertices_offset() + datatype.vertices_offset());
//			printf("To: %08" PRIx32 "\n", mesh.modeldata().vertices_offset() + datatype.vertices_offset() + mesh.modeldata().vertices() * datatype.vertices_stride());
//
//			printf("  - %" PRIu32 " Indices, Stride: %" PRIu32 ", ", mesh.modeldata().indices(), datatype.indices_stride());
//			printf("Offset: %08" PRIx32 ", ", mesh.modeldata().indices_offset());
//			printf("From: %08" PRIx32 ", ", mesh.modeldata().indices_offset() + datatype.indices_offset());
//			printf("To: %08" PRIx32 "\n", mesh.modeldata().indices_offset() + datatype.indices_offset() + mesh.modeldata().indices() * datatype.indices_stride());
//
//			printf("  - %" PRIu32 " Materials: ", mesh.materials());
//			for (size_t jdx = 0; jdx < mesh.materials(); jdx++) {
//				printf("%" PRIx32 ", ", mesh.material(jdx));
//			}
//			printf("\n");
//		}
//	}
//
//	//auto dt_path = std::filesystem::absolute(output_path.parent_path() / "../datatypes");
//	//{ // Export raw datatypes
//	//	std::filesystem::create_directories(dt_path);
//
//	//	for (size_t idx = 0; idx < datatypes.count(); idx++) {
//	//		auto        datatype = datatypes.at(idx);
//	//		std::string dt_hash  = datatype.hash();
//
//	//		// Generate name.
//	//		std::string filename = std::format("%s/%02" PRIu32 "_%s.dt", dt_path.generic_string().c_str(), datatype.vertices_stride(), dt_hash.c_str());
//
//	//		if (!std::filesystem::exists(filename)) {
//	//			auto   dtfilehandle = fopen(filename.c_str(), "w+b");
//	//			file_t dtfile{dtfilehandle};
//	//			fwrite(&datatype.unique(), 1, sizeof(hd2::meshinfo_datatype_t::data_t), dtfile.get());
//	//		}
//	//	}
//	//}
//
//	{ // Export meshes.
//		std::filesystem::create_directories(output_path);
//
//		printf("Exporting Meshes...\n");
//		for (size_t idx = 0; idx < meshes.count(); idx++) {
//			printf("- [%zu] Exporting...\n", idx);
//			auto mesh     = meshes.at(idx);
//			auto datatype = datatypes.at(mesh.datatype_index());
//
//			//{ // Write sample file for datatype.
//			//	std::string filename = std::format("%s/%02" PRIu32 "_%s.vtx", dt_path.generic_string().c_str(), datatype.vertices_stride(), datatype.hash().c_str());
//
//			//	if (!std::filesystem::exists(filename)) {
//			//		auto   dtfilehandle = fopen(filename.c_str(), "w+b");
//			//		file_t dtfile{dtfilehandle};
//
//			//		std::ptrdiff_t ptr = datatype.vertices_offset() + mesh.modeldata().vertices_offset() * datatype.vertices_stride();
//
//			//		fwrite(reinterpret_cast<void const*>(mesh_ptr + ptr), datatype.vertices_stride(), std::min<size_t>(mesh.modeldata().vertices_count(), 128), dtfile.get());
//			//	}
//			//}
//
//			{ // Write raw file.
//				std::string file_name;
//				file_t      file;
//				{
//					{
//						const char*       format = "%s/%08" PRIu32 ".raw";
//						std::vector<char> buf;
//						size_t            bufsz = snprintf(nullptr, 0, format, output_path.u8string().c_str(), idx);
//						buf.resize(bufsz + 1);
//						snprintf(buf.data(), buf.size(), format, output_path.u8string().c_str(), idx);
//						file_name = {buf.data(), buf.data() + buf.size() - 1};
//					}
//					auto file_handle = fopen(file_name.c_str(), "w+b");
//					file.reset(file_handle);
//				}
//
//				struct offsets_t {
//					size_t dt;
//					size_t mesh;
//					size_t vtx;
//					size_t idx;
//				};
//
//				offsets_t offsets;
//				offsets.dt   = sizeof(offsets_t);
//				offsets.mesh = offsets.dt + sizeof(hd2::meshinfo_datatype_t::data_t);
//				offsets.vtx  = offsets.mesh + mesh.size();
//				offsets.idx  = offsets.vtx + (datatype.vertices_stride() * mesh.modeldata().vertices());
//
//				fwrite(&offsets, sizeof(offsets_t), 1, file.get());
//
//				// Write datatype information.
//				fwrite(datatype._ptr, sizeof(hd2::meshinfo_datatype_t::data_t), 1, file.get());
//
//				// Write mesh information.
//				fwrite(&mesh, mesh.size(), 1, file.get());
//
//				// Write vertex information.
//				fwrite(mesh_ptr + datatype.vertices_offset() + mesh.modeldata().vertices_offset() * datatype.vertices_stride(), datatype.vertices_stride(), mesh.modeldata().vertices(), file.get());
//
//				// Write index information.
//				fwrite(mesh_ptr + datatype.indices_offset() + mesh.modeldata().indices_offset() * datatype.indices_stride(), datatype.indices_stride(), mesh.modeldata().indices(), file.get());
//			}
//
//			{ // Write model file.
//				std::string file_name;
//				file_t      file;
//				{
//					{
//						const char*       format = "%s/%08" PRIu32 ".obj";
//						std::vector<char> buf;
//						size_t            bufsz = snprintf(nullptr, 0, format, output_path.u8string().c_str(), idx);
//						buf.resize(bufsz + 1);
//						snprintf(buf.data(), buf.size(), format, output_path.u8string().c_str(), idx);
//						file_name = {buf.data(), buf.data() + buf.size() - 1};
//					}
//					auto file_handle = fopen(file_name.c_str(), "w+b");
//					file.reset(file_handle);
//					;
//				}
//
//				// Write overall data.
//				fprintf(file.get(), "o %s\n", file_name.c_str());
//				fprintf(file.get(), "g %s\n", file_name.c_str());
//				fprintf(file.get(), "s 0\n");
//				fflush(file.get());
//
//				{ // Write all vertices.
//					std::ptrdiff_t ptr         = datatype.vertices_offset() + mesh.modeldata().vertices_offset() * datatype.vertices_stride();
//					std::ptrdiff_t max_ptr     = ptr + mesh.modeldata().vertices() * datatype.vertices_stride();
//					std::ptrdiff_t abs_max_ptr = datatype.vertices_offset() + datatype.vertices_size();
//
//					bool is_error_ptr = false;
//					if (ptr > abs_max_ptr) {
//						printf("  ptr > abs_max_ptr\n");
//						is_error_ptr = true;
//					} else if (max_ptr > abs_max_ptr) {
//						printf("  max_ptr > abs_max_ptr\n");
//						is_error_ptr = true;
//					}
//					if (is_error_ptr) {
//						printf("  ptr =         %08zx", ptr);
//						printf("  max_ptr =     %08zx", max_ptr);
//						printf("  abs_max_ptr = %08zx", abs_max_ptr);
//						continue;
//					}
//
//					for (size_t vtx = 0; vtx < mesh.modeldata().vertices(); vtx++) {
//						auto vtx_ptr = reinterpret_cast<uint8_t const*>(mesh_ptr + ptr + datatype.vertices_stride() * vtx);
//						fprintf(file.get(), "# %zu\n", vtx);
//
//						for (size_t el = 0; el < datatype.elements(); el++) {
//							auto element = datatype.element(el);
//
//							std::string format_str;
//							switch (element.type) {
//							case hd2::vertex_element_type::_00_position: {
//								switch (element.format) {
//								case hd2::vertex_element_format::f16vec2: {
//									half const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
//									fprintf(file.get(), "v %#16.8g %#16.8g %#16.8g\n", (float)vec_ptr[0] * hd2::mesh_scale, (float)vec_ptr[1] * hd2::mesh_scale, 0.f);
//									break;
//								}
//								case hd2::vertex_element_format::_01_implied_float2: {
//									float const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
//									fprintf(file.get(), "v %#16.8g %#16.8g %#16.8g\n", vec_ptr[0] * hd2::mesh_scale, vec_ptr[1] * hd2::mesh_scale, 0.f);
//									break;
//								}
//
//								case hd2::vertex_element_format::_1E_implied_half3: {
//									half const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
//									fprintf(file.get(), "v %#16.8g %#16.8g %#16.8g\n", (float)vec_ptr[0] * hd2::mesh_scale, (float)vec_ptr[1] * hd2::mesh_scale, (float)vec_ptr[2] * hd2::mesh_scale);
//									break;
//								}
//								case hd2::vertex_element_format::f32vec3: {
//									float const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
//									fprintf(file.get(), "v %#16.8g %#16.8g %#16.8g\n", vec_ptr[0] * hd2::mesh_scale, vec_ptr[1] * hd2::mesh_scale, vec_ptr[2] * hd2::mesh_scale);
//									break;
//								}
//
//								default:
//									fprintf(file.get(), "# ERROR: type+format %08x, %08x, %08x, %08x, %08x is unknown\n", element.type, element.format, element.layer, element.__unk00, element.__unk01);
//									fprintf(file.get(), "#  DUMP: ");
//									for (size_t n = 0; n < hd2::vertex_element_format_size(element.format); n++) {
//										fprintf(file.get(), "%02X", *(vtx_ptr + n));
//									}
//									fprintf(file.get(), "\n");
//								}
//								break;
//							}
//
//							case hd2::vertex_element_type::_01_color: {
//								switch (element.format) {
//								case hd2::vertex_element_format::_1E_implied_half3: {
//									half const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
//									fprintf(file.get(), "# vc %#16.8g %#16.8g %#16.8g\n", (float)vec_ptr[0], (float)vec_ptr[1], (float)vec_ptr[2]);
//									break;
//								}
//								case hd2::vertex_element_format::f32vec3: {
//									float const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
//									fprintf(file.get(), "# vc %#16.8g %#16.8g %#16.8g\n", vec_ptr[0], vec_ptr[1], vec_ptr[2]);
//									break;
//								}
//
//								case hd2::vertex_element_format::f16vec4: {
//									half const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
//									fprintf(file.get(), "# vc %#16.8g %#16.8g %#16.8g %#16.8g\n", (float)vec_ptr[0], (float)vec_ptr[1], (float)vec_ptr[2], (float)vec_ptr[3]);
//									break;
//								}
//								case hd2::vertex_element_format::_03_implied_float4: {
//									float const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
//									fprintf(file.get(), "# vc %#16.8g %#16.8g %#16.8g %#16.8g\n", vec_ptr[0], vec_ptr[1], vec_ptr[2], vec_ptr[3]);
//									break;
//								}
//
//								default:
//									fprintf(file.get(), "# ERROR: type+format %08x, %08x, %08x, %08x, %08x is unknown\n", element.type, element.format, element.layer, element.__unk00, element.__unk01);
//									fprintf(file.get(), "#  DUMP: ");
//									for (size_t n = 0; n < hd2::vertex_element_format_size(element.format); n++) {
//										fprintf(file.get(), "%02X", *(vtx_ptr + n));
//									}
//									fprintf(file.get(), "\n");
//								}
//
//								break;
//							}
//
//							case hd2::vertex_element_type::_04_texcoord: {
//								if (element.layer != 0) {
//									fprintf(file.get(), "# ");
//								}
//
//								switch (element.format) {
//								case hd2::vertex_element_format::f16vec2: {
//									half const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
//									fprintf(file.get(), "vt %#16.8g %#16.8g\n", (float)vec_ptr[0], (float)vec_ptr[1]);
//									break;
//								}
//								case hd2::vertex_element_format::_01_implied_float2: {
//									float const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
//									fprintf(file.get(), "vt %#16.8g %#16.8g\n", (float)vec_ptr[0], (float)vec_ptr[1]);
//									break;
//								}
//
//								case hd2::vertex_element_format::_1E_implied_half3: {
//									half const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
//									fprintf(file.get(), "vt %#16.8g %#16.8g %#16.8g\n", (float)vec_ptr[0], (float)vec_ptr[1], (float)vec_ptr[2]);
//									break;
//								}
//								case hd2::vertex_element_format::f32vec3: {
//									float const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
//									fprintf(file.get(), "vt %#16.8g %#16.8g %#16.8g\n", (float)vec_ptr[0], (float)vec_ptr[1], (float)vec_ptr[2]);
//									break;
//								}
//
//								default:
//									fprintf(file.get(), "# ERROR: type+format %08x, %08x, %08x, %08x, %08x is unknown\n", element.type, element.format, element.layer, element.__unk00, element.__unk01);
//									fprintf(file.get(), "#  DUMP: ");
//									for (size_t n = 0; n < hd2::vertex_element_format_size(element.format); n++) {
//										fprintf(file.get(), "%02X", *(vtx_ptr + n));
//									}
//									fprintf(file.get(), "\n");
//								}
//								break;
//							}
//
//							case hd2::vertex_element_type::_07_normal: {
//								if (element.layer != 0) {
//									fprintf(file.get(), "# ");
//								}
//
//								switch (element.format) {
//								case hd2::vertex_element_format::_1E_implied_half3: {
//									half const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
//									fprintf(file.get(), "vn %#16.8g %#16.8g %#16.8g\n", (float)vec_ptr[0], (float)vec_ptr[1], (float)vec_ptr[2]);
//									break;
//								}
//								case hd2::vertex_element_format::f32vec3: {
//									float const* vec_ptr = reinterpret_cast<decltype(vec_ptr)>(vtx_ptr);
//									fprintf(file.get(), "vn %#16.8g %#16.8g %#16.8g\n", (float)vec_ptr[0], (float)vec_ptr[1], (float)vec_ptr[2]);
//									break;
//								}
//
//								default:
//									fprintf(file.get(), "# ERROR: type+format %08x, %08x, %08x, %08x, %08x is unknown\n", element.type, element.format, element.layer, element.__unk00, element.__unk01);
//									fprintf(file.get(), "#  DUMP: ");
//									for (size_t n = 0; n < hd2::vertex_element_format_size(element.format); n++) {
//										fprintf(file.get(), "%02X", *(vtx_ptr + n));
//									}
//									fprintf(file.get(), "\n");
//								}
//								break;
//							}
//
//							default:
//								fprintf(file.get(), "# ERROR: type %08x, %08x, %08x, %08x, %08x is unknown\n", element.type, element.format, element.layer, element.__unk00, element.__unk01);
//								fprintf(file.get(), "#  DUMP: ");
//								for (size_t n = 0; n < hd2::vertex_element_format_size(element.format); n++) {
//									fprintf(file.get(), "%02X", *(vtx_ptr + n));
//								}
//								fprintf(file.get(), "\n");
//							}
//
//							vtx_ptr += hd2::vertex_element_format_size(element.format);
//						}
//					}
//				}
//				fflush(file.get());
//
//				{ // Write all faces.
//					std::ptrdiff_t ptr         = datatype.indices_offset() + mesh.modeldata().indices_offset() * datatype.indices_stride();
//					std::ptrdiff_t max_ptr     = ptr + mesh.modeldata().indices() * datatype.indices_stride();
//					std::ptrdiff_t abs_max_ptr = datatype.indices_offset() + datatype.indices_size();
//
//					bool is_error_ptr = false;
//					if (ptr > abs_max_ptr) {
//						printf("  ptr > abs_max_ptr\n");
//						is_error_ptr = true;
//					} else if (max_ptr > abs_max_ptr) {
//						printf("  max_ptr > abs_max_ptr\n");
//						is_error_ptr = true;
//					}
//					if (is_error_ptr) {
//						printf("  ptr =         %08zx", ptr);
//						printf("  max_ptr =     %08zx", max_ptr);
//						printf("  abs_max_ptr = %08zx", abs_max_ptr);
//						continue;
//					}
//
//					for (size_t idx = 0; idx < mesh.modeldata().indices(); idx += 3) {
//						auto idx_ptr = reinterpret_cast<uint8_t const*>(mesh_ptr + ptr + datatype.indices_stride() * idx);
//						fprintf(file.get(), "# %zu\n", idx);
//
//						switch (datatype.indices_stride()) {
//						case 1: {
//							auto v0 = *reinterpret_cast<uint8_t const*>(idx_ptr + datatype.indices_stride() * 0) + 1;
//							auto v1 = *reinterpret_cast<uint8_t const*>(idx_ptr + datatype.indices_stride() * 1) + 1;
//							auto v2 = *reinterpret_cast<uint8_t const*>(idx_ptr + datatype.indices_stride() * 2) + 1;
//							fprintf(file.get(), "f %" PRIu8 "/%" PRIu8 "/%" PRIu8 " %" PRIu8 "/%" PRIu8 "/%" PRIu8 " %" PRIu8 "/%" PRIu8 "/%" PRIu8 "\n", v0, v0, v0, v1, v1, v1, v2, v2, v2);
//							break;
//						}
//						case 2: {
//							auto v0 = *reinterpret_cast<uint16_t const*>(idx_ptr + datatype.indices_stride() * 0) + 1;
//							auto v1 = *reinterpret_cast<uint16_t const*>(idx_ptr + datatype.indices_stride() * 1) + 1;
//							auto v2 = *reinterpret_cast<uint16_t const*>(idx_ptr + datatype.indices_stride() * 2) + 1;
//							fprintf(file.get(), "f %" PRIu16 "/%" PRIu16 "/%" PRIu16 " %" PRIu16 "/%" PRIu16 "/%" PRIu16 " %" PRIu16 "/%" PRIu16 "/%" PRIu16 "\n", v0, v0, v0, v1, v1, v1, v2, v2, v2);
//							break;
//						}
//						case 4: {
//							auto v0 = *reinterpret_cast<uint32_t const*>(idx_ptr + datatype.indices_stride() * 0) + 1;
//							auto v1 = *reinterpret_cast<uint32_t const*>(idx_ptr + datatype.indices_stride() * 1) + 1;
//							auto v2 = *reinterpret_cast<uint32_t const*>(idx_ptr + datatype.indices_stride() * 2) + 1;
//							fprintf(file.get(), "f %" PRIu32 "/%" PRIu32 "/%" PRIu32 " %" PRIu32 "/%" PRIu32 "/%" PRIu32 " %" PRIu32 "/%" PRIu32 "/%" PRIu32 "\n", v0, v0, v0, v1, v1, v1, v2, v2, v2);
//							break;
//						}
//						case 8: {
//							auto v0 = *reinterpret_cast<uint64_t const*>(idx_ptr + datatype.indices_stride() * 0) + 1;
//							auto v1 = *reinterpret_cast<uint64_t const*>(idx_ptr + datatype.indices_stride() * 1) + 1;
//							auto v2 = *reinterpret_cast<uint64_t const*>(idx_ptr + datatype.indices_stride() * 2) + 1;
//							fprintf(file.get(), "f %" PRIu64 "/%" PRIu64 "/%" PRIu64 " %" PRIu64 "/%" PRIu64 "/%" PRIu64 " %" PRIu64 "/%" PRIu64 "/%" PRIu64 "\n", v0, v0, v0, v1, v1, v1, v2, v2, v2);
//							break;
//						}
//						}
//					}
//				}
//				fflush(file.get());
//			}
//		}
//	}
//
//	std::cout << std::endl;
//	*/
