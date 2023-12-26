# Get list of sources.
file(GLOB_RECURSE SOURCES "*.cpp")
find_package(Boost 1.82.0)

get_filename_component(binary_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)

# Add the executable.
include_directories(${Boost_INCLUDE_DIRS})
add_executable("main_${binary_name}" ${SOURCES})
set_target_properties("main_${binary_name}" PROPERTIES OUTPUT_NAME "${binary_name}")

# Install application.
install(TARGETS "main_${binary_name}" DESTINATION "bin")
