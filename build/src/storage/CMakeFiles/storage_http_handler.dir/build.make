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
include src/storage/CMakeFiles/storage_http_handler.dir/depend.make

# Include the progress variables for this target.
include src/storage/CMakeFiles/storage_http_handler.dir/progress.make

# Include the compile flags for this target's objects.
include src/storage/CMakeFiles/storage_http_handler.dir/flags.make

src/storage/CMakeFiles/storage_http_handler.dir/http/StorageHttpIngestHandler.cpp.o: src/storage/CMakeFiles/storage_http_handler.dir/flags.make
src/storage/CMakeFiles/storage_http_handler.dir/http/StorageHttpIngestHandler.cpp.o: ../src/storage/http/StorageHttpIngestHandler.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kvgroup/lmx/k2-graph/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/storage/CMakeFiles/storage_http_handler.dir/http/StorageHttpIngestHandler.cpp.o"
	cd /home/kvgroup/lmx/k2-graph/build/src/storage && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/storage_http_handler.dir/http/StorageHttpIngestHandler.cpp.o -c /home/kvgroup/lmx/k2-graph/src/storage/http/StorageHttpIngestHandler.cpp

src/storage/CMakeFiles/storage_http_handler.dir/http/StorageHttpIngestHandler.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/storage_http_handler.dir/http/StorageHttpIngestHandler.cpp.i"
	cd /home/kvgroup/lmx/k2-graph/build/src/storage && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kvgroup/lmx/k2-graph/src/storage/http/StorageHttpIngestHandler.cpp > CMakeFiles/storage_http_handler.dir/http/StorageHttpIngestHandler.cpp.i

src/storage/CMakeFiles/storage_http_handler.dir/http/StorageHttpIngestHandler.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/storage_http_handler.dir/http/StorageHttpIngestHandler.cpp.s"
	cd /home/kvgroup/lmx/k2-graph/build/src/storage && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kvgroup/lmx/k2-graph/src/storage/http/StorageHttpIngestHandler.cpp -o CMakeFiles/storage_http_handler.dir/http/StorageHttpIngestHandler.cpp.s

src/storage/CMakeFiles/storage_http_handler.dir/http/StorageHttpDownloadHandler.cpp.o: src/storage/CMakeFiles/storage_http_handler.dir/flags.make
src/storage/CMakeFiles/storage_http_handler.dir/http/StorageHttpDownloadHandler.cpp.o: ../src/storage/http/StorageHttpDownloadHandler.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kvgroup/lmx/k2-graph/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/storage/CMakeFiles/storage_http_handler.dir/http/StorageHttpDownloadHandler.cpp.o"
	cd /home/kvgroup/lmx/k2-graph/build/src/storage && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/storage_http_handler.dir/http/StorageHttpDownloadHandler.cpp.o -c /home/kvgroup/lmx/k2-graph/src/storage/http/StorageHttpDownloadHandler.cpp

src/storage/CMakeFiles/storage_http_handler.dir/http/StorageHttpDownloadHandler.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/storage_http_handler.dir/http/StorageHttpDownloadHandler.cpp.i"
	cd /home/kvgroup/lmx/k2-graph/build/src/storage && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kvgroup/lmx/k2-graph/src/storage/http/StorageHttpDownloadHandler.cpp > CMakeFiles/storage_http_handler.dir/http/StorageHttpDownloadHandler.cpp.i

src/storage/CMakeFiles/storage_http_handler.dir/http/StorageHttpDownloadHandler.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/storage_http_handler.dir/http/StorageHttpDownloadHandler.cpp.s"
	cd /home/kvgroup/lmx/k2-graph/build/src/storage && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kvgroup/lmx/k2-graph/src/storage/http/StorageHttpDownloadHandler.cpp -o CMakeFiles/storage_http_handler.dir/http/StorageHttpDownloadHandler.cpp.s

src/storage/CMakeFiles/storage_http_handler.dir/http/StorageHttpAdminHandler.cpp.o: src/storage/CMakeFiles/storage_http_handler.dir/flags.make
src/storage/CMakeFiles/storage_http_handler.dir/http/StorageHttpAdminHandler.cpp.o: ../src/storage/http/StorageHttpAdminHandler.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kvgroup/lmx/k2-graph/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object src/storage/CMakeFiles/storage_http_handler.dir/http/StorageHttpAdminHandler.cpp.o"
	cd /home/kvgroup/lmx/k2-graph/build/src/storage && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/storage_http_handler.dir/http/StorageHttpAdminHandler.cpp.o -c /home/kvgroup/lmx/k2-graph/src/storage/http/StorageHttpAdminHandler.cpp

src/storage/CMakeFiles/storage_http_handler.dir/http/StorageHttpAdminHandler.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/storage_http_handler.dir/http/StorageHttpAdminHandler.cpp.i"
	cd /home/kvgroup/lmx/k2-graph/build/src/storage && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kvgroup/lmx/k2-graph/src/storage/http/StorageHttpAdminHandler.cpp > CMakeFiles/storage_http_handler.dir/http/StorageHttpAdminHandler.cpp.i

src/storage/CMakeFiles/storage_http_handler.dir/http/StorageHttpAdminHandler.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/storage_http_handler.dir/http/StorageHttpAdminHandler.cpp.s"
	cd /home/kvgroup/lmx/k2-graph/build/src/storage && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kvgroup/lmx/k2-graph/src/storage/http/StorageHttpAdminHandler.cpp -o CMakeFiles/storage_http_handler.dir/http/StorageHttpAdminHandler.cpp.s

storage_http_handler: src/storage/CMakeFiles/storage_http_handler.dir/http/StorageHttpIngestHandler.cpp.o
storage_http_handler: src/storage/CMakeFiles/storage_http_handler.dir/http/StorageHttpDownloadHandler.cpp.o
storage_http_handler: src/storage/CMakeFiles/storage_http_handler.dir/http/StorageHttpAdminHandler.cpp.o
storage_http_handler: src/storage/CMakeFiles/storage_http_handler.dir/build.make

.PHONY : storage_http_handler

# Rule to build all files generated by this target.
src/storage/CMakeFiles/storage_http_handler.dir/build: storage_http_handler

.PHONY : src/storage/CMakeFiles/storage_http_handler.dir/build

src/storage/CMakeFiles/storage_http_handler.dir/clean:
	cd /home/kvgroup/lmx/k2-graph/build/src/storage && $(CMAKE_COMMAND) -P CMakeFiles/storage_http_handler.dir/cmake_clean.cmake
.PHONY : src/storage/CMakeFiles/storage_http_handler.dir/clean

src/storage/CMakeFiles/storage_http_handler.dir/depend:
	cd /home/kvgroup/lmx/k2-graph/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kvgroup/lmx/k2-graph /home/kvgroup/lmx/k2-graph/src/storage /home/kvgroup/lmx/k2-graph/build /home/kvgroup/lmx/k2-graph/build/src/storage /home/kvgroup/lmx/k2-graph/build/src/storage/CMakeFiles/storage_http_handler.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/storage/CMakeFiles/storage_http_handler.dir/depend

