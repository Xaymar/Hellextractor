//
//// Figure out all the files we need to process.
//auto filter = [](std::filesystem::path const& path) {
//	if (path.has_extension()) {
//		return false;
//	}
//	return true;
//};
//std::set<std::filesystem::path> real_paths;
//for (auto const& path : unparsed) {
//	real_paths.merge(enumerate_files(path, filter));
//}
//
//std::cout << string_printf("%zu Input Files: ", real_paths.size()) << std::endl;
//for (auto const& path : real_paths) {
//	std::cout << string_printf("    %s", path.generic_u8string().c_str()) << std::endl;
//}
//
//// Load all the containers into memory. This should ideally only happen with virtual memory.
//std::list<hd2::data> containers;
//for (auto const& path : real_paths) {
//	containers.emplace_back(path);
//}
//
//// Create a full tree of {type, id}, {container, index}, so we don't waste time extracting things multiple times.
//std::map<std::pair<hd2::file_id_t, hd2::file_type>, std::pair<hd2::data const&, size_t>> files;
//for (auto const& cont : containers) {
//	for (size_t i = 0; i < cont.files(); i++) {
//		auto const& file = cont.file(i);
//		files.try_emplace(std::pair<hd2::file_id_t, hd2::file_type>{file.id, file.type}, std::pair<hd2::data const&, size_t>{cont, i});
//		// Files may exist multiple times, but they are usually exactly identical.
//	}
//}
//std::cout << string_printf("%zu Output Files: ", files.size()) << std::endl;
//for (auto const& file : files) {
//	std::cout << string_printf("    %016" PRIx64 " %016" PRIx64, htobe64((uint64_t)file.first.first), htobe64((uint64_t)file.first.second)) << std::endl;
//}
//
//// Export all discovered files.
//std::cout << "Exporting: " << std::endl;
//std::filesystem::create_directories(output);
//for (auto const& kv : files) {
//	auto const& cont  = kv.second.first;
//	auto const& file  = kv.second.first.file(kv.second.second);
//	auto        index = kv.second.second;
//
//	std::string ext = std::string{hd2::file_extension_from_type(file.type)};
//	if (ext.length() == 0) {
//		ext = string_printf("%016" PRIx64, htobe64((uint64_t)file.type));
//	}
//
//	std::filesystem::path name = output / string_printf("%016" PRIx64 "%s", htobe64((uint64_t)file.id), ext.c_str());
//
//	if (!std::filesystem::exists(name)) {
//		std::cout << string_printf("    [%08" PRIX32 ":%08" PRIX32 "] [%08" PRIX32 ":%08" PRIX32 "] [%08" PRIX32 ":%08" PRIX32 "] %s ...", file.offset, file.size, file.stream_offset, file.stream_size, file.gpu_offset, file.gpu_size, std::filesystem::relative(name, output).generic_string().c_str());
//
//		std::ofstream filestream{name, std::ios::trunc | std::ios::binary | std::ios::out};
//		if (!filestream || filestream.bad() || !filestream.is_open()) {
//			throw std::runtime_error(string_printf("Failed to export '%s'.", name.generic_u8string().c_str()));
//		}
//		filestream.flush();
//
//		if (cont.has_meta(index)) {
//			filestream.write(reinterpret_cast<char const*>(cont.meta_data(index)), cont.meta_size(index));
//		}
//		filestream.flush();
//
//		if (cont.has_stream(index)) {
//			filestream.write(reinterpret_cast<char const*>(cont.stream(index)), cont.stream_size(index));
//		}
//		filestream.flush();
//
//		if (cont.has_gpu(index)) {
//			filestream.write(reinterpret_cast<char const*>(cont.gpu(index)), cont.gpu_size(index));
//		}
//		filestream.flush();
//
//		filestream.close();
//
//		std::cout << " Done." << std::endl;
//	} else {
//		std::cout << string_printf("    Skipped %s.", std::filesystem::relative(name, output).generic_string().c_str()) << std::endl;
//	}
//}
