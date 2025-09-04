include_guard(GLOBAL)
cmake_minimum_required(VERSION 3.18.2) # 3.18 has modern python finder

include(../misc/cmake/GlobalHead.cmake)
project(libqif)
include(../misc/cmake/GlobalConfig.cmake)

# If this dir is the root, we need to include qif (if not, then qif is already loaded, and add_subdirectory will do nothing)
add_subdirectory(../lib_cpp lib_cpp)

# build pybind11, FindPython mode (see https://pybind11.readthedocs.io/en/stable/compiling.html)
set(PYBIND11_FINDPYTHON ON)
find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Module)

add_subdirectory(../external/pybind11 pybind11)

# build the python module inside the "qif" dir, unless CMAKE_LIBRARY_OUTPUT_DIRECTORY is set (see setup.py)
if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/qif")
endif()

# build qif python module
file(GLOB_RECURSE CPP_SOURCES src/*.cpp)
pybind11_add_module(qif_python ${CPP_SOURCES})

set_target_properties(qif_python PROPERTIES OUTPUT_NAME "_qif")		# the cpp module is build as "_qif", then imported by __init__.py
target_link_libraries(qif_python PRIVATE qif_cpp)
target_include_directories(qif_python PRIVATE include)

# copy .py files to the output dir (preserving directories)
file(COPY src/qif DESTINATION . FILES_MATCHING PATTERN "*.py")

# on windows, copy required dlls (mp++.dll and libopenblas.dll) to the target dir
if(MSVC)
	find_file(OPENBLAS_PATH NAME libopenblas.dll PATH_SUFFIXES lib/ REQUIRED)
	find_file(MSVPC140_PATH NAME msvcp140.dll REQUIRED)

	add_custom_command(
		TARGET qif_python POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
				$<TARGET_FILE:mp++> ${OPENBLAS_PATH} ${MSVPC140_PATH}
				$<TARGET_FILE_DIR:qif_python>)
endif()

# run test script
# add_custom_target(python_test DEPENDS python)
# add_custom_command(TARGET python_test COMMAND PYTHONPATH=. python3 ../../python/tests/test.py)
