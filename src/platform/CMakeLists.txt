
SET(PLATFORM_SOURCES
	Platform.cpp
)

SET(PLATFORM_HEADERS

)

add_executable(platform ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})

add_definitions(${GECOM_DEFINITIONS})
target_link_libraries(platform gecom ${GECOM_LIBRARIES})

set_target_properties(platform PROPERTIES LINKER_LANGUAGE CXX)
