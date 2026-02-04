add_library(lab2_core
    alphabet_index.cpp
)

target_include_directories(lab2_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(alphabet_cli alphabet_cli.cpp)
target_link_libraries(alphabet_cli PRIVATE lab2_core)
target_include_directories(alphabet_cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
