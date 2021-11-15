# CMake generated Testfile for 
# Source directory: /home/kvgroup/lmx/k2-graph/src/kvstore/raftex/test
# Build directory: /home/kvgroup/lmx/k2-graph/build/src/kvstore/raftex/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(leader_election_test "/home/kvgroup/lmx/k2-graph/build/bin/test/leader_election_test")
set_tests_properties(leader_election_test PROPERTIES  LABELS "kvstore/raftex" _BACKTRACE_TRIPLES "/home/kvgroup/lmx/k2-graph/CMakeLists.txt;453;add_test;/home/kvgroup/lmx/k2-graph/src/kvstore/raftex/test/CMakeLists.txt;15;nebula_add_test;/home/kvgroup/lmx/k2-graph/src/kvstore/raftex/test/CMakeLists.txt;0;")
add_test(log_append_test "/home/kvgroup/lmx/k2-graph/build/bin/test/log_append_test")
set_tests_properties(log_append_test PROPERTIES  LABELS "kvstore/raftex" _BACKTRACE_TRIPLES "/home/kvgroup/lmx/k2-graph/CMakeLists.txt;453;add_test;/home/kvgroup/lmx/k2-graph/src/kvstore/raftex/test/CMakeLists.txt;31;nebula_add_test;/home/kvgroup/lmx/k2-graph/src/kvstore/raftex/test/CMakeLists.txt;0;")
add_test(log_cas_test "/home/kvgroup/lmx/k2-graph/build/bin/test/log_cas_test")
set_tests_properties(log_cas_test PROPERTIES  LABELS "kvstore/raftex" _BACKTRACE_TRIPLES "/home/kvgroup/lmx/k2-graph/CMakeLists.txt;453;add_test;/home/kvgroup/lmx/k2-graph/src/kvstore/raftex/test/CMakeLists.txt;47;nebula_add_test;/home/kvgroup/lmx/k2-graph/src/kvstore/raftex/test/CMakeLists.txt;0;")
add_test(learner_test "/home/kvgroup/lmx/k2-graph/build/bin/test/learner_test")
set_tests_properties(learner_test PROPERTIES  LABELS "kvstore/raftex" _BACKTRACE_TRIPLES "/home/kvgroup/lmx/k2-graph/CMakeLists.txt;453;add_test;/home/kvgroup/lmx/k2-graph/src/kvstore/raftex/test/CMakeLists.txt;63;nebula_add_test;/home/kvgroup/lmx/k2-graph/src/kvstore/raftex/test/CMakeLists.txt;0;")
add_test(log_command_test "/home/kvgroup/lmx/k2-graph/build/bin/test/log_command_test")
set_tests_properties(log_command_test PROPERTIES  LABELS "kvstore/raftex" _BACKTRACE_TRIPLES "/home/kvgroup/lmx/k2-graph/CMakeLists.txt;453;add_test;/home/kvgroup/lmx/k2-graph/src/kvstore/raftex/test/CMakeLists.txt;79;nebula_add_test;/home/kvgroup/lmx/k2-graph/src/kvstore/raftex/test/CMakeLists.txt;0;")
add_test(raft_case_test "/home/kvgroup/lmx/k2-graph/build/bin/test/raft_case_test")
set_tests_properties(raft_case_test PROPERTIES  LABELS "kvstore/raftex" _BACKTRACE_TRIPLES "/home/kvgroup/lmx/k2-graph/CMakeLists.txt;453;add_test;/home/kvgroup/lmx/k2-graph/src/kvstore/raftex/test/CMakeLists.txt;94;nebula_add_test;/home/kvgroup/lmx/k2-graph/src/kvstore/raftex/test/CMakeLists.txt;0;")
add_test(leader_transfer_test "/home/kvgroup/lmx/k2-graph/build/bin/test/leader_transfer_test")
set_tests_properties(leader_transfer_test PROPERTIES  LABELS "kvstore/raftex" _BACKTRACE_TRIPLES "/home/kvgroup/lmx/k2-graph/CMakeLists.txt;453;add_test;/home/kvgroup/lmx/k2-graph/src/kvstore/raftex/test/CMakeLists.txt;109;nebula_add_test;/home/kvgroup/lmx/k2-graph/src/kvstore/raftex/test/CMakeLists.txt;0;")
add_test(snapshot_test "/home/kvgroup/lmx/k2-graph/build/bin/test/snapshot_test")
set_tests_properties(snapshot_test PROPERTIES  LABELS "kvstore/raftex" _BACKTRACE_TRIPLES "/home/kvgroup/lmx/k2-graph/CMakeLists.txt;453;add_test;/home/kvgroup/lmx/k2-graph/src/kvstore/raftex/test/CMakeLists.txt;124;nebula_add_test;/home/kvgroup/lmx/k2-graph/src/kvstore/raftex/test/CMakeLists.txt;0;")
add_test(member_change_test "/home/kvgroup/lmx/k2-graph/build/bin/test/member_change_test")
set_tests_properties(member_change_test PROPERTIES  LABELS "kvstore/raftex" _BACKTRACE_TRIPLES "/home/kvgroup/lmx/k2-graph/CMakeLists.txt;453;add_test;/home/kvgroup/lmx/k2-graph/src/kvstore/raftex/test/CMakeLists.txt;139;nebula_add_test;/home/kvgroup/lmx/k2-graph/src/kvstore/raftex/test/CMakeLists.txt;0;")
