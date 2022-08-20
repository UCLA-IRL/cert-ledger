# cmake version to be used
cmake_minimum_required(VERSION 3.12)

# project name
project(cert-ledger
        VERSION 0.1
        DESCRIPTION "A distributed ledger based on NDN")

# flags
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

# dependencies
find_package(leveldb REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(NDN_CXX REQUIRED libndn-cxx)
pkg_check_modules(NDN_SVS REQUIRED libndn-svs)


# files
set(CERT_LEDGER_LIB_SOURCE_FILES
        src/backend.cpp
        src/cert-ledger.cpp
        src/record.cpp
        src/config.cpp
        src/util.cpp)
# include
include_directories(${NDN_CXX_INCLUDE_DIRS})
include_directories(${NDN_SVS_INCLUDE_DIRS})

# link
link_directories(${NDN_CXX_LIBRARY_DIRS})
link_directories(${NDN_SVS_LIBRARY_DIRS})

# target

add_library(cert-ledger SHARED "")
target_sources(cert-ledger PRIVATE ${CERT_LEDGER_LIB_SOURCE_FILES})
target_include_directories(cert-ledger PUBLIC ./include)
target_include_directories(cert-ledger PRIVATE ./src)
target_compile_options(cert-ledger PUBLIC ${NDN_CXX_CFLAGS} ${NDN_SVS_CFLAGS})
target_link_libraries(cert-ledger PUBLIC ${NDN_CXX_LIBRARIES} ${NDN_SVS_LIBRARIES} leveldb)

add_subdirectory(test)

#configure
set(PKG_CONFIG_REQUIRES
        "libndn-cxx >= ${NDN_CXX_VERSION}"
        )
set(PKG_CONFIG_LIBS
        "-L\${libdir} -l${PROJECT_NAME}"
        )
set(PKG_CONFIG_CFLAGS
        "-I\${includedir}"
        )
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake-pkgconfig.pc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.pc")

include(GNUInstallDirs)
install(TARGETS cert-ledger
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
install(DIRECTORY include/cert-ledger DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h*")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.pc"
        DESTINATION lib/pkgconfig)