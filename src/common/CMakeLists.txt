cmake_minimum_required(VERSION 3.29.3)     # 指定 CMake 版本号
project(server_common)                            # 项目名称为 server_common
option(GROUP_BY_EXPLORER ON)            # 启用保留文件结构和资源管理器一样
set(CMAKE_CXX_STANDARD 20)              # 设置C++版本 11
set(CMAKE_BUILD_PARALLEL_LEVEL 32)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${PROJECT_COMPONENT_DIR}/../../dep/vcpkg/installed/x64-windows/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)
add_definitions(-DSPDLOG_USE_STD_FORMAT)
add_definitions(-D_WIN32_WINNT=0x0601)

set(COMMON_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include/common)

# 头文件：include/common + src/common 内私有头（如 curl）
file(GLOB_RECURSE HEADER_FILES
    ${COMMON_INCLUDE_DIR}/*.h
    ${COMMON_INCLUDE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
)

# 实现文件
file(GLOB_RECURSE SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

set(AllFile ${HEADER_FILES} ${SOURCE_FILES})

# 对 AllFile 变量里面的所有文件分类(保留资源管理器的目录结构)
foreach(fileItem ${AllFile})
    get_filename_component(PARENT_DIR "${fileItem}" DIRECTORY)
    string(REPLACE "${CMAKE_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
    string(REPLACE "/" "\\" GROUP "${GROUP}")
    source_group("${GROUP}" FILES "${fileItem}")
endforeach()

add_library(server_common STATIC ${AllFile})

option(USE_SOLUTION_FOLDERS  ON)
set_target_properties(server_common PROPERTIES FOLDER "src")

target_include_directories(server_common PUBLIC
    ${COMMON_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/curl
    ${PROJECT_COMPONENT_DIR}/../../dep/vcpkg/installed/x64-windows/include
    ${PROJECT_COMPONENT_DIR}/../../dep/vcpkg/installed/x64-windows/include/server_base/common
)
