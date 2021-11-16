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
include src/common/base/test/CMakeFiles/logging_bm.dir/depend.make

# Include the progress variables for this target.
include src/common/base/test/CMakeFiles/logging_bm.dir/progress.make

# Include the compile flags for this target's objects.
include src/common/base/test/CMakeFiles/logging_bm.dir/flags.make

src/common/base/test/CMakeFiles/logging_bm.dir/LoggingBenchmark.cpp.o: src/common/base/test/CMakeFiles/logging_bm.dir/flags.make
src/common/base/test/CMakeFiles/logging_bm.dir/LoggingBenchmark.cpp.o: ../src/common/base/test/LoggingBenchmark.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kvgroup/lmx/k2-graph/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/common/base/test/CMakeFiles/logging_bm.dir/LoggingBenchmark.cpp.o"
	cd /home/kvgroup/lmx/k2-graph/build/src/common/base/test && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/logging_bm.dir/LoggingBenchmark.cpp.o -c /home/kvgroup/lmx/k2-graph/src/common/base/test/LoggingBenchmark.cpp

src/common/base/test/CMakeFiles/logging_bm.dir/LoggingBenchmark.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/logging_bm.dir/LoggingBenchmark.cpp.i"
	cd /home/kvgroup/lmx/k2-graph/build/src/common/base/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kvgroup/lmx/k2-graph/src/common/base/test/LoggingBenchmark.cpp > CMakeFiles/logging_bm.dir/LoggingBenchmark.cpp.i

src/common/base/test/CMakeFiles/logging_bm.dir/LoggingBenchmark.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/logging_bm.dir/LoggingBenchmark.cpp.s"
	cd /home/kvgroup/lmx/k2-graph/build/src/common/base/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kvgroup/lmx/k2-graph/src/common/base/test/LoggingBenchmark.cpp -o CMakeFiles/logging_bm.dir/LoggingBenchmark.cpp.s

# Object files for target logging_bm
logging_bm_OBJECTS = \
"CMakeFiles/logging_bm.dir/LoggingBenchmark.cpp.o"

# External object files for target logging_bm
logging_bm_EXTERNAL_OBJECTS = \
"/home/kvgroup/lmx/k2-graph/build/src/common/base/CMakeFiles/base_obj.dir/Base.cpp.o" \
"/home/kvgroup/lmx/k2-graph/build/src/common/base/CMakeFiles/base_obj.dir/Cord.cpp.o" \
"/home/kvgroup/lmx/k2-graph/build/src/common/base/CMakeFiles/base_obj.dir/Configuration.cpp.o" \
"/home/kvgroup/lmx/k2-graph/build/src/common/base/CMakeFiles/base_obj.dir/Status.cpp.o" \
"/home/kvgroup/lmx/k2-graph/build/src/common/base/CMakeFiles/base_obj.dir/SanitizerOptions.cpp.o" \
"/home/kvgroup/lmx/k2-graph/build/src/common/base/CMakeFiles/base_obj.dir/SignalHandler.cpp.o" \
"/home/kvgroup/lmx/k2-graph/build/src/common/base/CMakeFiles/base_obj.dir/SlowOpTracker.cpp.o"

bin/bench/logging_bm: src/common/base/test/CMakeFiles/logging_bm.dir/LoggingBenchmark.cpp.o
bin/bench/logging_bm: src/common/base/CMakeFiles/base_obj.dir/Base.cpp.o
bin/bench/logging_bm: src/common/base/CMakeFiles/base_obj.dir/Cord.cpp.o
bin/bench/logging_bm: src/common/base/CMakeFiles/base_obj.dir/Configuration.cpp.o
bin/bench/logging_bm: src/common/base/CMakeFiles/base_obj.dir/Status.cpp.o
bin/bench/logging_bm: src/common/base/CMakeFiles/base_obj.dir/SanitizerOptions.cpp.o
bin/bench/logging_bm: src/common/base/CMakeFiles/base_obj.dir/SignalHandler.cpp.o
bin/bench/logging_bm: src/common/base/CMakeFiles/base_obj.dir/SlowOpTracker.cpp.o
bin/bench/logging_bm: src/common/base/test/CMakeFiles/logging_bm.dir/build.make
bin/bench/logging_bm: /opt/vesoft/third-party/lib/libssl.a
bin/bench/logging_bm: /opt/vesoft/third-party/lib/libcrypto.a
bin/bench/logging_bm: /opt/vesoft/third-party/lib/libgssapi_krb5.a
bin/bench/logging_bm: /opt/vesoft/third-party/lib/libkrb5.a
bin/bench/logging_bm: /opt/vesoft/third-party/lib/libk5crypto.a
bin/bench/logging_bm: /opt/vesoft/third-party/lib/libcom_err.a
bin/bench/logging_bm: /opt/vesoft/third-party/lib/libkrb5support.a
bin/bench/logging_bm: /opt/vesoft/third-party/lib/liblzma.a
bin/bench/logging_bm: /opt/vesoft/third-party/lib/libunwind.a
bin/bench/logging_bm: src/common/base/test/CMakeFiles/logging_bm.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/kvgroup/lmx/k2-graph/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../../../bin/bench/logging_bm"
	cd /home/kvgroup/lmx/k2-graph/build/src/common/base/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/logging_bm.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/common/base/test/CMakeFiles/logging_bm.dir/build: bin/bench/logging_bm

.PHONY : src/common/base/test/CMakeFiles/logging_bm.dir/build

src/common/base/test/CMakeFiles/logging_bm.dir/clean:
	cd /home/kvgroup/lmx/k2-graph/build/src/common/base/test && $(CMAKE_COMMAND) -P CMakeFiles/logging_bm.dir/cmake_clean.cmake
.PHONY : src/common/base/test/CMakeFiles/logging_bm.dir/clean

src/common/base/test/CMakeFiles/logging_bm.dir/depend:
	cd /home/kvgroup/lmx/k2-graph/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kvgroup/lmx/k2-graph /home/kvgroup/lmx/k2-graph/src/common/base/test /home/kvgroup/lmx/k2-graph/build /home/kvgroup/lmx/k2-graph/build/src/common/base/test /home/kvgroup/lmx/k2-graph/build/src/common/base/test/CMakeFiles/logging_bm.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/common/base/test/CMakeFiles/logging_bm.dir/depend
