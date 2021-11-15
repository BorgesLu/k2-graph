# CMake generated Testfile for 
# Source directory: /home/kvgroup/lmx/k2-graph/src/parser/test
# Build directory: /home/kvgroup/lmx/k2-graph/build/src/parser/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(parser_test "/home/kvgroup/lmx/k2-graph/build/bin/test/parser_test")
set_tests_properties(parser_test PROPERTIES  LABELS "parser" _BACKTRACE_TRIPLES "/home/kvgroup/lmx/k2-graph/CMakeLists.txt;453;add_test;/home/kvgroup/lmx/k2-graph/src/parser/test/CMakeLists.txt;21;nebula_add_test;/home/kvgroup/lmx/k2-graph/src/parser/test/CMakeLists.txt;0;")
add_test(scanner_test "/home/kvgroup/lmx/k2-graph/build/bin/test/scanner_test")
set_tests_properties(scanner_test PROPERTIES  LABELS "parser" _BACKTRACE_TRIPLES "/home/kvgroup/lmx/k2-graph/CMakeLists.txt;453;add_test;/home/kvgroup/lmx/k2-graph/src/parser/test/CMakeLists.txt;35;nebula_add_test;/home/kvgroup/lmx/k2-graph/src/parser/test/CMakeLists.txt;0;")
