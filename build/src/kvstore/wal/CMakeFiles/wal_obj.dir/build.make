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
include src/kvstore/wal/CMakeFiles/wal_obj.dir/depend.make

# Include the progress variables for this target.
include src/kvstore/wal/CMakeFiles/wal_obj.dir/progress.make

# Include the compile flags for this target's objects.
include src/kvstore/wal/CMakeFiles/wal_obj.dir/flags.make

src/kvstore/wal/CMakeFiles/wal_obj.dir/InMemoryLogBuffer.cpp.o: src/kvstore/wal/CMakeFiles/wal_obj.dir/flags.make
src/kvstore/wal/CMakeFiles/wal_obj.dir/InMemoryLogBuffer.cpp.o: ../src/kvstore/wal/InMemoryLogBuffer.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kvgroup/lmx/k2-graph/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/kvstore/wal/CMakeFiles/wal_obj.dir/InMemoryLogBuffer.cpp.o"
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/wal && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/wal_obj.dir/InMemoryLogBuffer.cpp.o -c /home/kvgroup/lmx/k2-graph/src/kvstore/wal/InMemoryLogBuffer.cpp

src/kvstore/wal/CMakeFiles/wal_obj.dir/InMemoryLogBuffer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/wal_obj.dir/InMemoryLogBuffer.cpp.i"
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/wal && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kvgroup/lmx/k2-graph/src/kvstore/wal/InMemoryLogBuffer.cpp > CMakeFiles/wal_obj.dir/InMemoryLogBuffer.cpp.i

src/kvstore/wal/CMakeFiles/wal_obj.dir/InMemoryLogBuffer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/wal_obj.dir/InMemoryLogBuffer.cpp.s"
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/wal && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kvgroup/lmx/k2-graph/src/kvstore/wal/InMemoryLogBuffer.cpp -o CMakeFiles/wal_obj.dir/InMemoryLogBuffer.cpp.s

src/kvstore/wal/CMakeFiles/wal_obj.dir/FileBasedWalIterator.cpp.o: src/kvstore/wal/CMakeFiles/wal_obj.dir/flags.make
src/kvstore/wal/CMakeFiles/wal_obj.dir/FileBasedWalIterator.cpp.o: ../src/kvstore/wal/FileBasedWalIterator.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kvgroup/lmx/k2-graph/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/kvstore/wal/CMakeFiles/wal_obj.dir/FileBasedWalIterator.cpp.o"
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/wal && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/wal_obj.dir/FileBasedWalIterator.cpp.o -c /home/kvgroup/lmx/k2-graph/src/kvstore/wal/FileBasedWalIterator.cpp

src/kvstore/wal/CMakeFiles/wal_obj.dir/FileBasedWalIterator.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/wal_obj.dir/FileBasedWalIterator.cpp.i"
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/wal && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kvgroup/lmx/k2-graph/src/kvstore/wal/FileBasedWalIterator.cpp > CMakeFiles/wal_obj.dir/FileBasedWalIterator.cpp.i

src/kvstore/wal/CMakeFiles/wal_obj.dir/FileBasedWalIterator.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/wal_obj.dir/FileBasedWalIterator.cpp.s"
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/wal && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kvgroup/lmx/k2-graph/src/kvstore/wal/FileBasedWalIterator.cpp -o CMakeFiles/wal_obj.dir/FileBasedWalIterator.cpp.s

src/kvstore/wal/CMakeFiles/wal_obj.dir/FileBasedWal.cpp.o: src/kvstore/wal/CMakeFiles/wal_obj.dir/flags.make
src/kvstore/wal/CMakeFiles/wal_obj.dir/FileBasedWal.cpp.o: ../src/kvstore/wal/FileBasedWal.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kvgroup/lmx/k2-graph/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object src/kvstore/wal/CMakeFiles/wal_obj.dir/FileBasedWal.cpp.o"
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/wal && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/wal_obj.dir/FileBasedWal.cpp.o -c /home/kvgroup/lmx/k2-graph/src/kvstore/wal/FileBasedWal.cpp

src/kvstore/wal/CMakeFiles/wal_obj.dir/FileBasedWal.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/wal_obj.dir/FileBasedWal.cpp.i"
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/wal && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kvgroup/lmx/k2-graph/src/kvstore/wal/FileBasedWal.cpp > CMakeFiles/wal_obj.dir/FileBasedWal.cpp.i

src/kvstore/wal/CMakeFiles/wal_obj.dir/FileBasedWal.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/wal_obj.dir/FileBasedWal.cpp.s"
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/wal && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kvgroup/lmx/k2-graph/src/kvstore/wal/FileBasedWal.cpp -o CMakeFiles/wal_obj.dir/FileBasedWal.cpp.s

wal_obj: src/kvstore/wal/CMakeFiles/wal_obj.dir/InMemoryLogBuffer.cpp.o
wal_obj: src/kvstore/wal/CMakeFiles/wal_obj.dir/FileBasedWalIterator.cpp.o
wal_obj: src/kvstore/wal/CMakeFiles/wal_obj.dir/FileBasedWal.cpp.o
wal_obj: src/kvstore/wal/CMakeFiles/wal_obj.dir/build.make

.PHONY : wal_obj

# Rule to build all files generated by this target.
src/kvstore/wal/CMakeFiles/wal_obj.dir/build: wal_obj

.PHONY : src/kvstore/wal/CMakeFiles/wal_obj.dir/build

src/kvstore/wal/CMakeFiles/wal_obj.dir/clean:
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/wal && $(CMAKE_COMMAND) -P CMakeFiles/wal_obj.dir/cmake_clean.cmake
.PHONY : src/kvstore/wal/CMakeFiles/wal_obj.dir/clean

src/kvstore/wal/CMakeFiles/wal_obj.dir/depend:
	cd /home/kvgroup/lmx/k2-graph/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kvgroup/lmx/k2-graph /home/kvgroup/lmx/k2-graph/src/kvstore/wal /home/kvgroup/lmx/k2-graph/build /home/kvgroup/lmx/k2-graph/build/src/kvstore/wal /home/kvgroup/lmx/k2-graph/build/src/kvstore/wal/CMakeFiles/wal_obj.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/kvstore/wal/CMakeFiles/wal_obj.dir/depend
