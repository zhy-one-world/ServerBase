cmake_minimum_required(VERSION 4.0.2)     # 设置CMake版本号
project(ServerBase)                      # 项目名称为 HldServer

# 添加所有的子目录到 ComponentAllSubDir 变量中
list(APPEND ComponentAllSubDir "common")
list(APPEND ComponentAllSubDir "lua")
list(APPEND ComponentAllSubDir "net")

# 设置 PROJECT_COMPONENT_DIR 变量为 ZhyServer 路径. 方便其他访问
set(PROJECT_COMPONENT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 遍历每个组件目录. 调用生成
foreach(dirname ${ComponentAllSubDir}) 
    add_subdirectory(${dirname})
endforeach()