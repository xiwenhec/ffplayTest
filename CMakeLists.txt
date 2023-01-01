cmake_minimum_required(VERSION 3.16)
# 这里是工程名称  C工程
project(ffplayd C)

set(CMAKE_C_STANDARD 99)


set(FFMPEG_DIR ${CMAKE_SOURCE_DIR}/../ffmpeg/)

set(INC_DIR ${FFMPEG_DIR} /usr/include/SDL2/)

message(STATUS ${INC_DIR})

# 动态链接库或静态链接库的搜索路径
set(LIB_DIR ${CMAKE_SOURCE_DIR}/../ffmpeg_build/x86_64/lib/)

# file(GLOB SOURCE_LIST ${CMAKE_SOURCE_DIR}/*.*)

add_executable(ffplayd "")

set(TEST_FFPLAY ON)
# set(TEST_DEMUXER ON)



if(${TEST_FFPLAY})
target_sources(ffplayd PRIVATE 
        cmdutils.c 
        ffplay.c 
        opt_common.c)
endif()


if (${TEST_DEMUXER})
target_sources(ffplayd PRIVATE SNDmuxer.c)
endif()
target_include_directories(ffplayd PUBLIC
        ${INC_DIR}
        .
        )

target_link_directories(ffplayd PUBLIC 
            ${LIB_DIR})

#链接库
target_link_libraries(ffplayd
        #FFmpeg 库
        avfilter
        avformat
        avcodec
        # avdevice
        # postproc
        avutil
        swresample
        swscale
        # SDL2库
        SDL2
        m
        z
        pthread
        )