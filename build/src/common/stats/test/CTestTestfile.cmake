# CMake generated Testfile for 
# Source directory: /home/kvgroup/lmx/k2-graph/src/common/stats/test
# Build directory: /home/kvgroup/lmx/k2-graph/build/src/common/stats/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(stats_manager_test "/home/kvgroup/lmx/k2-graph/build/bin/test/stats_manager_test")
set_tests_properties(stats_manager_test PROPERTIES  LABELS "common/stats" _BACKTRACE_TRIPLES "/home/kvgroup/lmx/k2-graph/CMakeLists.txt;453;add_test;/home/kvgroup/lmx/k2-graph/src/common/stats/test/CMakeLists.txt;1;nebula_add_test;/home/kvgroup/lmx/k2-graph/src/common/stats/test/CMakeLists.txt;0;")
add_test(stats_manager_rate_test "/home/kvgroup/lmx/k2-graph/build/bin/test/stats_manager_rate_test")
set_tests_properties(stats_manager_rate_test PROPERTIES  LABELS "common/stats" _BACKTRACE_TRIPLES "/home/kvgroup/lmx/k2-graph/CMakeLists.txt;453;add_test;/home/kvgroup/lmx/k2-graph/src/common/stats/test/CMakeLists.txt;15;nebula_add_test;/home/kvgroup/lmx/k2-graph/src/common/stats/test/CMakeLists.txt;0;")
add_test(stats_manager_cross_level_test "/home/kvgroup/lmx/k2-graph/build/bin/test/stats_manager_cross_level_test")
set_tests_properties(stats_manager_cross_level_test PROPERTIES  LABELS "common/stats" _BACKTRACE_TRIPLES "/home/kvgroup/lmx/k2-graph/CMakeLists.txt;453;add_test;/home/kvgroup/lmx/k2-graph/src/common/stats/test/CMakeLists.txt;29;nebula_add_test;/home/kvgroup/lmx/k2-graph/src/common/stats/test/CMakeLists.txt;0;")
