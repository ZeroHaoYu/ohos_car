# the minimum version of CMake.
cmake_minimum_required(VERSION 3.4.1)
project(ai)

set(CMAKE_CXX_STANDARD 17)

set(NATIVE_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${NATIVE_ROOT_PATH}
                    ${NATIVE_ROOT_PATH}/include
                    ${NATIVE_ROOT_PATH}/third_party/rknn/runtime/Linux/librknn_api/include
                    )

add_library(rknnbenchmark SHARED
      src/main.cpp

      )


## libnnrt
add_library( libnnrt SHARED IMPORTED )
set_target_properties(libnnrt
        PROPERTIES IMPORTED_LOCATION
        ${NATIVE_ROOT_PATH}/third_party/rknn/lib/librknnrt.so)

set(FSRMODELZOO  rknnbenchmark)
foreach(EXE ${FSRMODELZOO})
  
    target_link_libraries(${EXE} PUBLIC   libnnrt
                                           )

endforeach()

