class Resourse {
public:
	virtual void load_peice(const std::string& path);
	virtual void unload();
};

class ResourceManager {
	std::map<std::string, std::unique_ptr<Resource> > resources;

public:
	// get will block if the requested resource isn't already loaded.
	template <class T>
	T* get(const std::string& path) {
		if(resources[path] != 0)
	}	
}