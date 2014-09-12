
#include "Render.hpp"

namespace gecom {

	std::unordered_map<std::type_index, Technique *> Technique::sm_singletons;
	std::mutex Technique::sm_mutex;

}