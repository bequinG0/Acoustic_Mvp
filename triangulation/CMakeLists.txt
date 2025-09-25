cmake_minimum_required(VERSION 3.10)

project(triangulation LANGUAGES CXX)

# стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
# предупреждения и оптимизация
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# путь к include/
include_directories(${PROJECT_SOURCE_DIR}/include)

# собираем все cpp
file(GLOB SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/main.cpp
)

# исполняемый файл
add_executable(${PROJECT_NAME} ${SOURCES})

# ищем и подключаем hiredis
find_package(PkgConfig REQUIRED)
pkg_check_modules(HIREDIS REQUIRED hiredis)

target_include_directories(${PROJECT_NAME} PRIVATE ${HIREDIS_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${HIREDIS_LIBRARIES})

