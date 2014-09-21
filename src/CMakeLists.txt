
# allow targets to include from each other
include_directories("${PROJECT_SOURCE_DIR}/src")

# this needs to be before pxljm
add_subdirectory(gecom)

add_subdirectory(pxljm)
