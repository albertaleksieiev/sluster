cmake_minimum_required(VERSION 3.6.3)
project(Training)



SET(CMAKE_CXX_COMPILER /usr/local/bin/g++-6)
add_compile_options(-std=c++1z)


set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.61.0)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_executable(SocketClient calculation.cpp)
    target_link_libraries(SocketClient ${Boost_LIBRARIES})
endif()
