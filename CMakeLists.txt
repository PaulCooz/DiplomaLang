cmake_minimum_required(VERSION 4.0.1)
project(DiplomaPKuz LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)

set(INTERPRETER_SOURCE "source/interpreter/console_interpreter.cpp" CACHE STRING "path to interpreter's source")

file(GLOB files "source/*.cpp")
foreach(file_path ${files})
    cmake_path(GET file_path STEM file_name)

    if(NOT ${file_name} STREQUAL "main")
        list(APPEND sources ${file_path})
    endif()
endforeach()

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR})

find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

add_executable(${PROJECT_NAME} "source/main.cpp" ${sources} ${INTERPRETER_SOURCE})
target_include_directories(${PROJECT_NAME} PRIVATE "interface")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "diploma")

llvm_map_components_to_libnames(llvm_libs core support)
target_link_libraries(${PROJECT_NAME} ${llvm_libs})

if(WITH_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY "https://github.com/google/googletest.git"
        GIT_TAG "v1.16.0"
        SOURCE_DIR "${PROJECT_SOURCE_DIR}/lib/googletest"
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    include(CTest)
    include(GoogleTest)

    file(GLOB files "test/*.cpp")
    foreach(file_path ${files})
        cmake_path(GET file_path STEM file_name)

        add_executable(${file_name} ${file_path} ${sources} ${INTERPRETER_SOURCE})
        target_compile_features(${file_name} PRIVATE cxx_std_23)
        target_include_directories(${file_name} PRIVATE "${PROJECT_SOURCE_DIR}/interface")
        target_link_libraries(${file_name} GTest::gtest_main)

        gtest_discover_tests(${file_name})
    endforeach()
endif()
