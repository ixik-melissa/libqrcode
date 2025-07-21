# 
# The MIT License
# 
# Copyright (c) 2021 Sebastian Bauer
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# 
cmake_minimum_required(VERSION 3.15)

function(enable_cxx version)
    set(CMAKE_CXX_STANDARD ${version} PARENT_SCOPE)
endfunction()

project(libqrcode)

enable_testing()
enable_cxx(20)

add_library(qrcode INTERFACE)

target_link_libraries(qrcode INTERFACE $<$<PLATFORM_ID:Linux>:stdc++ m>)
target_link_libraries(qrcode INTERFACE $<$<PLATFORM_ID:Darwin>:stdc++ m>)
target_compile_options(qrcode INTERFACE $<$<PLATFORM_ID:Windows>:/std:c++latest>)

if (QRCODE_TESTS_ENABLED)
    add_subdirectory(test)
endif ()


# Installation rules
target_include_directories(qrcode INTERFACE 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/libqrcodeConfigVersion.cmake"
    VERSION 0.1
    COMPATIBILITY AnyNewerVersion
)

install(TARGETS qrcode
    EXPORT qrcodeTargets
    LIBRARY DESTINATION lib COMPONENT Runtime
    ARCHIVE DESTINATION lib COMPONENT Development
    RUNTIME DESTINATION bin COMPONENT Runtime
    PUBLIC_HEADER DESTINATION include COMPONENT Development
    BUNDLE DESTINATION bin COMPONENT Runtime
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/libqrcodeConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/libqrcodeConfig.cmake"
    INSTALL_DESTINATION lib/cmake/qrcode
)

install(EXPORT qrcodeTargets DESTINATION lib/cmake/qrcode)
install(FILES "${PROJECT_BINARY_DIR}/libqrcodeConfigVersion.cmake"
              "${PROJECT_BINARY_DIR}/libqrcodeConfig.cmake"
        DESTINATION lib/cmake/qrcode)
install(FILES "${PROJECT_SOURCE_DIR}/cmake/qrcode.pc.in"
        DESTINATION lib/pkgconfig)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include/qrcode)