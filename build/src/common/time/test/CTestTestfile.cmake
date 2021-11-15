# CMake generated Testfile for 
# Source directory: /home/kvgroup/lmx/k2-graph/src/common/time/test
# Build directory: /home/kvgroup/lmx/k2-graph/build/src/common/time/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(duration_test "/home/kvgroup/lmx/k2-graph/build/bin/test/duration_test")
set_tests_properties(duration_test PROPERTIES  LABELS "common/time" _BACKTRACE_TRIPLES "/home/kvgroup/lmx/k2-graph/CMakeLists.txt;453;add_test;/home/kvgroup/lmx/k2-graph/src/common/time/test/CMakeLists.txt;1;nebula_add_test;/home/kvgroup/lmx/k2-graph/src/common/time/test/CMakeLists.txt;0;")
add_test(wallclock_test "/home/kvgroup/lmx/k2-graph/build/bin/test/wallclock_test")
set_tests_properties(wallclock_test PROPERTIES  LABELS "common/time" _BACKTRACE_TRIPLES "/home/kvgroup/lmx/k2-graph/CMakeLists.txt;453;add_test;/home/kvgroup/lmx/k2-graph/src/common/time/test/CMakeLists.txt;14;nebula_add_test;/home/kvgroup/lmx/k2-graph/src/common/time/test/CMakeLists.txt;0;")
