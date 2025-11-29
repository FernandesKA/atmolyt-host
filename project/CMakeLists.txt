cmake_minimum_required(VERSION 3.28)

project(atmolyt-host
    VERSION 0.1.0
    LANGUAGES CXX)

set(CMAKE_CXX_STANDART 23)
set(CMAKE_CXX_SANDART_REQUIRED ON)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto -march=native -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g3 -fsanitize=address,undefined")

find_package(Boost 1.83 REQUIRED COMPONENTS
    system
    filesystem
    program_options
)

# Modules
add_subdirectory(src/core)
add_subdirectory(src/hal)
add_subdirectory(src/sensors)
add_subdirectory(src/storage)
add_subdirectory(src/config)
add_subdirectory(src/alerts)
add_subdirectory(src/app)

add_executable(src/main.cpp)

target_link_libraries(atmolyt-host PRIVATE
    atmolyt::core
    atmolyt::hal
    atmolyt::sensors
    atmolyt::storage
    atmolyt::config
    atmolyt::alerts
    atmolyt::app
    Boost::system
    Boost::filesystem
)

install(TARGETS atmolyt-host RUNTIME DESTINATION bin)
install(FILES config/atmolyt.json DESTINATION /etc/atmolyt/)
install(DIRECTORY DESTINATION /var/lib/atmolyt)