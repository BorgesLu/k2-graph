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
include src/common/concurrent/CMakeFiles/concurrent_obj.dir/depend.make

# Include the progress variables for this target.
include src/common/concurrent/CMakeFiles/concurrent_obj.dir/progress.make

# Include the compile flags for this target's objects.
include src/common/concurrent/CMakeFiles/concurrent_obj.dir/flags.make

src/common/concurrent/CMakeFiles/concurrent_obj.dir/Barrier.cpp.o: src/common/concurrent/CMakeFiles/concurrent_obj.dir/flags.make
src/common/concurrent/CMakeFiles/concurrent_obj.dir/Barrier.cpp.o: ../src/common/concurrent/Barrier.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kvgroup/lmx/k2-graph/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/common/concurrent/CMakeFiles/concurrent_obj.dir/Barrier.cpp.o"
	cd /home/kvgroup/lmx/k2-graph/build/src/common/concurrent && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/concurrent_obj.dir/Barrier.cpp.o -c /home/kvgroup/lmx/k2-graph/src/common/concurrent/Barrier.cpp

src/common/concurrent/CMakeFiles/concurrent_obj.dir/Barrier.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/concurrent_obj.dir/Barrier.cpp.i"
	cd /home/kvgroup/lmx/k2-graph/build/src/common/concurrent && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kvgroup/lmx/k2-graph/src/common/concurrent/Barrier.cpp > CMakeFiles/concurrent_obj.dir/Barrier.cpp.i

src/common/concurrent/CMakeFiles/concurrent_obj.dir/Barrier.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/concurrent_obj.dir/Barrier.cpp.s"
	cd /home/kvgroup/lmx/k2-graph/build/src/common/concurrent && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kvgroup/lmx/k2-graph/src/common/concurrent/Barrier.cpp -o CMakeFiles/concurrent_obj.dir/Barrier.cpp.s

src/common/concurrent/CMakeFiles/concurrent_obj.dir/Latch.cpp.o: src/common/concurrent/CMakeFiles/concurrent_obj.dir/flags.make
src/common/concurrent/CMakeFiles/concurrent_obj.dir/Latch.cpp.o: ../src/common/concurrent/Latch.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kvgroup/lmx/k2-graph/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/common/concurrent/CMakeFiles/concurrent_obj.dir/Latch.cpp.o"
	cd /home/kvgroup/lmx/k2-graph/build/src/common/concurrent && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/concurrent_obj.dir/Latch.cpp.o -c /home/kvgroup/lmx/k2-graph/src/common/concurrent/Latch.cpp

src/common/concurrent/CMakeFiles/concurrent_obj.dir/Latch.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/concurrent_obj.dir/Latch.cpp.i"
	cd /home/kvgroup/lmx/k2-graph/build/src/common/concurrent && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kvgroup/lmx/k2-graph/src/common/concurrent/Latch.cpp > CMakeFiles/concurrent_obj.dir/Latch.cpp.i

src/common/concurrent/CMakeFiles/concurrent_obj.dir/Latch.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/concurrent_obj.dir/Latch.cpp.s"
	cd /home/kvgroup/lmx/k2-graph/build/src/common/concurrent && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kvgroup/lmx/k2-graph/src/common/concurrent/Latch.cpp -o CMakeFiles/concurrent_obj.dir/Latch.cpp.s

concurrent_obj: src/common/concurrent/CMakeFiles/concurrent_obj.dir/Barrier.cpp.o
concurrent_obj: src/common/concurrent/CMakeFiles/concurrent_obj.dir/Latch.cpp.o
concurrent_obj: src/common/concurrent/CMakeFiles/concurrent_obj.dir/build.make

.PHONY : concurrent_obj

# Rule to build all files generated by this target.
src/common/concurrent/CMakeFiles/concurrent_obj.dir/build: concurrent_obj

.PHONY : src/common/concurrent/CMakeFiles/concurrent_obj.dir/build

src/common/concurrent/CMakeFiles/concurrent_obj.dir/clean:
	cd /home/kvgroup/lmx/k2-graph/build/src/common/concurrent && $(CMAKE_COMMAND) -P CMakeFiles/concurrent_obj.dir/cmake_clean.cmake
.PHONY : src/common/concurrent/CMakeFiles/concurrent_obj.dir/clean

src/common/concurrent/CMakeFiles/concurrent_obj.dir/depend:
	cd /home/kvgroup/lmx/k2-graph/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kvgroup/lmx/k2-graph /home/kvgroup/lmx/k2-graph/src/common/concurrent /home/kvgroup/lmx/k2-graph/build /home/kvgroup/lmx/k2-graph/build/src/common/concurrent /home/kvgroup/lmx/k2-graph/build/src/common/concurrent/CMakeFiles/concurrent_obj.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/common/concurrent/CMakeFiles/concurrent_obj.dir/depend

