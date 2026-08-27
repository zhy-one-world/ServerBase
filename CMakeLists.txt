cmake_minimum_required(VERSION 3.29.2)     # 设置CMake版本号
project(ServerBase)                        # 项目名称为 AllProject

# 添加所有子目录到 allSubDir
list(APPEND allSubDir "src")

# 遍历生成
foreach(dirname ${allSubDir})
    add_subdirectory(${dirname})
endforeach()

# 最终生成的项目文件. 会有 ALL_BUILD 和 ZERO_CHECK. 将它们放置在 Other 文件夹下
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "Other")