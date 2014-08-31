#ifndef GECOM_ENGINECOMPONENT_HEADER
#define GECOM_ENGINECOMPONENT_HEADER

#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <typeindex>

#include "Game.hpp"

namespace gecom {
	class Game;

	class EngineComponent {
		std::shared_ptr<Game> active_game;
	public:
		EngineComponent(std::shared_ptr<Game> g) : active_game(g) { }

		virtual void init() =0;
		virtual void update() =0;
	};

	class EngineComponentManager {
		std::unordered_map<std::type_index, std::shared_ptr<EngineComponent> > EngineComponents;

	public:
		// void add(const std::type_index &k, const std::shared_ptr<EngineComponent> &nsi) {
		// 	EngineComponents[k] = nsi;
		// }

		template <typename T>
		void add(const std::shared_ptr<T> &nsi) {
			static_assert(std::is_base_of<EngineComponent, T>::value, "T must implement EngineComponent");
			EngineComponents[std::type_index(typeid(T))] = nsi;
		}

		template <typename T>
		std::shared_ptr<T> get() {
			auto it = EngineComponents.find(std::type_index(typeid(T)));
			if(it != EngineComponents.end())
				return std::static_pointer_cast<T>(it->second);
			throw std::runtime_error("No component of that type available");
		}

		template <typename T>
		bool exists() {
			return EngineComponents.count(std::type_index(typeid(T))) > 0;
		}

		void init() {
			for(auto it : EngineComponents) {
				it.second->init();
			}
		}

		void update() {
			for(auto it : EngineComponents) {
				it.second->update();
			}
		}
	};
}

#endif