# CMake generated Testfile for 
# Source directory: /home/kvgroup/lmx/k2-graph/src/kvstore/wal/test
# Build directory: /home/kvgroup/lmx/k2-graph/build/src/kvstore/wal/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(file_based_wal_test "/home/kvgroup/lmx/k2-graph/build/bin/test/file_based_wal_test")
set_tests_properties(file_based_wal_test PROPERTIES  LABELS "kvstore/wal" _BACKTRACE_TRIPLES "/home/kvgroup/lmx/k2-graph/CMakeLists.txt;453;add_test;/home/kvgroup/lmx/k2-graph/src/kvstore/wal/test/CMakeLists.txt;1;nebula_add_test;/home/kvgroup/lmx/k2-graph/src/kvstore/wal/test/CMakeLists.txt;0;")
add_test(inmemory_log_buffer_test "/home/kvgroup/lmx/k2-graph/build/bin/test/inmemory_log_buffer_test")
set_tests_properties(inmemory_log_buffer_test PROPERTIES  LABELS "kvstore/wal" _BACKTRACE_TRIPLES "/home/kvgroup/lmx/k2-graph/CMakeLists.txt;453;add_test;/home/kvgroup/lmx/k2-graph/src/kvstore/wal/test/CMakeLists.txt;16;nebula_add_test;/home/kvgroup/lmx/k2-graph/src/kvstore/wal/test/CMakeLists.txt;0;")
