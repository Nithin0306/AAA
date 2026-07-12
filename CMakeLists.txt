cmake_minimum_required(VERSION 3.16)
project(OptimalCachingAnalysis VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(cache_lib STATIC
    src/LRUCache.cpp
    src/NaiveLRU.cpp
    src/FIFOCache.cpp
    src/BeladyCache.cpp
    src/SLRU.cpp
)
target_include_directories(cache_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(cache_sim src/main.cpp)
target_link_libraries(cache_sim PRIVATE cache_lib)

add_executable(test_anomaly tests/test_beladys_anomaly.cpp)
target_link_libraries(test_anomaly PRIVATE cache_lib)

add_executable(test_pollution tests/test_cache_pollution.cpp)
target_link_libraries(test_pollution PRIVATE cache_lib)

file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/data)
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/results)
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/results/plots)
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/docs)

install(TARGETS cache_sim DESTINATION bin)
