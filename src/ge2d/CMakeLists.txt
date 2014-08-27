SET(GE2D_SOURCES 
	
)

SET(GE2D_HEADERS
	GE2D.hpp
	Game.hpp
	State.hpp
)

source_group("Headers" FILES ${GE2D_HEADERS})

add_library(GE2D ${GE2D_SOURCES} ${GE2D_HEADERS})
set_target_properties(GE2D PROPERTIES LINKER_LANGUAGE CXX)
include_directories("${PROJECT_SOURCE_DIR}/src")
target_link_libraries(GE2D GECommon)