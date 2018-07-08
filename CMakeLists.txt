cmake_minimum_required(VERSION 2.8)
project(graph_api C CXX)
# The version number.
set(graph_api_VERSION_MAJOR 1)
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the tpye to build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
      "Debug" "Release" "MinsizeRel" "RelWithDebInfo")
endif()


include(CheckLibraryExists)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include_directories(${CMAKE_SOURCE_DIR})
include(cmake/LibFindMacros.cmake)
if (UNIX AND NOT APPLE)
    libfind_pkg_check_modules(libbsd libbsd)
    find_library(libbsd_LIBRARY
        NAMES bsd
        PATHS ${libbsd_LIBRARY_DIRS}
    )
endif()


find_package(MPI)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
include_directories(${MPI_CXX_INCLUDE_PATH})
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MPI_CXX_LINK_FLAGS}")
include("cmake/FindGlog.cmake")
#find_package(Glog REQUIRED)
include("cmake/FindGFlags.cmake")
#find_package(gflags REQUIRED)
find_package(Boost 1.58 COMPONENTS system filesystem program_options)
include_directories(${Boost_INCLUDE_DIRS})

set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)
# -----------------------------------------------------------------------------
# generete libgraphapi
# -----------------------------------------------------------------------------
file(GLOB CORE_SRC_FILES
      "cpp/core/*.cc"
	  "cpp/parallel/*.cc"
	  "cpp/serial/*.cc"
	  "cpp/utils/*.cc")
add_library(graph_api ${CORE_SRC_FILES})
target_link_libraries(graph_api ${MPI_CXX_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INTI}
                      ${GLOG_LIBRARIES} ${GFLAGS_LIBRARIES})

# -----------------------------------------------------------------------------
# generete applications
# -----------------------------------------------------------------------------
set(APP_FILES function test)

foreach (f ${APP_FILES})
  file(GLOB APP_VARIANTS RELATIVE "${CMAKE_SOURCE_DIR}/cpp/app/${f}" "${CMAKE_SOURCE_DIR}/cpp/app/${f}/*.cc")
  foreach (vr ${APP_VARIANTS})
    string(REGEX MATCH "^(.*)\\.[^.]*$" dummy ${vr})
      set(APP_VARIANT_NAME ${CMAKE_MATCH_1})
      message(STATUS "Found application - " ${APP_VARIANT_NAME})
      add_executable(${APP_VARIANT_NAME} cpp/app/${f}/${APP_VARIANT_NAME}.cc)
      target_link_libraries(${APP_VARIANT_NAME} graph_api)
    endforeach (vr ${APP_VARIANTS})
endforeach (f ${APP_FILES})
