
add_executable(cert_ledger-logger cert-ledger-ca.cpp)
target_link_libraries(cert_ledger-logger PUBLIC cert-ledger)

install(TARGETS cert_ledger-logger
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})