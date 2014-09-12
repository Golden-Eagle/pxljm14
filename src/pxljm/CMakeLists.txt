
SET(PXLJM_SOURCES
	PxlJm.cpp
	Level.cpp
)

SET(PXLJM_HEADERS
	Level.hpp
)

add_executable(pxljm ${PXLJM_SOURCES} ${PXLJM_HEADERS})

add_definitions(${GECOM_DEFINITIONS})
target_link_libraries(pxljm gecom ${GECOM_LIBRARIES})

set_target_properties(pxljm PROPERTIES LINKER_LANGUAGE CXX)
