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

# Utility rule file for graph_thrift_headers.

# Include the progress variables for this target.
include src/interface/CMakeFiles/graph_thrift_headers.dir/progress.make

src/interface/CMakeFiles/graph_thrift_headers: src/interface/gen-cpp2/graph_constants.h
src/interface/CMakeFiles/graph_thrift_headers: src/interface/gen-cpp2/graph_data.h
src/interface/CMakeFiles/graph_thrift_headers: src/interface/gen-cpp2/graph_types.h
src/interface/CMakeFiles/graph_thrift_headers: src/interface/gen-cpp2/graph_types_custom_protocol.h
src/interface/CMakeFiles/graph_thrift_headers: src/interface/gen-cpp2/graph_types.tcc
src/interface/CMakeFiles/graph_thrift_headers: src/interface/gen-cpp2/GraphService.h
src/interface/CMakeFiles/graph_thrift_headers: src/interface/gen-cpp2/GraphService.tcc
src/interface/CMakeFiles/graph_thrift_headers: src/interface/gen-cpp2/GraphServiceAsyncClient.h
src/interface/CMakeFiles/graph_thrift_headers: src/interface/gen-cpp2/GraphService_custom_protocol.h


src/interface/gen-cpp2/graph_constants.h: ../src/interface/graph.thrift
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/kvgroup/lmx/k2-graph/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating thrift files for graph"
	cd /home/kvgroup/lmx/k2-graph/build/src/interface && /opt/vesoft/third-party/bin/thrift1 --strict --allow-neg-enum-vals --templates /opt/vesoft/third-party/include/thrift/templates --gen mstch_cpp2:include_prefix=interface,process_in_event_base,stack_arguments --gen py --gen java:hashcode --gen go:thrift_import=github.com/facebook/fbthrift/thrift/lib/go/thrift,package_prefix=github.com/vesoft-inc/nebula-go/ -o . /home/kvgroup/lmx/k2-graph/src/interface/graph.thrift

src/interface/gen-cpp2/graph_data.h: src/interface/gen-cpp2/graph_constants.h
	@$(CMAKE_COMMAND) -E touch_nocreate src/interface/gen-cpp2/graph_data.h

src/interface/gen-cpp2/graph_types.h: src/interface/gen-cpp2/graph_constants.h
	@$(CMAKE_COMMAND) -E touch_nocreate src/interface/gen-cpp2/graph_types.h

src/interface/gen-cpp2/graph_types_custom_protocol.h: src/interface/gen-cpp2/graph_constants.h
	@$(CMAKE_COMMAND) -E touch_nocreate src/interface/gen-cpp2/graph_types_custom_protocol.h

src/interface/gen-cpp2/graph_types.tcc: src/interface/gen-cpp2/graph_constants.h
	@$(CMAKE_COMMAND) -E touch_nocreate src/interface/gen-cpp2/graph_types.tcc

src/interface/gen-cpp2/GraphService.h: src/interface/gen-cpp2/graph_constants.h
	@$(CMAKE_COMMAND) -E touch_nocreate src/interface/gen-cpp2/GraphService.h

src/interface/gen-cpp2/GraphService.tcc: src/interface/gen-cpp2/graph_constants.h
	@$(CMAKE_COMMAND) -E touch_nocreate src/interface/gen-cpp2/GraphService.tcc

src/interface/gen-cpp2/GraphServiceAsyncClient.h: src/interface/gen-cpp2/graph_constants.h
	@$(CMAKE_COMMAND) -E touch_nocreate src/interface/gen-cpp2/GraphServiceAsyncClient.h

src/interface/gen-cpp2/GraphService_custom_protocol.h: src/interface/gen-cpp2/graph_constants.h
	@$(CMAKE_COMMAND) -E touch_nocreate src/interface/gen-cpp2/GraphService_custom_protocol.h

src/interface/gen-cpp2/graph_constants.cpp: src/interface/gen-cpp2/graph_constants.h
	@$(CMAKE_COMMAND) -E touch_nocreate src/interface/gen-cpp2/graph_constants.cpp

src/interface/gen-cpp2/graph_data.cpp: src/interface/gen-cpp2/graph_constants.h
	@$(CMAKE_COMMAND) -E touch_nocreate src/interface/gen-cpp2/graph_data.cpp

src/interface/gen-cpp2/graph_types.cpp: src/interface/gen-cpp2/graph_constants.h
	@$(CMAKE_COMMAND) -E touch_nocreate src/interface/gen-cpp2/graph_types.cpp

src/interface/gen-cpp2/GraphService.cpp: src/interface/gen-cpp2/graph_constants.h
	@$(CMAKE_COMMAND) -E touch_nocreate src/interface/gen-cpp2/GraphService.cpp

src/interface/gen-cpp2/GraphServiceAsyncClient.cpp: src/interface/gen-cpp2/graph_constants.h
	@$(CMAKE_COMMAND) -E touch_nocreate src/interface/gen-cpp2/GraphServiceAsyncClient.cpp

src/interface/gen-cpp2/GraphService_processmap_binary.cpp: src/interface/gen-cpp2/graph_constants.h
	@$(CMAKE_COMMAND) -E touch_nocreate src/interface/gen-cpp2/GraphService_processmap_binary.cpp

src/interface/gen-cpp2/GraphService_processmap_compact.cpp: src/interface/gen-cpp2/graph_constants.h
	@$(CMAKE_COMMAND) -E touch_nocreate src/interface/gen-cpp2/GraphService_processmap_compact.cpp

graph_thrift_headers: src/interface/CMakeFiles/graph_thrift_headers
graph_thrift_headers: src/interface/gen-cpp2/graph_constants.h
graph_thrift_headers: src/interface/gen-cpp2/graph_data.h
graph_thrift_headers: src/interface/gen-cpp2/graph_types.h
graph_thrift_headers: src/interface/gen-cpp2/graph_types_custom_protocol.h
graph_thrift_headers: src/interface/gen-cpp2/graph_types.tcc
graph_thrift_headers: src/interface/gen-cpp2/GraphService.h
graph_thrift_headers: src/interface/gen-cpp2/GraphService.tcc
graph_thrift_headers: src/interface/gen-cpp2/GraphServiceAsyncClient.h
graph_thrift_headers: src/interface/gen-cpp2/GraphService_custom_protocol.h
graph_thrift_headers: src/interface/gen-cpp2/graph_constants.cpp
graph_thrift_headers: src/interface/gen-cpp2/graph_data.cpp
graph_thrift_headers: src/interface/gen-cpp2/graph_types.cpp
graph_thrift_headers: src/interface/gen-cpp2/GraphService.cpp
graph_thrift_headers: src/interface/gen-cpp2/GraphServiceAsyncClient.cpp
graph_thrift_headers: src/interface/gen-cpp2/GraphService_processmap_binary.cpp
graph_thrift_headers: src/interface/gen-cpp2/GraphService_processmap_compact.cpp
graph_thrift_headers: src/interface/CMakeFiles/graph_thrift_headers.dir/build.make

.PHONY : graph_thrift_headers

# Rule to build all files generated by this target.
src/interface/CMakeFiles/graph_thrift_headers.dir/build: graph_thrift_headers

.PHONY : src/interface/CMakeFiles/graph_thrift_headers.dir/build

src/interface/CMakeFiles/graph_thrift_headers.dir/clean:
	cd /home/kvgroup/lmx/k2-graph/build/src/interface && $(CMAKE_COMMAND) -P CMakeFiles/graph_thrift_headers.dir/cmake_clean.cmake
.PHONY : src/interface/CMakeFiles/graph_thrift_headers.dir/clean

src/interface/CMakeFiles/graph_thrift_headers.dir/depend:
	cd /home/kvgroup/lmx/k2-graph/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kvgroup/lmx/k2-graph /home/kvgroup/lmx/k2-graph/src/interface /home/kvgroup/lmx/k2-graph/build /home/kvgroup/lmx/k2-graph/build/src/interface /home/kvgroup/lmx/k2-graph/build/src/interface/CMakeFiles/graph_thrift_headers.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/interface/CMakeFiles/graph_thrift_headers.dir/depend

