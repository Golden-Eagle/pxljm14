
#ifndef GECOM_HPP
#define GECOM_HPP

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
	
}

#endif // GECOM_HPP
