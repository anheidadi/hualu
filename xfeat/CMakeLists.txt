cmake_minimum_required(VERSION 3.16)
project(xfeat)
#set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})
set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_BUILD_TYPE "Debug")
# rknn api
include_directories(.

        )

find_package(OpenCV 4.7 REQUIRED )
set(RKNN_API_PATH ${CMAKE_SOURCE_DIR}/rknpu2/librknn_api)  #该变量定义了RKNN API库的路径
set(LIB_ARCH aarch64)
# 假设 librknnrt.so 位于 /path/to/weights 目录下
#find_library(
#    RKNN_RT_LIB  # 变量名
#    rknnrt       # 库名称（不带前缀和后缀）
#    PATHS /home/linaro/rvbs/lib # 指定搜索路径
#    NO_DEFAULT_PATH  # 不搜索默认路径（如 /usr/lib 等）
#)
#
#if (NOT RKNNRT_LIBRARY)
#    message(FATAL_ERROR "librknnrt.so not found in /path/to/weights")
#endif()
#set(RKNN_RT_LIB ${RKNN_API_PATH}/${LIB_ARCH}/librknnrt.so)  # 该变量定义了RKNN运行时库的路径。
set(RKNN_RT_LIB /home/linaro/rvbs/lib/librknnrt.so)  # 该变量定义了RKNN运行时库的路径。
include_directories(${RKNN_API_PATH}/include)
#include_directories(
#        /usr/include/mariadb)
#

add_executable(xfeat
        test.cpp)
        
#add_executable(xfeat
#        registration.cpp
#        XFeat.cpp
#        Logger.cpp
#        json.hpp
#        includelib.h
#        )
#add_executable(xfeat
#        testV2.cpp)
#find_package(OpenCLHeaders REQUIRED)
#find_package(OpenCL REQUIRED)
#MESSAGE(STATUS "Project: ${PROJECT_NAME}")
#MESSAGE(STATUS "OpenCL library status:")
#MESSAGE(STATUS "    version: ${OpenCL_VERSION_STRING}")
#MESSAGE(STATUS "    libraries: ${OpenCL_LIBRARY}")
#MESSAGE(STATUS "    include path: ${OpenCL_INCLUDE_DIR}")

#add_library(
#        xfeat SHARED
#        registration.cpp
#        XFeat.cpp
#        Logger.cpp
#        json.hpp
#        includelib.h
#)

target_link_libraries(xfeat
        PRIVATE dl
        ${RKNN_RT_LIB}
        ${OpenCV_LIBS}
#        mariadbclient
#        OpenCL::Headers
#        OpenCL::OpenCL
        )
#set_target_properties(rknn_example PROPERTIES
#    BUILD_RPATH "$ORIGIN/weights"
#    INSTALL_RPATH "$ORIGIN/weights"
#)

set_target_properties(xfeat PROPERTIES
			OUTPUT_NAME  registration PREFIX ""
#			BUILD_RPATH "/home/linaro/rvbs/lib"
#			BUILD_RPATH "$ORIGIN/../laneRegistration/weights"
#    			INSTALL_RPATH /home/linaro/rvbs/lib

)


#set_target_properties(xfeat PROPERTIES
#    BUILD_WITH_INSTALL_RPATH TRUE
#    INSTALL_RPATH "${RKNN_LIB_DIR}"
#)