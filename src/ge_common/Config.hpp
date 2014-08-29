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
		Config(const char* cpath) : Config(std::string(cpath)) { }
		Config(const std::string& path) : filepath(path) { parse_file(); }
		Config(const json11::Json& no) : json_obj(no) { }

		const Config* get(const std::string& key) const {
			if(!json_obj[key].is_object()) throw std::runtime_error("Requested object is not an object");
			return new Config(json_obj[key]);
		}

		const std::string& get_string(const std::string& key) const {
			if(!json_obj[key].is_string()) throw std::runtime_error("Requested key '" + key + "' is not a string");
			return json_obj[key].string_value();
		}

		const int get_int(const std::string& key) const {
			if(!json_obj[key].is_number()) throw std::runtime_error("Requested key '" + key + "' is not an integer");
			return json_obj[key].int_value();
		}
	};
}

#endif