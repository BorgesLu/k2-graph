# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/kvgroup/lmx/k2-graph

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/kvgroup/lmx/k2-graph/build

# Include any dependencies generated for this target.
include src/storage/CMakeFiles/storage_client.dir/depend.make

# Include the progress variables for this target.
include src/storage/CMakeFiles/storage_client.dir/progress.make

# Include the compile flags for this target's objects.
include src/storage/CMakeFiles/storage_client.dir/flags.make

src/storage/CMakeFiles/storage_client.dir/client/StorageClient.cpp.o: src/storage/CMakeFiles/storage_client.dir/flags.make
src/storage/CMakeFiles/storage_client.dir/client/StorageClient.cpp.o: ../src/storage/client/StorageClient.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kvgroup/lmx/k2-graph/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/storage/CMakeFiles/storage_client.dir/client/StorageClient.cpp.o"
	cd /home/kvgroup/lmx/k2-graph/build/src/storage && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/storage_client.dir/client/StorageClient.cpp.o -c /home/kvgroup/lmx/k2-graph/src/storage/client/StorageClient.cpp

src/storage/CMakeFiles/storage_client.dir/client/StorageClient.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/storage_client.dir/client/StorageClient.cpp.i"
	cd /home/kvgroup/lmx/k2-graph/build/src/storage && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kvgroup/lmx/k2-graph/src/storage/client/StorageClient.cpp > CMakeFiles/storage_client.dir/client/StorageClient.cpp.i

src/storage/CMakeFiles/storage_client.dir/client/StorageClient.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/storage_client.dir/client/StorageClient.cpp.s"
	cd /home/kvgroup/lmx/k2-graph/build/src/storage && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kvgroup/lmx/k2-graph/src/storage/client/StorageClient.cpp -o CMakeFiles/storage_client.dir/client/StorageClient.cpp.s

storage_client: src/storage/CMakeFiles/storage_client.dir/client/StorageClient.cpp.o
storage_client: src/storage/CMakeFiles/storage_client.dir/build.make

.PHONY : storage_client

# Rule to build all files generated by this target.
src/storage/CMakeFiles/storage_client.dir/build: storage_client

.PHONY : src/storage/CMakeFiles/storage_client.dir/build

src/storage/CMakeFiles/storage_client.dir/clean:
	cd /home/kvgroup/lmx/k2-graph/build/src/storage && $(CMAKE_COMMAND) -P CMakeFiles/storage_client.dir/cmake_clean.cmake
.PHONY : src/storage/CMakeFiles/storage_client.dir/clean

src/storage/CMakeFiles/storage_client.dir/depend:
	cd /home/kvgroup/lmx/k2-graph/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kvgroup/lmx/k2-graph /home/kvgroup/lmx/k2-graph/src/storage /home/kvgroup/lmx/k2-graph/build /home/kvgroup/lmx/k2-graph/build/src/storage /home/kvgroup/lmx/k2-graph/build/src/storage/CMakeFiles/storage_client.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/storage/CMakeFiles/storage_client.dir/depend
