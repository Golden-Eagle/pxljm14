add_library(GE2D GE2D.hpp)
set_target_properties(GE2D PROPERTIES LINKER_LANGUAGE CXX)
include_directories("${PROJECT_SOURCE_DIR}/src")
target_link_libraries(GE2D GECommon)