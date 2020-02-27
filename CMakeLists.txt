cmake_minimum_required(VERSION 3.14)
if(NOT DEFINED PROJECT_NAME)
    set(vq_master_project ON)
endif()
project(vq
    VERSION 0.1
    LANGUAGES CXX
)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if(NOT MSVC)
    option(TC_SEPARATE_SYMBOLS "Separate debug symbols from executable file into .dbg file." OFF)
endif()

include(Symbols)

if(vq_master_project)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

find_package(Qt5 REQUIRED 
    COMPONENTS 
        Widgets
        Gui
)
set(THREADS_PREFER_PTHREAD_FLAG ON)

set(CMAKE_AUTOMOC ON)

add_subdirectory(external EXCLUDE_FROM_ALL)
add_subdirectory(src)

if(UNIX)
    set(CPACK_GENERATOR DEB)
    set(CPACK_PACKAGING_INSTALL_PREFIX /opt/vq)
    set(CPACK_DEBIAN_PACKAGE_NAME vq)
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "jm4R")
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Convert vcxproj to Qt Creator base project")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    set(CPACK_DEBIAN_PACKAGE_SECTION base)
    set(CPACK_DEBIAN_ARCHIVE_TYPE gnutar)
    set(CPACK_DEBIAN_PACKAGE_PRIORITY optional)
elseif(WIN32)
    set(CPACK_PACKAGE_NAME vq)
    set(CPACK_PACKAGE_VENDOR "jm4R")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Convert vcxproj to Qt Creator base project")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "vq")
    set(CPACK_PACKAGE_EXECUTABLES "vq;vq")
endif()
include(CPack)