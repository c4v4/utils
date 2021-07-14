cmake_minimum_required(VERSION 3.16)
project(UTILS)

set(CMAKE_CXX_STANDARD 17)

set(WARNING_FLAGS "-Wall -Wextra -Wpedantic -Wuninitialized -Wshadow ")
set(SANITIZERS_FLAGS "-g -O0 -fno-omit-frame-pointer -fsanitize=address -fsanitize-address-use-after-scope -fsanitize=undefined")
set(OPT_FLAGS "-O3 -march=native -ffat-lto-objects -flto")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SANITIZERS_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${OPT_FLAGS}")
set(CMAKE_VERBOSE_MAKEFILE ON)

set(DEFAULT_LIBRARIES fmt pthread dl m)

# Find CPLEX
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(CPLEX)
if (CPLEX_FOUND)
    set(SOURCE ${SOURCE} CPLEXSolver.hpp)
    include_directories(${CPLEX_INCLUDE_DIRS})
    message(${CPLEX_INCLUDE_DIRS})
    set(${CPLEX_LIBRARIES} ${CPLEX_LIBRARIES} pthread dl m)
    add_definitions(-DIL_STD)
endif(CPLEX_FOUND)

include_directories(include ${CPLEX_INCLUDE_DIRS})

# CONCORDE
set(CONCORDE_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/concorde/libconcorde.a)
#add_custom_target(build_concorde ALL 
#                   COMMAND ${CMAKE_MAKE_PROGRAM}
#                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/concorde
#                   COMMENT "Concorde solver")

add_library(libconcorde STATIC IMPORTED)
set_property(TARGET libconcorde APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(libconcorde PROPERTIES IMPORTED_LOCATION_NOCONFIG "${CONCORDE_LIBRARY}")

set(SOURCE1  src/concorde_example.cpp)
add_executable(concorde_example ${SOURCE1})
target_link_libraries(concorde_example ${DEFAULT_LIBRARIES} ${CPLEX_LIBRARIES} ${CMAKE_CURRENT_SOURCE_DIR}/concorde/libconcorde.a)


# CPLEX TSP
set(SOURCE2  src/cplex_tsp_example.cpp)
add_executable(cplex_tsp_example ${SOURCE2})
target_link_libraries(cplex_tsp_example ${DEFAULT_LIBRARIES} ${CPLEX_LIBRARIES} ${CMAKE_CURRENT_SOURCE_DIR}/concorde/libconcorde.a)

# CPLEX SCP
#set(SOURCE3  src/cplex_scp_example.cpp)
#add_executable(cplex_scp_example ${SOURCE3})
#target_link_libraries(cplex_scp_example ${DEFAULT_LIBRARIES} ${CPLEX_LIBRARIES})
