cmake_minimum_required(VERSION 3.18)

project(atmolyt-host
    VERSION 0.1.0
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build types
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g3")

# Host vs target selection:
# - TARGET_HOST=ON  -> build with mock peripherals (host build)
# - TARGET_HOST=OFF -> target build (RPi/ARM), toolchain/sysroot must be provided externally
option(TARGET_HOST "Build for host with mock peripherals" OFF)

if(TARGET_HOST)
    message(STATUS "Configuring for host (mock peripherals)")
else()
    message(STATUS "Configuring for target (RPi/ARM). Provide toolchain externally via CMAKE_TOOLCHAIN_FILE or SDK environment.")
endif()

find_package(Boost 1.70 REQUIRED COMPONENTS
    filesystem
    system
    program_options)

set(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")

file(GLOB_RECURSE PROJECT_SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${REPO_ROOT}/src/*.cpp
)

if(TARGET_HOST)
    list(APPEND PROJECT_SOURCES
        ${REPO_ROOT}/src/peripheral/mock_environmental.cpp
        ${REPO_ROOT}/src/connections/mock_connection.cpp)
endif()

add_executable(atmolyt-host ${PROJECT_SOURCES})

target_include_directories(atmolyt-host PRIVATE
    ${REPO_ROOT}/inc
    ${REPO_ROOT}/inc/connecttions
    ${REPO_ROOT}/inc/peripheral
    ${REPO_ROOT}/inc/app
    ${CMAKE_SOURCE_DIR}/inc
)

target_compile_definitions(atmolyt-host PRIVATE TARGET_HOST=$<BOOL:${TARGET_HOST}>)

target_link_libraries(atmolyt-host PRIVATE
    Boost::filesystem
    Boost::system
    Boost::program_options
)

# Enable CTest
enable_testing()

# Add test executable
file(GLOB_RECURSE TEST_SOURCES
    ${REPO_ROOT}/tests/*.cpp
)

if(TEST_SOURCES)
    # Collect sources for tests (exclude main.cpp)
    set(TEST_LINK_SOURCES
        ${REPO_ROOT}/src/peripheral/bme280.cpp
        ${REPO_ROOT}/src/peripheral/peripheral_factory.cpp
        ${REPO_ROOT}/src/connections/mock_connection.cpp
        ${REPO_ROOT}/src/config/config_loader.cpp
    )

    add_executable(test_atmolyt ${TEST_SOURCES} ${TEST_LINK_SOURCES})
    target_include_directories(test_atmolyt PRIVATE
        ${REPO_ROOT}/inc
        ${REPO_ROOT}/inc/connections
        ${REPO_ROOT}/inc/peripheral
        ${REPO_ROOT}/inc/app
        ${REPO_ROOT}/inc/config
        ${CMAKE_SOURCE_DIR}/inc
        ${REPO_ROOT}/tests
    )
    target_link_libraries(test_atmolyt PRIVATE
        Boost::filesystem
        Boost::system
        Boost::program_options
    )

    # Compile with host target features for test
    add_compile_definitions(TARGET_HOST)

    add_test(NAME atmolyt_tests COMMAND test_atmolyt)
endif()

install(TARGETS atmolyt-host RUNTIME DESTINATION bin)
install(PROGRAMS ${REPO_ROOT}/scripts/start.sh ${REPO_ROOT}/scripts/debug.sh ${REPO_ROOT}/scripts/install.sh ${REPO_ROOT}/scripts/remove.sh DESTINATION scripts)
install(DIRECTORY ${REPO_ROOT}/configs/ DESTINATION config)

# Packaging with CPack - create a tarball which includes the binary and configs
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_NAME "atmolyt-host")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
include(CPack)
