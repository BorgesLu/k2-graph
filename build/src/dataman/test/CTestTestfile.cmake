# CMake generated Testfile for 
# Source directory: /home/kvgroup/lmx/k2-graph/src/dataman/test
# Build directory: /home/kvgroup/lmx/k2-graph/build/src/dataman/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(row_reader_test "/home/kvgroup/lmx/k2-graph/build/bin/test/row_reader_test")
set_tests_properties(row_reader_test PROPERTIES  LABELS "dataman" _BACKTRACE_TRIPLES "/home/kvgroup/lmx/k2-graph/CMakeLists.txt;453;add_test;/home/kvgroup/lmx/k2-graph/src/dataman/test/CMakeLists.txt;12;nebula_add_test;/home/kvgroup/lmx/k2-graph/src/dataman/test/CMakeLists.txt;0;")
add_test(row_writer_test "/home/kvgroup/lmx/k2-graph/build/bin/test/row_writer_test")
set_tests_properties(row_writer_test PROPERTIES  LABELS "dataman" _BACKTRACE_TRIPLES "/home/kvgroup/lmx/k2-graph/CMakeLists.txt;453;add_test;/home/kvgroup/lmx/k2-graph/src/dataman/test/CMakeLists.txt;20;nebula_add_test;/home/kvgroup/lmx/k2-graph/src/dataman/test/CMakeLists.txt;0;")
add_test(row_updater_test "/home/kvgroup/lmx/k2-graph/build/bin/test/row_updater_test")
set_tests_properties(row_updater_test PROPERTIES  LABELS "dataman" _BACKTRACE_TRIPLES "/home/kvgroup/lmx/k2-graph/CMakeLists.txt;453;add_test;/home/kvgroup/lmx/k2-graph/src/dataman/test/CMakeLists.txt;28;nebula_add_test;/home/kvgroup/lmx/k2-graph/src/dataman/test/CMakeLists.txt;0;")
add_test(rowset_reader_writer_test "/home/kvgroup/lmx/k2-graph/build/bin/test/rowset_reader_writer_test")
set_tests_properties(rowset_reader_writer_test PROPERTIES  LABELS "dataman" _BACKTRACE_TRIPLES "/home/kvgroup/lmx/k2-graph/CMakeLists.txt;453;add_test;/home/kvgroup/lmx/k2-graph/src/dataman/test/CMakeLists.txt;36;nebula_add_test;/home/kvgroup/lmx/k2-graph/src/dataman/test/CMakeLists.txt;0;")
add_test(nebula_codec_test "/home/kvgroup/lmx/k2-graph/build/bin/test/nebula_codec_test")
set_tests_properties(nebula_codec_test PROPERTIES  LABELS "dataman" _BACKTRACE_TRIPLES "/home/kvgroup/lmx/k2-graph/CMakeLists.txt;453;add_test;/home/kvgroup/lmx/k2-graph/src/dataman/test/CMakeLists.txt;59;nebula_add_test;/home/kvgroup/lmx/k2-graph/src/dataman/test/CMakeLists.txt;0;")
