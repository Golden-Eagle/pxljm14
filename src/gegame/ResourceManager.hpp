#ifndef GEGAME_RESOURCE_HEADER
#define GEGAME_RESOURCE_HEADER

#include <map>
#include <memory>
#include <type_traits>
#include <fstream>
#include <stdexcept>

class resource_manager_error : public std::runtime_error { 
	const std::string filepath;
public:
	resource_manager_error(const std::string& msg, const std::string& path) : runtime_error(msg), filepath(path) { }
};

class Resource {
	const std::string source_path;
public:
	Resource(const std::string& path) : source_path(path) { }
	virtual ~Resource();
	virtual bool load_piece() =0;

	void ensure_loaded() {
		while(load_piece()) ;
	}
};

inline Resource::~Resource() { }

class Text : public Resource {
	std::string contents;
	std::string tmp;
	std::ifstream in;
public:
	Text(const std::string& path) : Resource(path) {
		in.open(path);
		if(!in.good())
			throw new resource_manager_error("File not found", path);
	}
	~Text() { }
	bool load_piece() {
		if(in.eof()) return false;
		if(contents.length() > 0) contents += " ";

		in >> tmp;
		contents += tmp;
		return true;
	}

	std::string& data() { return contents; }
};

class ResourceManager {
	std::map<std::string, std::shared_ptr<Resource> > resources;
	std::string path_prefix;
public:
	ResourceManager(const std::string& rdir) : path_prefix(rdir) { }
	~ResourceManager() {
		// TODO: Force unload everything?
	}

	// returns true if there is more files / blocks to be loaded
	bool cache() {
		for(auto it : resources) {
			if(it.second->load_piece()) return true;
		}
		return false;
	}

	template <class T>
	void add_cache_file(const std::string& path) {
		static_assert(std::is_base_of<Resource, T>::value, "T must be a Resource subclass");

		std::string real_path = path_prefix + path;

		if(resources.find(real_path) != resources.end())
			return;

		auto new_t = std::make_shared<T>(real_path);
		resources[real_path] = new_t;
	}

	// get will block if the requested resource isn't already loaded.
	template <class T>
	std::shared_ptr<T> get(const std::string& path) {
		static_assert(std::is_base_of<Resource, T>::value, "T must be a Resource subclass");

		std::string real_path = path_prefix + path;

		if(resources.find(real_path) != resources.end()) {
			resources[real_path]->ensure_loaded();
			return std::static_pointer_cast<T>(resources[real_path]);
		}

		auto new_t = std::make_shared<T>(real_path);
		new_t->ensure_loaded();
		resources[real_path] = new_t;
		return new_t;
	}	
};

#endif