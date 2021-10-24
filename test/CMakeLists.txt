
add_executable(backend-test backend-test.cpp)
target_include_directories(backend-test PRIVATE ./src)
target_link_libraries(backend-test PUBLIC mnemosyne)

add_executable(record-test record-test.cpp)
target_link_libraries(record-test PUBLIC mnemosyne)

add_executable(dag-sync-test dag-sync-test.cpp)
target_link_libraries(dag-sync-test PUBLIC mnemosyne)

add_executable(mnemosyne-test-client mnemosyne-test-client.cpp)
target_link_libraries(mnemosyne-test-client PUBLIC mnemosyne)