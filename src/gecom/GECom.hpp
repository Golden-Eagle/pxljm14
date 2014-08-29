/*
 * GECom Main Header
 *
 * aka shit that needs to go somewhere
 */

#ifndef GECOM_HPP
#define GECOM_HPP

#include <memory>
#include <utility>

#include "Initial3D.hpp"

// this alias will be available by default in new i3d
namespace i3d = initial3d;

namespace gecom {
	
	class Uncopyable {
	private:
		Uncopyable(const Uncopyable &rhs) = delete;
		Uncopyable & operator=(const Uncopyable &rhs) = delete;
	protected:
		Uncopyable() { }
	};

	// real std::make_unique is c++14, so this will do for the moment
	template <typename T, typename... ArgTR>
	std::unique_ptr<T> make_unique(ArgTR && ...args) {
		return std::unique_ptr<T>(new T(std::forward<ArgTR>(args)...));
	}
	
}

#endif // GECOM_HPP
