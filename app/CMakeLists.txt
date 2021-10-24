
add_executable(mnemosyne-logger mnemosyne-logger.cpp)
target_link_libraries(mnemosyne-logger PUBLIC mnemosyne)

install(TARGETS mnemosyne-logger
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})