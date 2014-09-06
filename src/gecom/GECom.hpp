/*
 * GECom Main Header
 *
 * aka shit that needs to go somewhere
 */

#ifndef GECOM_HPP
#define GECOM_HPP

#include <cctype>
#include <string>
#include <algorithm>
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
	inline std::unique_ptr<T> make_unique(ArgTR && ...args) {
		return std::unique_ptr<T>(new T(std::forward<ArgTR>(args)...));
	}
	
	// trim leading and trailing whitespace
	inline std::string trim(const std::string &s) {
	   auto wsfront = std::find_if_not(s.begin(), s.end(), std::isspace);
	   auto wsback = std::find_if_not(s.rbegin(), s.rend(), std::isspace).base();
	   return wsback <= wsfront ? std::string() : std::string(wsfront, wsback);
	}
}

#endif // GECOM_HPP
