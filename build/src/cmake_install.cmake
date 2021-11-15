# Install script for directory: /home/kvgroup/lmx/k2-graph/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local/nebula")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/kvgroup/lmx/k2-graph/build/src/common/cmake_install.cmake")
  include("/home/kvgroup/lmx/k2-graph/build/src/interface/cmake_install.cmake")
  include("/home/kvgroup/lmx/k2-graph/build/src/client/cmake_install.cmake")
  include("/home/kvgroup/lmx/k2-graph/build/src/meta/cmake_install.cmake")
  include("/home/kvgroup/lmx/k2-graph/build/src/console/cmake_install.cmake")
  include("/home/kvgroup/lmx/k2-graph/build/src/kvstore/cmake_install.cmake")
  include("/home/kvgroup/lmx/k2-graph/build/src/parser/cmake_install.cmake")
  include("/home/kvgroup/lmx/k2-graph/build/src/dataman/cmake_install.cmake")
  include("/home/kvgroup/lmx/k2-graph/build/src/graph/cmake_install.cmake")
  include("/home/kvgroup/lmx/k2-graph/build/src/daemons/cmake_install.cmake")
  include("/home/kvgroup/lmx/k2-graph/build/src/storage/cmake_install.cmake")
  include("/home/kvgroup/lmx/k2-graph/build/src/webservice/cmake_install.cmake")
  include("/home/kvgroup/lmx/k2-graph/build/src/tools/cmake_install.cmake")

endif()

