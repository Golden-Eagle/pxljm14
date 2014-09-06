/*
 * GECom GL Header
 *
 * Provides access to the GL API.
 * This should be included instead of GLEW or GLFW,
 * although you probably just want Window.hpp.
 *
 * Main purpose is to resolve an include loop between Window.hpp and Shader.hpp.
 * 
 */

#ifndef GECOM_GL_HPP
#define GECOM_GL_HPP

// this is to enable multiple context support, defined in Window.cpp
namespace gecom {
	void * getGlewContext();
}

#define glewGetContext() ((GLEWContext *) gecom::getGlewContext())

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#endif // GECOM_GL_HPP