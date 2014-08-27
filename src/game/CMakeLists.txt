add_executable(PlatformGame platform.cpp)
set_target_properties(PlatformGame PROPERTIES LINKER_LANGUAGE CXX)
include_directories("${PROJECT_SOURCE_DIR}/src/ge2d")
target_link_libraries(PlatformGame GE2D)