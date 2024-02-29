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
#include "converter.hpp"
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
		std::cout << "  -r, --rename          Rename/Delete files with older or untranslated names or types." << std::endl;
		std::cout << "  -q, --quiet           Decrease verbosity of output." << std::endl;
		std::cout << "  -v, --verbose         Increase verbosity of output." << std::endl;
		std::cout << "  -x, --index <path>    Generate an hash -> file index (csv) for use in external tools." << std::endl;
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
	bool                                      is_dry    = false;
	bool                                      rename    = false;
	int32_t                                   verbosity = 0;
	std::optional<std::filesystem::path>      index_path;

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
				is_dry = true;
			} else if ((arg == "-r") || (arg == "--rename")) {
				rename = true;
			} else if ((arg == "-v") || (arg == "--verbose")) {
				verbosity++;
			} else if ((arg == "-q") || (arg == "--quiet")) {
				verbosity--;
			} else if ((arg == "-x") || (arg == "--index")) {
				if ((idx + 1) < edx) {
					index_path = std::filesystem::absolute(args[idx + 1]);
					++idx;
				} else {
					std::cerr << "Expected path, got end of line." << std::endl;
					return 1;
				}
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
		if (verbosity >= 0)
			std::cout << "Loading translation tables..." << std::endl;
		for (auto const& path : type_paths) {
			if (verbosity >= 1)
				std::cout << "    " << path.generic_string() << std::endl;
			typedbs.emplace_back(path);
		}
		for (auto const& path : name_paths) {
			if (verbosity >= 1)
				std::cout << "    " << path.generic_string() << std::endl;
			namedbs.emplace_back(path);
		}
		for (auto const& path : string_paths) {
			if (verbosity >= 1)
				std::cout << "    " << path.generic_string() << std::endl;
			strings.emplace_back(path);
		}
	}

	// Log some information for the end user.
	if (verbosity >= 0)
		std::cout << "Writing files to: " << output_path.generic_string() << std::endl;
	if ((verbosity >= 0) && index_path.has_value())
		std::cout << "Writing index to: " << index_path.value().generic_string() << std::endl;

	if (is_dry) {
		if (verbosity >= -1)
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
	if (verbosity >= 0)
		std::cout << "Loading " << input_paths.size() << " containers..." << std::endl;
	std::list<helldivers2::data> containers;
	for (auto const& path : input_paths) {
		try {
			containers.emplace_back(path);
			if (verbosity >= 1)
				std::cout << "    " << path.generic_string() << std::endl;
		} catch (std::exception const& ex) {
			std::cerr << "Error loading '" << path.generic_string() << "': " << ex.what() << std::endl;
			continue;
		}
	}

	// Merge all containers to get a full view of what we really have.
	typedef std::pair<helldivers2::file_id_t, helldivers2::file_type_t> key_t;
	typedef helldivers2::data::meta_t                                   data_t;
	std::map<key_t, data_t>                                             files;
	for (auto const& cont : containers) {
		for (size_t i = 0; i < cont.files(); i++) {
			// In all testing, files that existed multiple times had zero differences. So this is safe to do.
			auto file = cont.meta(i);
			files.try_emplace(key_t{file.file.id, file.file.type}, file);
		}
	}
	if (verbosity >= 0)
		std::cout << "Found " << files.size() << " files." << std::endl;

	// Export files (if not in dry run mode)
	size_t stats_total    = 0;
	size_t stats_written  = 0;
	size_t stats_renamed  = 0;
	size_t stats_removed  = 0;
	size_t stats_skipped  = 0;
	size_t stats_filtered = 0;
	size_t stats_names    = 0;
	size_t stats_types    = 0;
	if (!is_dry) {
		std::filesystem::create_directories(output_path);
	}
	for (auto const& file : files) {
		auto meta = file.second;
		stats_total++;

		auto translations = [](uint64_t hash, std::list<hellextractor::hash_db>& primary, std::list<hellextractor::hash_db>& secondary) {
			std::vector<std::string> translations;

			for (auto& db : primary) {
				if (auto tv = db.strings().find(hash); db.strings().end() != tv) {
					if (std::find(translations.cbegin(), translations.cend(), tv->second) == translations.end()) {
						translations.push_back(tv->second);
					}
				}
			}
			for (auto& db : secondary) {
				if (auto tv = db.strings().find(hash); db.strings().end() != tv) {
					if (std::find(translations.cbegin(), translations.cend(), tv->second) == translations.end()) {
						translations.push_back(tv->second);
					}
				}
			}

			return translations;
		};

		// Match the name with the name databases.
		auto file_names = translations(static_cast<uint64_t>(meta.file.id), namedbs, strings);
		if (file_names.size() > 0) {
			++stats_names;
		}
		file_names.emplace_back(string_printf("%016" PRIx64, htobe64((uint64_t)meta.file.id)));
		file_names.emplace_back(string_printf("%016" PRIx64, htole64((uint64_t)meta.file.id)));

		// Match the type with the type databases.
		auto file_types = translations(static_cast<uint64_t>(meta.file.type), typedbs, strings);
		if (file_types.size() > 0) {
			++stats_types;
		}
		file_types.emplace_back(string_printf("%016" PRIx64, htobe64((uint64_t)meta.file.type)));
		file_types.emplace_back(string_printf("%016" PRIx64, htole64((uint64_t)meta.file.type)));

		// Generate all permutations.
		std::vector<std::pair<std::string, std::string>> permutations;
		for (auto const& fn : file_names) {
			for (auto const& ft : file_types) {
				permutations.emplace_back(fn, ft);
			}
		}

		// Generate a proper file path.
		auto base_file_name = std::filesystem::path(permutations[0].first).replace_extension(permutations[0].second);
		auto base_file_path = output_path / base_file_name;

		if (!is_dry) {
			std::filesystem::create_directories(base_file_path.parent_path());
		}

		// Try to create a converter for the file type.
		auto converter = hellextractor::converter::registry::find(meta);
		if (converter) {
			auto outputs = converter->outputs();

			stats_total--;
			stats_total += outputs.size();

			// Remove pre-conversion data.
			if (std::filesystem::exists(base_file_path)) {
				if (verbosity >= 0)
					std::cout << "  d " << base_file_name.generic_string() << std::endl;
				if (!is_dry) {
					std::filesystem::remove(base_file_path);
				}
				stats_removed++;
			}

			// Go through each output.
			for (auto output : outputs) {
				bool do_export = true;

				// Figure out the file name and absolute path.
				auto file_name = std::filesystem::path(permutations[0].first).replace_extension(output.second.second);
				auto file_path = output_path / file_name;

				if (verbosity >= 1)
					std::cout << "  " << file_name.generic_string() << std::endl;

				// If the user provided a filter, use it now to exclude files they may not want.
				if (output_filter.has_value() && (!std::regex_match(file_name.generic_string(), output_filter.value()))) {
					if (verbosity >= 1)
						std::cout << "  f " << file_name.generic_string() << std::endl;
					stats_filtered++;
					continue;
				}

				// Check if the existing file needs to be exported again.
				bool file_exists = std::filesystem::exists(file_path);
				if (file_exists) {
					do_export = (std::filesystem::file_size(file_path) != output.second.first);
				}

				// Rename or delete older files if the user requested it.
				if (rename) {
					auto renamedeleter = [&file_name, &file_path, &do_export, &stats_renamed, &stats_removed, &is_dry, &verbosity, &output_path, &output, &file_exists](std::filesystem::path path) {
						auto old_file_name = path;
						auto old_file_path = output_path / old_file_name;

						// Ensure we do not try to rename or delete the main file.
						if (old_file_path == file_path) {
							return;
						}

						// If the old file exists...
						if (std::filesystem::exists(old_file_path)) {
							// Then attempt to rename the file if it is the correct size, and we need to export, and the target doesn't exist.
							if ((std::filesystem::file_size(old_file_path) == output.second.first) && do_export && !file_exists) {
								if (verbosity >= 0)
									std::cout << "  r " << old_file_name.generic_string() << " -> " << file_name.generic_string() << " <- " << std::endl;
								if (!is_dry) {
									std::filesystem::rename(old_file_path, file_path);
								}
								do_export   = false; // This automatically handles the case where we have multiple files.
								file_exists = true;
								stats_renamed++;
							} else {
								if (verbosity >= 0)
									std::cout << "  d " << old_file_name.generic_string() << std::endl;
								if (!is_dry) {
									std::filesystem::remove(old_file_path);
								}
								stats_removed++;
							}
						}
					};

					// Go through all permutations.
					for (size_t idx = 1; idx < permutations.size(); idx++) {
						renamedeleter(std::filesystem::path(permutations[idx].first).concat(".").concat(permutations[idx].second).concat(".").concat(output.second.second));
					}
				}

				// Finally, if we still need to export things, do so.
				if (do_export) {
					if (verbosity >= 0)
						std::cout << "  e " << file_name.generic_string() << std::endl;

					if (!is_dry) {
						converter->extract(output.first, file_path);
					}
					stats_written++;
				} else {
					if (verbosity >= 1)
						std::cout << "  s " << base_file_name.generic_string() << std::endl;
					stats_skipped++;
				}
			}
		} else {
			bool   needs_export = true;
			bool   had_rename   = false;
			size_t data_size    = (meta.main_size + meta.gpu_size + meta.stream_size);

			if (verbosity >= 1)
				std::cout << "  " << base_file_name.generic_string() << std::endl;

			// If the user provided a filter, use it now.
			if (output_filter.has_value()) {
				if (!std::regex_match(base_file_name.generic_string(), output_filter.value())) {
					if (verbosity >= 1)
						std::cout << "  f " << base_file_name.generic_string() << std::endl;
					stats_filtered++;
					continue;
				}
			}

			// Check if the target file is a different size.
			bool file_exists = std::filesystem::exists(base_file_path);
			if (file_exists) {
				needs_export = std::filesystem::file_size(base_file_path) != data_size;
			}

			// Rename any existing files.
			if (rename) {
				for (size_t idx = 1; idx < permutations.size(); idx++) {
					auto lfile = std::filesystem::path(permutations[idx].first).replace_extension(permutations[idx].second);
					auto lpath = output_path / lfile;

					if (lfile == base_file_name) {
						// This should be impossible, but lets deal with it anyway. We don't want weird behavior.
						continue;
					}

					if (std::filesystem::exists(lpath)) {
						if (needs_export && (std::filesystem::file_size(lpath) == data_size) && !file_exists) {
							if (verbosity >= 0)
								std::cout << "  r " << base_file_name.generic_string() << " <- " << lfile.generic_string() << std::endl;
							if (!is_dry) {
								std::filesystem::rename(lpath, base_file_path);
							}
							needs_export = false;
							had_rename   = true;
							stats_renamed++;
						} else {
							if (verbosity >= 0)
								std::cout << "  d " << lfile.generic_string() << std::endl;
							if (!is_dry) {
								std::filesystem::remove(lpath);
							}
							stats_removed++;
						}
					}
				}
			}

			if (needs_export) {
				if (verbosity >= 0)
					std::cout << "  e " << base_file_name.generic_string() << std::endl;

				if (!is_dry) {
					std::ofstream stream{base_file_path, std::ios::trunc | std::ios::binary | std::ios::out};
					if (!stream || stream.bad() || !stream.is_open()) {
						throw std::runtime_error(string_printf("Failed to export '%s'.", base_file_name.generic_string().c_str()));
					}
					stream.flush();

					if (meta.main_size) {
						if (verbosity >= 1)
							std::cout << "        Writing main section..." << std::endl;
						stream.write(reinterpret_cast<char const*>(meta.main), meta.main_size);
						stream.flush();
					}

					if (meta.stream_size) {
						if (verbosity >= 1)
							std::cout << "        Writing stream section..." << std::endl;
						stream.write(reinterpret_cast<char const*>(meta.stream), meta.stream_size);
						stream.flush();
					}

					if (meta.gpu_size) {
						if (verbosity >= 1)
							std::cout << "        Writing gpu section..." << std::endl;
						stream.write(reinterpret_cast<char const*>(meta.gpu), meta.gpu_size);
						stream.flush();
					}

					stream.close();
				}
				stats_written++;
			} else {
				if (verbosity >= 1)
					std::cout << "  s " << base_file_name.generic_string() << std::endl;
				stats_skipped++;
			}
		}
	}

	std::cout << std::endl;
	if (verbosity >= 0) {
		std::cout << "Total Files: " << stats_total << std::endl;
		std::cout << "    Exported: " << stats_written << std::endl;
		std::cout << "    Skipped:  " << stats_skipped << std::endl;
		std::cout << "    Filtered: " << stats_filtered << std::endl;
		std::cout << "    Names Translated: " << stats_names << std::endl;
		std::cout << "    Types Translated: " << stats_types << std::endl;
		if (rename) {
			std::cout << "Filesystem changes: " << std::endl;
			std::cout << "    Renamed:  " << stats_renamed << std::endl;
			std::cout << "    Deleted:  " << stats_removed << std::endl;
		}
	}

	return 0;
}
static auto instance = hellextractor::mode(std::string(name), std::string(help), mode_extract);
