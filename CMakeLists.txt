cmake_minimum_required(VERSION 2.8)
PROJECT(lz77Compressor CXX)
SET(lz77Compressor_VERSION_MAJOR 1)
SET(lz77Compressor_VERSION_MINOR 1)
SET(CMAKE_CXX_COMPILER /usr/local/bin/g++)
SET(CMAKE_CXX_FLAGS_DEBUG "-g -pg")
SET(CMAKE_CXX_FLAGS_RELEASE "-O2")
SET(CMAKE_CXX_FLAGS_RELEASE_WITH_PROFILE "-O2 -pg")


execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
                OUTPUT_VARIABLE GCC_VERSION)
if (GCC_VERSION VERSION_GREATER 4.3 OR GCC_VERSION VERSION_EQUAL 4.3)
        message(STATUS "Version >= 4.3")
endif()

FIND_PACKAGE(wxWidgets REQUIRED xml adv core base)
if (wxWidgets_FOUND)
  execute_process(COMMAND  wx-config --cxxflags
                   OUTPUT_VARIABLE  wxWidgets_CXX_FLAGS)
  STRING(STRIP ${wxWidgets_CXX_FLAGS} wxWidgets_CXX_FLAGS)
  SET(CMAKE_CXX_FLAGS "-Wall -std=c++0x ${wxWidgets_CXX_FLAGS}")
  ADD_EXECUTABLE(lz77Compressor
   lz77App.cpp lz77Frame.cpp lz77Type10.cpp lz77Type11.cpp lzBase.cpp Trie.cpp lookupTable.cpp
  )
	TARGET_LINK_LIBRARIES(lz77Compressor ${wxWidgets_LIBRARIES})
else()
   MESSAGE("wxWidgets not found!")
endif()
