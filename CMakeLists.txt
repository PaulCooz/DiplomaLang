cmake_minimum_required(VERSION 3.31.6)
project(DiplomaPKuzmenko LANGUAGES C CXX)

add_executable(${PROJECT_NAME} code/main.cpp)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "diploma")
