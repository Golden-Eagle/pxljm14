#ifndef GECOM_SNAPIN_HEADER
#define GECOM_SNAPIN_HEADER

#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <typeindex>

namespace gecom {
	class SnapIn {
		std::shared_ptr<Game> active_game;
	public:
		SnapIn(std::shared_ptr<Game> g) : active_game(g) { }
	};

	class SnapInManager {
		std::unordered_map<std::type_index, std::shared_ptr<SnapIn> > snapins;

	public:
		// void add(const std::type_index &k, const std::shared_ptr<SnapIn> &nsi) {
		// 	snapins[k] = nsi;
		// }

		template <typename T>
		void add(const std::shared_ptr<T> &nsi) {
			static_assert(std::is_base_of<SnapIn, T>::value, "T must implement SnapIn");
			snapins[std::type_index(typeid(T))] = nsi;
		}

		template <typename T>
		std::shared_ptr<T> get() {
			
			auto it = snapins.find(std::type_index(typeid(T)));
			if(it != snapins.end())
				return std::static_pointer_cast<T>(it->second);
			throw std::runtime_error("No component of that type available");
		}
	};
}

#endif