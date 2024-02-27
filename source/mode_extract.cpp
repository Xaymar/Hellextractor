// Copyright 2024 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <set>
#include <unordered_set>
#include "endian.h"
#include "hash_db.hpp"
#include "hd2_data.hpp"
#include "main.hpp"
#include "string_printf.hpp"

static std::string_view constexpr name = "extract";
static std::string_view constexpr help = "Extract files from data, stream and gpu_resources files";

std::set<std::filesystem::path> enumerate_files(std::filesystem::path const& path, std::function<bool(std::filesystem::path const&)> filter)
{
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

int32_t mode_extract(std::vector<std::string> const& args)
{
	bool show_help = false;
	if (args.size() == 1) {
		show_help = true;
	} else {
		if ((args[1] == "-h") || (args[1] == "--help")) {
			show_help = true;
		}
	}
	if (show_help) {
		auto self = std::filesystem::path(args[0]).filename();
		std::cout << self.generic_string() << " " << name << " [options] data_file_or_path [...]" << std::endl;
		std::cout << "Exports all data from the given " << std::endl;
		std::cout << std::endl;
		std::cout << "Options" << std::endl;
		std::cout << "  -h, --help            Show this help" << std::endl;
		std::cout << "  -o, --output <path>   Set output directory" << std::endl;
		std::cout << "  -i, --input <regex>   Only include input files matching the regular expression. Default is to exclude any file with an extension." << std::endl;
		std::cout << "  -f, --filter <regex>  Only include output files matching the regular expression. Will be run after any translation tables have been applied." << std::endl;
		std::cout << "  -t, --types <path>    Add a database file to the Type Hash translation table. Will search the most recently added one first, then continue until there's none left, then search the Strings Hash translation tables." << std::endl;
		std::cout << "  -n, --names <path>    Add a database file to the Name Hash translation table. Will search the most recently added one first, then continue until there's none left, then search the Strings Hash translation tables." << std::endl;
		std::cout << "  -s, --strings <path>  Add a database file to the String Hash translation table." << std::endl;
		std::cout << "  -d, --dry-run         Don't actually do anything." << std::endl;
		std::cout << std::endl;
		return 1;
	}

	std::optional<std::regex>                 input_filter;
	std::set<std::filesystem::path>           input_paths;
	std::optional<std::regex>                 output_filter;
	std::filesystem::path                     output_path = std::filesystem::absolute(std::filesystem::path(args[0])).parent_path();
	std::unordered_set<std::filesystem::path> type_paths;
	std::unordered_set<std::filesystem::path> name_paths;
	std::unordered_set<std::filesystem::path> string_paths;
	bool                                      dryrun = false;

	// Figure out what is what.
	for (size_t edx = args.size(), idx = 1; idx < edx; ++idx) {
		auto arg = args[idx];
		if (arg[0] == '-') {
			if ((arg == "-o") || (arg == "--output")) {
				if ((idx + 1) < edx) {
					output_path = std::filesystem::absolute(args[idx + 1]);
					++idx;
				} else {
					std::cerr << "Expected path, got end of line." << std::endl;
					return 1;
				}
			} else if ((arg == "-i") || (arg == "--input")) {
				if ((idx + 1) < edx) {
					input_filter = args[idx + 1];
					++idx;
				} else {
					std::cerr << "Expected regex, got end of line." << std::endl;
					return 1;
				}
			} else if ((arg == "-f") || (arg == "--filter")) {
				if ((idx + 1) < edx) {
					output_filter = args[idx + 1];
					++idx;
				} else {
					std::cerr << "Expected regex, got end of line." << std::endl;
					return 1;
				}
			} else if ((arg == "-t") || (arg == "--types")) {
				if ((idx + 1) < edx) {
					type_paths.insert(std::filesystem::absolute(args[idx + 1]));
					++idx;
				} else {
					std::cerr << "Expected path, got end of line." << std::endl;
					return 1;
				}
			} else if ((arg == "-n") || (arg == "--names")) {
				if ((idx + 1) < edx) {
					name_paths.insert(std::filesystem::absolute(args[idx + 1]));
					++idx;
				} else {
					std::cerr << "Expected path, got end of line." << std::endl;
					return 1;
				}
			} else if ((arg == "-s") || (arg == "--strings")) {
				if ((idx + 1) < edx) {
					string_paths.insert(std::filesystem::absolute(args[idx + 1]));
					++idx;
				} else {
					std::cerr << "Expected path, got end of line." << std::endl;
					return 1;
				}
			} else if ((arg == "-d") || (arg == "--dry-run")) {
				dryrun = true;
				//} else if ((arg == "-") || (arg == "--")) {
			} else {
				std::cerr << "Unrecognized argument: " << arg << std::endl;
				return 1;
			}
		} else {
			input_paths.insert(std::filesystem::path(arg));
		}
	}

	// Load translation tables...
	std::list<hellextractor::hash_db> typedbs;
	std::list<hellextractor::hash_db> namedbs;
	std::list<hellextractor::hash_db> strings;
	{
		std::cout << "Loading translation tables..." << std::endl;
		for (auto const& path : type_paths) {
			std::cout << "    " << path.generic_string() << std::endl;
			typedbs.emplace_back(path);
		}
		for (auto const& path : name_paths) {
			std::cout << "    " << path.generic_string() << std::endl;
			namedbs.emplace_back(path);
		}
		for (auto const& path : string_paths) {
			std::cout << "    " << path.generic_string() << std::endl;
			strings.emplace_back(path);
		}
	}

	// Log some information for the end user.
	std::cout << "Writing files to: " << output_path.generic_string() << std::endl;
	if (dryrun) {
		std::cout << "This is a dry run, no files will be written. It's all pretend!" << std::endl;
	}

	std::cout << std::endl;

	{ // Filter input path either by default filter, or by user specified
		std::set<std::filesystem::path> paths;
		for (auto const& path : input_paths) {
			paths.merge(enumerate_files(path, [&input_filter](std::filesystem::path const& path) {
				if (input_filter.has_value()) {
					auto name = path.generic_string();
					return std::regex_match(name, input_filter.value());
				} else {
					return !path.has_extension();
				}
				return true;
			}));
		}
		input_paths = paths;
	}

	// We'll load all containers into memory at once, which should ideally only require virtual memory.
	std::cout << "Loading " << input_paths.size() << " containers..." << std::endl;
	std::list<hd2::data> containers;
	for (auto const& path : input_paths) {
		try {
			containers.emplace_back(path);
			std::cout << "    " << path.generic_string() << std::endl;
		} catch (std::exception const& ex) {
			std::cout << "    ERROR " << path.generic_string() << ": " << ex.what() << std::endl;
			continue;
		}
	}
	std::cout << std::endl;

	// Merge all containers to get a full view of what we really have.
	std::map<std::pair<hd2::file_id_t, hd2::file_type>, std::pair<hd2::data const&, size_t>> files;
	for (auto const& cont : containers) {
		for (size_t i = 0; i < cont.files(); i++) {
			// In all testing, files that existed multiple times had zero differences. So this is safe to do.
			auto const& file = cont.file(i);
			files.try_emplace(std::pair<hd2::file_id_t, hd2::file_type>{file.id, file.type}, std::pair<hd2::data const&, size_t>{cont, i});
		}
	}
	std::cout << "Found " << files.size() << " files." << std::endl;

	// Export files (if not in dryrun mode)
	if (!dryrun) {
		std::filesystem::create_directories(output_path);
	}
	for (auto const& file : files) {
		auto        index = file.second.second;
		auto const& cont  = file.second.first;

		// Match the type with the type databases.
		std::string file_type;
		for (auto& db : typedbs) {
			if (auto tv = db.strings().find(static_cast<uint64_t>(file.first.second)); db.strings().end() != tv) {
				file_type = tv->second;
				break;
			}
		}
		if (file_type.empty()) {
			for (auto& db : strings) {
				if (auto tv = db.strings().find(static_cast<uint64_t>(file.first.second)); db.strings().end() != tv) {
					file_type = tv->second;
					break;
				}
			}
		}
		if (file_type.empty()) {
			file_type = string_printf("%016" PRIx64, file.first.second);
		}

		// Match the name with the name databases.
		std::string file_name;
		for (auto& db : namedbs) {
			if (auto tv = db.strings().find(static_cast<uint64_t>(file.first.first)); db.strings().end() != tv) {
				file_name = tv->second;
				break;
			}
		}
		if (file_name.empty()) {
			for (auto& db : strings) {
				if (auto tv = db.strings().find(static_cast<uint64_t>(file.first.first)); db.strings().end() != tv) {
					file_name = tv->second;
					break;
				}
			}
		}
		if (file_name.empty()) {
			file_name = string_printf("%016" PRIx64, file.first.first);
		}

		// Generate a proper file path.
		std::filesystem::path file = std::filesystem::path(file_name).replace_extension(file_type);
		std::filesystem::path path = output_path / file;

		// If the user provided a filter, use it now.
		if (output_filter.has_value()) {
			if (!std::regex_match(file.generic_string(), output_filter.value())) {
				//std::cout << "      Skipped" << std::endl;
				continue;
			}
		}

		// Only print files that are actually exported.
		std::cout << "    " << file.generic_string() << std::endl;

		if (!dryrun) {
			std::filesystem::create_directories(path.parent_path());

			std::ofstream filestream{path, std::ios::trunc | std::ios::binary | std::ios::out};
			if (!filestream || filestream.bad() || !filestream.is_open()) {
				throw std::runtime_error(string_printf("Failed to export '%s'.", file.generic_string().c_str()));
			}
			filestream.flush();

			if (cont.has_meta(index)) {
				std::cout << "        Writing meta section..." << std::endl;
				filestream.write(reinterpret_cast<char const*>(cont.meta_data(index)), cont.meta_size(index));
				filestream.flush();
			}

			if (cont.has_stream(index)) {
				std::cout << "        Writing stream section..." << std::endl;
				filestream.write(reinterpret_cast<char const*>(cont.stream(index)), cont.stream_size(index));
				filestream.flush();
			}

			if (cont.has_gpu(index)) {
				std::cout << "        Writing gpu section..." << std::endl;
				filestream.write(reinterpret_cast<char const*>(cont.gpu(index)), cont.gpu_size(index));
				filestream.flush();
			}

			filestream.close();
		}
	}

	return 0;
}
static auto instance = hellextractor::mode(std::string(name), std::string(help), mode_extract);
