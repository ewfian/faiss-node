cmake_minimum_required(VERSION 3.17 FATAL_ERROR)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0042 NEW)

project(faiss-node LANGUAGES CXX)

# set(OpenMP_CXX_FLAGS "-Xclang -fopenmp -I/usr/local/Cellar/libomp/14.0.0/include")
# set(OpenMP_CXX_LIB_NAMES "libomp")
# set(OpenMP_libomp_LIBRARY "omp")

if (APPLE)
    if (CMAKE_C_COMPILER_ID MATCHES "Clang")
        set(OpenMP_C "${CMAKE_C_COMPILER}")
        set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp -Wno-unused-command-line-argument -I/opt/homebrew/opt/libomp/include")
        set(OpenMP_C_LIB_NAMES "libomp")   
        set(OpenMP_libomp_LIBRARY "/opt/homebrew/opt/libomp/lib/libomp.dylib")
    endif ()
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(OpenMP_CXX "${CMAKE_CXX_COMPILER}")
        set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -Wno-unused-command-line-argument -I/opt/homebrew/opt/libomp/include")
        set(OpenMP_CXX_LIB_NAMES "libomp")
        set(OpenMP_libomp_LIBRARY "/opt/homebrew/opt/libomp/lib/libomp.dylib")
    endif ()
endif ()

if(WIN32)
    set(BUILD_TESTING OFF)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

set(CMAKE_CXX_STANDARD 11)
set(FAISS_ENABLE_GPU OFF)
set(FAISS_ENABLE_PYTHON OFF)

if(DEFINED <napi_build_version>)
    message("napi_build_version: " ${napi_build_version})
    add_compile_definitions(NAPI_VERSION=${napi_build_version})
else()
    add_definitions(-DNAPI_VERSION=6)
endif()

include_directories(${CMAKE_JS_INC})
file(GLOB SOURCE_FILES "src/*.cc" "src/*.h")

add_subdirectory("deps/faiss")
set_target_properties(faiss PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>
    LIBRARY_OUTPUT_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>
    RUNTIME_OUTPUT_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/node_modules/node-addon-api")
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB} faiss)

if(MSVC AND CMAKE_JS_NODELIB_DEF AND CMAKE_JS_NODELIB_TARGET)
  # Generate node.lib
  execute_process(COMMAND ${CMAKE_AR} /def:${CMAKE_JS_NODELIB_DEF} /out:${CMAKE_JS_NODELIB_TARGET} ${CMAKE_STATIC_LINKER_FLAGS})
endif()

if(WIN32)
    # Copy mkl_sequential.2.dll
    if(DEFINED ENV{MKLROOT})
        file(TO_CMAKE_PATH "$ENV{MKLROOT}\\redist\\intel64\\mkl_sequential.2.dll" MKL_sequential)
        message("MKL_sequential: " ${MKL_sequential})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${MKL_sequential}
                $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    endif()
endif()