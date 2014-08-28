#ifndef GECOMMON_CONFIG_HEADER
#define GECOMMON_CONFIG_HEADER

#include "json11.hpp"

#include <fstream>

namespace gecom {
	class Config {
		std::string filepath;
		std::ifstream in;

		json11::Json json_obj;

		void parse_file() {
			in.open(filepath);
			if(!in.good()) throw std::runtime_error("Unable to open config file: " + filepath);
			std::stringstream buf;
			buf << in.rdbuf();
			std::string json_parse_err;
			json_obj = json11::Json::parse(buf.str(), json_parse_err);
			if(json_obj.is_null()) std::cout << "Failed to parse: " << json_parse_err << std::endl;

			// JSON object should be valid now, I guess.
		}

	public:
		Config(const std::string& path) : filepath(path) { parse_file(); }

		template<typename T>
		T get(const std::string& key) {
			// throw new std::runtime_error("Key not in config");
		}

		template<>
		int get<int>(const std::string& key) {
			return std::stoi(get<std::string>(key));
		}
	};
}

#endif