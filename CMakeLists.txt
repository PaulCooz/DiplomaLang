cmake_minimum_required(VERSION 4.0.1)
project(SuperBuild LANGUAGES C CXX)

include(ExternalProject)

if(USE_LLVM)
    ExternalProject_Add(
        LLVM

        PREFIX "${PROJECT_SOURCE_DIR}/lib/LLVM"
        SOURCE_DIR "${PROJECT_SOURCE_DIR}/lib/llvm-project"
        SOURCE_SUBDIR "llvm"
        BINARY_DIR "${PROJECT_SOURCE_DIR}/lib/LLVM/build"
        INSTALL_DIR "${PROJECT_SOURCE_DIR}/lib/LLVM/install"

        GIT_REPOSITORY "https://github.com/llvm/llvm-project.git"
        GIT_TAG "llvmorg-21.1.8"
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE

        CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DLLVM_TARGETS_TO_BUILD=X86
        -DLLVM_BUILD_UTILS=OFF
        -DLLVM_INCLUDE_BENCHMARKS=OFF
        -DLLVM_INCLUDE_TESTS=OFF
        -DLLVM_INCLUDE_EXAMPLES=OFF
        -DCMAKE_INSTALL_PREFIX=${PROJECT_SOURCE_DIR}/lib/LLVM/install
        BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --parallel
        INSTALL_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --target install
    )
endif()

ExternalProject_Add(
    frontend
    DEPENDS LLVM

    PREFIX "${PROJECT_SOURCE_DIR}/build/frontend"
    BINARY_DIR "${PROJECT_SOURCE_DIR}/build/frontend"
    SOURCE_DIR "${PROJECT_SOURCE_DIR}/frontend"

    INSTALL_COMMAND ""
    BUILD_ALWAYS TRUE

    CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DUSE_LLVM=${USE_LLVM}
    -DWITH_TESTS=${WITH_TESTS}
    -DLLVM_DIR=${PROJECT_SOURCE_DIR}/lib/LLVM/install/lib/cmake/llvm
)
