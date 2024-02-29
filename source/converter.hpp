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
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include "hd2_data.hpp"

namespace hellextractor::converter {
	class base;

	class registry {
		public:
		static std::shared_ptr<hellextractor::converter::base> find(helldivers2::data::meta_t meta);

		public:
		typedef std::function<std::shared_ptr<hellextractor::converter::base>(helldivers2::data::meta_t meta)> function_t;

		struct do_register {

			public:
			do_register(std::list<uint64_t> types, function_t fn);
		};
	};

	class base {
		protected:
		helldivers2::data::meta_t _meta;

		public:
		virtual ~base();
		base(helldivers2::data::meta_t meta);
		
		/** Retrieve a list of outputs
		 * 
		 * Key is the section that is exported.
		 * Value is a pair of size and the suffix to the file name.
		 */
		virtual std::map<std::string, std::pair<size_t, std::string>> outputs() = 0;

		virtual void extract(std::string section, std::filesystem::path path) = 0;
	};
} // namespace hellextractor::converter
