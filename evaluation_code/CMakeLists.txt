###############################################################################
# Author dmytro.bobkov@tum.de
#
cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(evaluate_multiscale)

###############################################################################
# Set cmake options
#
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS} -g -ftest-coverage -fprofile-arcs")

###############################################################################
# Eigen3 is needed in case we use our custom PCL_DIR because otherwise no EIGEN has been cross-linked
#
find_package( Eigen3 REQUIRED ) #


###############################################################################
# PCL linking set custom directory for PCL in case you want to use your bleeding edge stuff
#
message(STATUS "Set PCL directory to " ${PCL_DIR})
find_package(PCL 1.8 REQUIRED NO_DEFAULT_PATH)





###############################################################################
# Setting dirs
#
set(INCLUDE_DIRS    ${PCL_INCLUDE_DIRS}
                    ${Boost_INCLUDE_DIRS}
                    ${EIGEN3_INCLUDE_DIR})

set(LIBRARY_DIRS    ${PCL_LIBRARY_DIRS}
                    ${Boost_LIBRARY_DIRS})

set(LIBRARIES       ${PCL_LIBRARIES}
                    ${Boost_LIBRARIES})

set(DEFINITIONS   ${PCL_DEFINITIONS})


###############################################################################
# Final build setup
#
include_directories(${INCLUDE_DIRS})
link_directories(${LIBRARY_DIRS})
add_definitions(${DEFINITIONS})


add_executable(evaluate_multiscale evaluate_multiscale.cpp evaluation_computer.h evaluation_computer.cpp)

target_link_libraries(evaluate_multiscale ${LIBRARIES})
