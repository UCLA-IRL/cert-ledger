
add_executable(backend-test backend-test.cpp)
target_include_directories(backend-test PRIVATE ./src)
target_link_libraries(backend-test PUBLIC cert-ledger)

add_executable(record-test record-test.cpp)
target_link_libraries(record-test PUBLIC cert-ledger)

add_executable(cert-ledger-test cert-ledger-test.cpp)
target_link_libraries(cert-ledger-test PUBLIC cert-ledger)