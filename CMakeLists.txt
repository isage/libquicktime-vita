cmake_minimum_required(VERSION 3.12)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
    include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(quicktime)

FILE(GLOB LIB_SRC src/*.c)
FILE(GLOB AUDIOCODEC_SRC plugins/audiocodec/*.c)
FILE(GLOB VIDEOCODEC_SRC plugins/videocodec/*.c)
FILE(GLOB FFMPEGCODEC_SRC plugins/ffmpeg/*.c)

add_library(${PROJECT_NAME} STATIC
  ${LIB_SRC}
  ${AUDIOCODEC_SRC}
  ${VIDEOCODEC_SRC}
  ${FFMPEGCODEC_SRC}
)

target_include_directories(${PROJECT_NAME} PRIVATE include)

set(MY_PUBLIC_HEADERS
    ${PROJECT_SOURCE_DIR}/include/quicktime/colormodels.h
    ${PROJECT_SOURCE_DIR}/include/quicktime/compression.h
    ${PROJECT_SOURCE_DIR}/include/quicktime/lqt_atoms.h
    ${PROJECT_SOURCE_DIR}/include/quicktime/lqt_codecinfo.h
    ${PROJECT_SOURCE_DIR}/include/quicktime/lqt.h
    ${PROJECT_SOURCE_DIR}/include/quicktime/lqt_qtvr.h
    ${PROJECT_SOURCE_DIR}/include/quicktime/lqt_version.h
    ${PROJECT_SOURCE_DIR}/include/quicktime/quicktime.h
)

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${MY_PUBLIC_HEADERS}")
include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/lqt"
)
