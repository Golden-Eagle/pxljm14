SET(GECOM_SOURCES 
	Log.cpp
	Concurrent.cpp
)

SET(GECOM_HEADERS
	GECom.hpp
	Log.hpp
	Concurrent.hpp
)

source_group("Headers" FILES ${GECOM_HEADERS})

add_library(GECommon ${GECOM_SOURCES} ${GECOM_HEADERS})
target_link_libraries(GECommon cryptopp ${CRYPTOPP_LIBRARIES})


add_definitions(${CRYPTOPP_DEFINITIONS})

set_target_properties(GECommon PROPERTIES LINKER_LANGUAGE CXX)