# 
# The MIT License
# 
# Copyright (c) 2025 Melissa Bauer
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

add_executable(libqrcode_test main.cpp)

target_compile_options(libqrcode_test PRIVATE $<$<CXX_COMPILER_ID:GNU>:
    -Wall -Wextra -Wpedantic -Werror
    -fconstexpr-depth=9999999 -fconstexpr-ops-limit=9999999999 -fconcepts-diagnostics-depth=10>)

target_compile_options(libqrcode_test PRIVATE $<$<CXX_COMPILER_ID:Clang>:
    -Wall -Wextra -Wpedantic -Werror -fconstexpr-steps=999999999>)

target_compile_options(libqrcode_test PRIVATE $<$<CXX_COMPILER_ID:AppleClang>:
    -Wall -Wextra -Wpedantic -Werror -fconstexpr-steps=999999999>)
    
target_compile_options(libqrcode_test PRIVATE $<$<CXX_COMPILER_ID:MSVC>:
    /Wall /WX /constexpr:steps999999999>)

target_compile_options(libqrcode_test PRIVATE -DQRCODE_TESTS_ENABLED)
target_link_libraries(libqrcode_test PRIVATE qrcode)

add_test(NAME libqrcode_test COMMAND libqrcode_test)
