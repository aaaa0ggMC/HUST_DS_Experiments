cmake_minimum_required(VERSION 3.10)
project(ds_experiments)

# 这些是几个实验公用的一些代码,也许是UI啥的,anyway,至少先留一个slot是吧
include_directories(./shared)
# C++20应该Window那边MINGW也支持了吧?
add_compile_options(-std=c++20)
link_libraries(ncursesw)

# 实验1
aux_source_directory("./experiment_1/src" EXP_1_SRC)
add_executable(experiment_1 ${EXP_1_SRC} )
# ...

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
