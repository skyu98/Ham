add_executable(chargenServer chargen/chargen.cc chargen/main.cc)
target_link_libraries(chargenServer ham_net)