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
include src/meta/CMakeFiles/meta_http_handler.dir/depend.make

# Include the progress variables for this target.
include src/meta/CMakeFiles/meta_http_handler.dir/progress.make

# Include the compile flags for this target's objects.
include src/meta/CMakeFiles/meta_http_handler.dir/flags.make

src/meta/CMakeFiles/meta_http_handler.dir/MetaHttpIngestHandler.cpp.o: src/meta/CMakeFiles/meta_http_handler.dir/flags.make
src/meta/CMakeFiles/meta_http_handler.dir/MetaHttpIngestHandler.cpp.o: ../src/meta/MetaHttpIngestHandler.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kvgroup/lmx/k2-graph/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/meta/CMakeFiles/meta_http_handler.dir/MetaHttpIngestHandler.cpp.o"
	cd /home/kvgroup/lmx/k2-graph/build/src/meta && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/meta_http_handler.dir/MetaHttpIngestHandler.cpp.o -c /home/kvgroup/lmx/k2-graph/src/meta/MetaHttpIngestHandler.cpp

src/meta/CMakeFiles/meta_http_handler.dir/MetaHttpIngestHandler.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/meta_http_handler.dir/MetaHttpIngestHandler.cpp.i"
	cd /home/kvgroup/lmx/k2-graph/build/src/meta && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kvgroup/lmx/k2-graph/src/meta/MetaHttpIngestHandler.cpp > CMakeFiles/meta_http_handler.dir/MetaHttpIngestHandler.cpp.i

src/meta/CMakeFiles/meta_http_handler.dir/MetaHttpIngestHandler.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/meta_http_handler.dir/MetaHttpIngestHandler.cpp.s"
	cd /home/kvgroup/lmx/k2-graph/build/src/meta && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kvgroup/lmx/k2-graph/src/meta/MetaHttpIngestHandler.cpp -o CMakeFiles/meta_http_handler.dir/MetaHttpIngestHandler.cpp.s

src/meta/CMakeFiles/meta_http_handler.dir/MetaHttpDownloadHandler.cpp.o: src/meta/CMakeFiles/meta_http_handler.dir/flags.make
src/meta/CMakeFiles/meta_http_handler.dir/MetaHttpDownloadHandler.cpp.o: ../src/meta/MetaHttpDownloadHandler.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kvgroup/lmx/k2-graph/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/meta/CMakeFiles/meta_http_handler.dir/MetaHttpDownloadHandler.cpp.o"
	cd /home/kvgroup/lmx/k2-graph/build/src/meta && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/meta_http_handler.dir/MetaHttpDownloadHandler.cpp.o -c /home/kvgroup/lmx/k2-graph/src/meta/MetaHttpDownloadHandler.cpp

src/meta/CMakeFiles/meta_http_handler.dir/MetaHttpDownloadHandler.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/meta_http_handler.dir/MetaHttpDownloadHandler.cpp.i"
	cd /home/kvgroup/lmx/k2-graph/build/src/meta && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kvgroup/lmx/k2-graph/src/meta/MetaHttpDownloadHandler.cpp > CMakeFiles/meta_http_handler.dir/MetaHttpDownloadHandler.cpp.i

src/meta/CMakeFiles/meta_http_handler.dir/MetaHttpDownloadHandler.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/meta_http_handler.dir/MetaHttpDownloadHandler.cpp.s"
	cd /home/kvgroup/lmx/k2-graph/build/src/meta && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kvgroup/lmx/k2-graph/src/meta/MetaHttpDownloadHandler.cpp -o CMakeFiles/meta_http_handler.dir/MetaHttpDownloadHandler.cpp.s

src/meta/CMakeFiles/meta_http_handler.dir/MetaHttpReplaceHostHandler.cpp.o: src/meta/CMakeFiles/meta_http_handler.dir/flags.make
src/meta/CMakeFiles/meta_http_handler.dir/MetaHttpReplaceHostHandler.cpp.o: ../src/meta/MetaHttpReplaceHostHandler.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kvgroup/lmx/k2-graph/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object src/meta/CMakeFiles/meta_http_handler.dir/MetaHttpReplaceHostHandler.cpp.o"
	cd /home/kvgroup/lmx/k2-graph/build/src/meta && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/meta_http_handler.dir/MetaHttpReplaceHostHandler.cpp.o -c /home/kvgroup/lmx/k2-graph/src/meta/MetaHttpReplaceHostHandler.cpp

src/meta/CMakeFiles/meta_http_handler.dir/MetaHttpReplaceHostHandler.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/meta_http_handler.dir/MetaHttpReplaceHostHandler.cpp.i"
	cd /home/kvgroup/lmx/k2-graph/build/src/meta && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kvgroup/lmx/k2-graph/src/meta/MetaHttpReplaceHostHandler.cpp > CMakeFiles/meta_http_handler.dir/MetaHttpReplaceHostHandler.cpp.i

src/meta/CMakeFiles/meta_http_handler.dir/MetaHttpReplaceHostHandler.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/meta_http_handler.dir/MetaHttpReplaceHostHandler.cpp.s"
	cd /home/kvgroup/lmx/k2-graph/build/src/meta && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kvgroup/lmx/k2-graph/src/meta/MetaHttpReplaceHostHandler.cpp -o CMakeFiles/meta_http_handler.dir/MetaHttpReplaceHostHandler.cpp.s

meta_http_handler: src/meta/CMakeFiles/meta_http_handler.dir/MetaHttpIngestHandler.cpp.o
meta_http_handler: src/meta/CMakeFiles/meta_http_handler.dir/MetaHttpDownloadHandler.cpp.o
meta_http_handler: src/meta/CMakeFiles/meta_http_handler.dir/MetaHttpReplaceHostHandler.cpp.o
meta_http_handler: src/meta/CMakeFiles/meta_http_handler.dir/build.make

.PHONY : meta_http_handler

# Rule to build all files generated by this target.
src/meta/CMakeFiles/meta_http_handler.dir/build: meta_http_handler

.PHONY : src/meta/CMakeFiles/meta_http_handler.dir/build

src/meta/CMakeFiles/meta_http_handler.dir/clean:
	cd /home/kvgroup/lmx/k2-graph/build/src/meta && $(CMAKE_COMMAND) -P CMakeFiles/meta_http_handler.dir/cmake_clean.cmake
.PHONY : src/meta/CMakeFiles/meta_http_handler.dir/clean

src/meta/CMakeFiles/meta_http_handler.dir/depend:
	cd /home/kvgroup/lmx/k2-graph/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kvgroup/lmx/k2-graph /home/kvgroup/lmx/k2-graph/src/meta /home/kvgroup/lmx/k2-graph/build /home/kvgroup/lmx/k2-graph/build/src/meta /home/kvgroup/lmx/k2-graph/build/src/meta/CMakeFiles/meta_http_handler.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/meta/CMakeFiles/meta_http_handler.dir/depend

