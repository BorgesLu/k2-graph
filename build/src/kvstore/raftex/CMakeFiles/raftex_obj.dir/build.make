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
include src/kvstore/raftex/CMakeFiles/raftex_obj.dir/depend.make

# Include the progress variables for this target.
include src/kvstore/raftex/CMakeFiles/raftex_obj.dir/progress.make

# Include the compile flags for this target's objects.
include src/kvstore/raftex/CMakeFiles/raftex_obj.dir/flags.make

src/kvstore/raftex/CMakeFiles/raftex_obj.dir/LogStrListIterator.cpp.o: src/kvstore/raftex/CMakeFiles/raftex_obj.dir/flags.make
src/kvstore/raftex/CMakeFiles/raftex_obj.dir/LogStrListIterator.cpp.o: ../src/kvstore/raftex/LogStrListIterator.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kvgroup/lmx/k2-graph/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/kvstore/raftex/CMakeFiles/raftex_obj.dir/LogStrListIterator.cpp.o"
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/raftex && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/raftex_obj.dir/LogStrListIterator.cpp.o -c /home/kvgroup/lmx/k2-graph/src/kvstore/raftex/LogStrListIterator.cpp

src/kvstore/raftex/CMakeFiles/raftex_obj.dir/LogStrListIterator.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/raftex_obj.dir/LogStrListIterator.cpp.i"
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/raftex && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kvgroup/lmx/k2-graph/src/kvstore/raftex/LogStrListIterator.cpp > CMakeFiles/raftex_obj.dir/LogStrListIterator.cpp.i

src/kvstore/raftex/CMakeFiles/raftex_obj.dir/LogStrListIterator.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/raftex_obj.dir/LogStrListIterator.cpp.s"
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/raftex && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kvgroup/lmx/k2-graph/src/kvstore/raftex/LogStrListIterator.cpp -o CMakeFiles/raftex_obj.dir/LogStrListIterator.cpp.s

src/kvstore/raftex/CMakeFiles/raftex_obj.dir/RaftPart.cpp.o: src/kvstore/raftex/CMakeFiles/raftex_obj.dir/flags.make
src/kvstore/raftex/CMakeFiles/raftex_obj.dir/RaftPart.cpp.o: ../src/kvstore/raftex/RaftPart.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kvgroup/lmx/k2-graph/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/kvstore/raftex/CMakeFiles/raftex_obj.dir/RaftPart.cpp.o"
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/raftex && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/raftex_obj.dir/RaftPart.cpp.o -c /home/kvgroup/lmx/k2-graph/src/kvstore/raftex/RaftPart.cpp

src/kvstore/raftex/CMakeFiles/raftex_obj.dir/RaftPart.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/raftex_obj.dir/RaftPart.cpp.i"
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/raftex && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kvgroup/lmx/k2-graph/src/kvstore/raftex/RaftPart.cpp > CMakeFiles/raftex_obj.dir/RaftPart.cpp.i

src/kvstore/raftex/CMakeFiles/raftex_obj.dir/RaftPart.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/raftex_obj.dir/RaftPart.cpp.s"
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/raftex && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kvgroup/lmx/k2-graph/src/kvstore/raftex/RaftPart.cpp -o CMakeFiles/raftex_obj.dir/RaftPart.cpp.s

src/kvstore/raftex/CMakeFiles/raftex_obj.dir/RaftexService.cpp.o: src/kvstore/raftex/CMakeFiles/raftex_obj.dir/flags.make
src/kvstore/raftex/CMakeFiles/raftex_obj.dir/RaftexService.cpp.o: ../src/kvstore/raftex/RaftexService.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kvgroup/lmx/k2-graph/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object src/kvstore/raftex/CMakeFiles/raftex_obj.dir/RaftexService.cpp.o"
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/raftex && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/raftex_obj.dir/RaftexService.cpp.o -c /home/kvgroup/lmx/k2-graph/src/kvstore/raftex/RaftexService.cpp

src/kvstore/raftex/CMakeFiles/raftex_obj.dir/RaftexService.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/raftex_obj.dir/RaftexService.cpp.i"
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/raftex && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kvgroup/lmx/k2-graph/src/kvstore/raftex/RaftexService.cpp > CMakeFiles/raftex_obj.dir/RaftexService.cpp.i

src/kvstore/raftex/CMakeFiles/raftex_obj.dir/RaftexService.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/raftex_obj.dir/RaftexService.cpp.s"
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/raftex && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kvgroup/lmx/k2-graph/src/kvstore/raftex/RaftexService.cpp -o CMakeFiles/raftex_obj.dir/RaftexService.cpp.s

src/kvstore/raftex/CMakeFiles/raftex_obj.dir/Host.cpp.o: src/kvstore/raftex/CMakeFiles/raftex_obj.dir/flags.make
src/kvstore/raftex/CMakeFiles/raftex_obj.dir/Host.cpp.o: ../src/kvstore/raftex/Host.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kvgroup/lmx/k2-graph/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object src/kvstore/raftex/CMakeFiles/raftex_obj.dir/Host.cpp.o"
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/raftex && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/raftex_obj.dir/Host.cpp.o -c /home/kvgroup/lmx/k2-graph/src/kvstore/raftex/Host.cpp

src/kvstore/raftex/CMakeFiles/raftex_obj.dir/Host.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/raftex_obj.dir/Host.cpp.i"
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/raftex && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kvgroup/lmx/k2-graph/src/kvstore/raftex/Host.cpp > CMakeFiles/raftex_obj.dir/Host.cpp.i

src/kvstore/raftex/CMakeFiles/raftex_obj.dir/Host.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/raftex_obj.dir/Host.cpp.s"
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/raftex && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kvgroup/lmx/k2-graph/src/kvstore/raftex/Host.cpp -o CMakeFiles/raftex_obj.dir/Host.cpp.s

src/kvstore/raftex/CMakeFiles/raftex_obj.dir/SnapshotManager.cpp.o: src/kvstore/raftex/CMakeFiles/raftex_obj.dir/flags.make
src/kvstore/raftex/CMakeFiles/raftex_obj.dir/SnapshotManager.cpp.o: ../src/kvstore/raftex/SnapshotManager.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kvgroup/lmx/k2-graph/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object src/kvstore/raftex/CMakeFiles/raftex_obj.dir/SnapshotManager.cpp.o"
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/raftex && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/raftex_obj.dir/SnapshotManager.cpp.o -c /home/kvgroup/lmx/k2-graph/src/kvstore/raftex/SnapshotManager.cpp

src/kvstore/raftex/CMakeFiles/raftex_obj.dir/SnapshotManager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/raftex_obj.dir/SnapshotManager.cpp.i"
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/raftex && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kvgroup/lmx/k2-graph/src/kvstore/raftex/SnapshotManager.cpp > CMakeFiles/raftex_obj.dir/SnapshotManager.cpp.i

src/kvstore/raftex/CMakeFiles/raftex_obj.dir/SnapshotManager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/raftex_obj.dir/SnapshotManager.cpp.s"
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/raftex && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kvgroup/lmx/k2-graph/src/kvstore/raftex/SnapshotManager.cpp -o CMakeFiles/raftex_obj.dir/SnapshotManager.cpp.s

raftex_obj: src/kvstore/raftex/CMakeFiles/raftex_obj.dir/LogStrListIterator.cpp.o
raftex_obj: src/kvstore/raftex/CMakeFiles/raftex_obj.dir/RaftPart.cpp.o
raftex_obj: src/kvstore/raftex/CMakeFiles/raftex_obj.dir/RaftexService.cpp.o
raftex_obj: src/kvstore/raftex/CMakeFiles/raftex_obj.dir/Host.cpp.o
raftex_obj: src/kvstore/raftex/CMakeFiles/raftex_obj.dir/SnapshotManager.cpp.o
raftex_obj: src/kvstore/raftex/CMakeFiles/raftex_obj.dir/build.make

.PHONY : raftex_obj

# Rule to build all files generated by this target.
src/kvstore/raftex/CMakeFiles/raftex_obj.dir/build: raftex_obj

.PHONY : src/kvstore/raftex/CMakeFiles/raftex_obj.dir/build

src/kvstore/raftex/CMakeFiles/raftex_obj.dir/clean:
	cd /home/kvgroup/lmx/k2-graph/build/src/kvstore/raftex && $(CMAKE_COMMAND) -P CMakeFiles/raftex_obj.dir/cmake_clean.cmake
.PHONY : src/kvstore/raftex/CMakeFiles/raftex_obj.dir/clean

src/kvstore/raftex/CMakeFiles/raftex_obj.dir/depend:
	cd /home/kvgroup/lmx/k2-graph/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kvgroup/lmx/k2-graph /home/kvgroup/lmx/k2-graph/src/kvstore/raftex /home/kvgroup/lmx/k2-graph/build /home/kvgroup/lmx/k2-graph/build/src/kvstore/raftex /home/kvgroup/lmx/k2-graph/build/src/kvstore/raftex/CMakeFiles/raftex_obj.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/kvstore/raftex/CMakeFiles/raftex_obj.dir/depend
