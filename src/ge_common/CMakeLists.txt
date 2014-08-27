SET(GECOM_SOURCES 
	
)

SET(GECOM_HEADERS
	gecom.hpp
)

source_group("Headers" FILES ${GECOM_HEADERS})

add_library(GECommon ${GECOM_SOURCES} ${GECOM_HEADERS})
set_target_properties(GECommon PROPERTIES LINKER_LANGUAGE CXX)