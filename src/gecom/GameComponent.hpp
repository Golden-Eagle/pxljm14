#ifndef GECOM_GAMECOM_HEADER
#define GECOM_GAMECOM_HEADER

#include "Game.hpp"

namespace gecom {
	class Game;

	class GameComponent {
		// TODO use a pointer for this
		Game& active_game;
	public:
		GameComponent(Game &g) : active_game(g) { }

		virtual void init() = 0;
		virtual void update() = 0;
	};

	class GameComponentManager {
		std::unordered_map<std::type_index, std::shared_ptr<GameComponent> > GameComponents;

	public:
		// void add(const std::type_index &k, const std::shared_ptr<GameComponent> &nsi) {
		// 	GameComponents[k] = nsi;
		// }

		template <typename T>
		void add(const std::shared_ptr<T> &nsi) {
			static_assert(std::is_base_of<GameComponent, T>::value, "T must implement GameComponent");
			GameComponents[std::type_index(typeid(T))] = nsi;
		}

		template <typename T>
		std::shared_ptr<T> get() {
			auto it = GameComponents.find(std::type_index(typeid(T)));
			if (it != GameComponents.end())
				return std::static_pointer_cast<T>(it->second);
			throw std::runtime_error("No component of that type available");
		}

		template <typename T>
		inline bool exists() {
			return GameComponents.count(std::type_index(typeid(T))) > 0;
		}

		inline void init() {
			for (auto it : GameComponents) {
				it.second->init();
			}
		}

		void update() {
			for (auto it : GameComponents) {
				it.second->update();
			}
		}
	};

}

#endif