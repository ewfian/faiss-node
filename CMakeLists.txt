cmake_minimum_required(VERSION 3.17 FATAL_ERROR)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0042 NEW)

project(faiss-node LANGUAGES CXX)

if (APPLE)
    find_package(OpenMP)
    if (NOT OpenMP_FOUND)
        execute_process(COMMAND brew --prefix libomp
            OUTPUT_VARIABLE HOMEBREW_LIBOMP_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        if (CMAKE_C_COMPILER_ID MATCHES "Clang")
            set(OpenMP_C "${CMAKE_C_COMPILER}")
            set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp -Wno-unused-command-line-argument -I${HOMEBREW_LIBOMP_PREFIX}/include")
            set(OpenMP_C_LIB_NAMES "libomp")   
            set(OpenMP_libomp_LIBRARY "${HOMEBREW_LIBOMP_PREFIX}/lib/libomp.dylib")
        endif ()
        if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            set(OpenMP_CXX "${CMAKE_CXX_COMPILER}")
            set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -Wno-unused-command-line-argument -I${HOMEBREW_LIBOMP_PREFIX}/include")
            set(OpenMP_CXX_LIB_NAMES "libomp")
            set(OpenMP_libomp_LIBRARY "${HOMEBREW_LIBOMP_PREFIX}/lib/libomp.dylib")
        endif ()
    endif ()
endif ()

# if(WIN32)
set(BUILD_TESTING OFF)
# endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

set(CMAKE_CXX_STANDARD 17)
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

set(SOURCE ${SOURCE_FILES})

add_subdirectory("deps/faiss")
set_target_properties(faiss PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>
    LIBRARY_OUTPUT_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>
    RUNTIME_OUTPUT_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # https://stackoverflow.com/questions/43330165/how-to-link-a-shared-library-with-cmake-with-relative-path/69707790#69707790
    set_target_properties(${PROJECT_NAME} PROPERTIES
            BUILD_WITH_INSTALL_RPATH FALSE
            LINK_FLAGS "-Wl,-rpath,$ORIGIN/")
endif()
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

# https://gitlab.kitware.com/cmake/community/-/wikis/doc/tutorials/How-To-Write-Platform-Checks#cmake-system
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Copy libmkl_intel_lp64.so, libmkl_sequential.so, libmkl_core.so
    if(DEFINED ENV{MKLROOT})
        set(DEPENDANCES_MKL
            libmkl_intel_lp64.so
            libmkl_sequential.so
            libmkl_core.so)
        foreach(dep ${DEPENDANCES_MKL})
            file(TO_CMAKE_PATH "$ENV{MKLROOT}/lib/intel64/${dep}" MKL_dep)
            message("Copy DEPENDANCES_MKL: " ${MKL_dep})
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${MKL_dep}
                    $<TARGET_FILE_DIR:${PROJECT_NAME}>)
        endforeach()
    endif()
    if(EXISTS "/etc/debian_version")
        file(READ "/etc/issue" ETC_ISSUE)
        string(REGEX MATCH "Debian|Ubuntu" DIST ${ETC_ISSUE})
        if(DIST STREQUAL "Debian")
            # Debian
            EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
            message("Debian:" ${ARCHITECTURE})
            if( ${ARCHITECTURE} STREQUAL "x86_64" )
                # Copy libgomp.so.1, libopenblas.so.0, libgfortran.so.5, libquadmath.so.0
                set(DEPENDANCES
                    libgomp.so.1
                    libopenblas.so.0
                    libgfortran.so.5
                    libquadmath.so.0)
                foreach(dep ${DEPENDANCES})
                    file(TO_CMAKE_PATH "/usr/lib/x86_64-linux-gnu/${dep}" MUSL_dep)
                    message("Copy DEPENDANCES: " ${MUSL_dep})
                    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            ${MUSL_dep}
                            $<TARGET_FILE_DIR:${PROJECT_NAME}>)
                endforeach()
                add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND patchelf --set-rpath "\\$$ORIGIN/" $<TARGET_FILE_DIR:${PROJECT_NAME}>/libopenblas.so.0
                    COMMAND patchelf --set-rpath "\\$$ORIGIN/" $<TARGET_FILE_DIR:${PROJECT_NAME}>/libgfortran.so.5
                )
            else()
                # Copy libgomp.so.1, libopenblas.so.0, libgfortran.so.5
                set(DEPENDANCES
                    libgomp.so.1
                    libopenblas.so.0
                    libgfortran.so.5)
                foreach(dep ${DEPENDANCES})
                    file(TO_CMAKE_PATH "/lib/aarch64-linux-gnu/${dep}" MUSL_dep)
                    message("Copy DEPENDANCES: " ${MUSL_dep})
                    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            ${MUSL_dep}
                            $<TARGET_FILE_DIR:${PROJECT_NAME}>)
                endforeach()
                add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND patchelf --set-rpath "\\$$ORIGIN/" $<TARGET_FILE_DIR:${PROJECT_NAME}>/libopenblas.so.0
                    COMMAND patchelf --set-rpath "\\$$ORIGIN/" $<TARGET_FILE_DIR:${PROJECT_NAME}>/libgfortran.so.5
                )
            endif()
        endif()
    endif()
    if(EXISTS "/etc/alpine-release")
        # Alpine
        EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
        message("Alpine:" ${ARCHITECTURE})
        if( ${ARCHITECTURE} STREQUAL "x86_64" )
            # Copy libgomp.so.1, libopenblas.so.3, libgfortran.so.5, libquadmath.so.0
            set(DEPENDANCES_MUSL
                libgomp.so.1
                libopenblas.so.3
                libgfortran.so.5
                libquadmath.so.0)
        else()
            # Copy libgomp.so.1, libopenblas.so.3, libgfortran.so.5
            set(DEPENDANCES_MUSL
                libgomp.so.1
                libopenblas.so.3
                libgfortran.so.5)
        endif()
        foreach(dep ${DEPENDANCES_MUSL})
            file(TO_CMAKE_PATH "/usr/lib/${dep}" MUSL_dep)
            message("Copy DEPENDANCES_MUSL: " ${MUSL_dep})
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${MUSL_dep}
                    $<TARGET_FILE_DIR:${PROJECT_NAME}>)
        endforeach()
    endif()
endif()

if(APPLE)
    # Copy libomp.dylib
    message("OpenMP_libomp_LIBRARY: " ${OpenMP_libomp_LIBRARY})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${OpenMP_libomp_LIBRARY}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    # fix loader path
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change ${OpenMP_libomp_LIBRARY} @rpath/libomp.dylib
        $<TARGET_FILE:${PROJECT_NAME}>)
    # add /opt/homebrew/opt/libomp/lib into rpath for M1/M2
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_INSTALL_NAME_TOOL} -add_rpath /opt/homebrew/opt/libomp/lib
        $<TARGET_FILE:${PROJECT_NAME}>)
    # add /usr/local/opt/libomp/lib into rpath for Intel
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_INSTALL_NAME_TOOL} -add_rpath /usr/local/opt/libomp/lib
        $<TARGET_FILE:${PROJECT_NAME}>)
    # add @loader_path into rpath as a fallback
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_INSTALL_NAME_TOOL} -add_rpath @loader_path
        $<TARGET_FILE:${PROJECT_NAME}>)
endif()