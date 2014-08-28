SET(GE2D_SOURCES 
	Window.cpp	
)

SET(GE2D_HEADERS
	GEGame.hpp
	Fs.hpp
	ResourceManager.hpp
	Game.hpp
	State.hpp
	Window.hpp
)

source_group("Headers" FILES ${GE2D_HEADERS})

add_library(GE2D ${GE2D_SOURCES} ${GE2D_HEADERS})
set_target_properties(GE2D PROPERTIES LINKER_LANGUAGE CXX)
include_directories("${PROJECT_SOURCE_DIR}/src")

add_definitions(-DGLEW_STATIC -DGLEW_MX)
set(ge2d_link_libraries GECommon glew glfw ${GLFW_LIBRARIES} ${OPENGL_glu_LIBRARY})
target_link_libraries(GE2D ${ge2d_link_libraries})
if(WIN32)
	target_link_libraries(wsock32 ws2_32)
endif()