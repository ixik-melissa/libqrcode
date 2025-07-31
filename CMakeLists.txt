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

function(enable_cxx version)
    set(CMAKE_CXX_STANDARD ${version} PARENT_SCOPE)
endfunction()

project(libqrcode)

enable_testing()
enable_cxx(23)

add_library(qrcode INTERFACE)

target_sources(qrcode INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/cx/vector.h

    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/result.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/svg.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/symbol.h

    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/code/bit_view.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/code/block_info.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/code/byte_view.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/code/code_block.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/code/code_capacity.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/code/codeword_view.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/code/error_correction_code.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/code/error_correction_polynomial.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/code/extended_remainder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/code/format_encoding.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/code/gf2p8.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/code/lfsr.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/code/padding_view.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/code/polynomial.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/code/polynomial_view.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/code/sequence.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/code/sequence_description.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/code/sequence_permutation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/code/sequence_view.h

    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/data/alphanumeric.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/data/alphanumeric_encoding.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/data/alphanumeric_encoder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/data/bit_stream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/data/byte.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/data/byte_encoder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/data/encoders.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/data/indicator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/data/kanji.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/data/kanji_encoding.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/data/kanji_encoder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/data/numeric.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/data/numeric_encoding.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/data/numeric_encoder.h

    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/data/best_fit/best_encoder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/data/best_fit/data_encoding.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/data/best_fit/data_length.h
    
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/data/optimizer/data_encoding.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/data/optimizer/data_length.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/data/optimizer/dispatch_mode.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/data/optimizer/mode.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/data/optimizer/optimize.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/data/optimizer/optimizer_state.h

    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/structure/apply_mask.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/structure/cartesian_product_view.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/structure/data_masking.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/structure/dimension.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/structure/element_view.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/structure/horizontal_view.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/structure/make_matrix.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/structure/matrix.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/structure/module.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/structure/module_traits.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/structure/occupied_columns.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/structure/place_data.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/structure/position.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/structure/separator_pattern.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/structure/skip_column_view.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/structure/timing_pattern.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/structure/vertical_view.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/structure/zigzag_view.h

    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/eci/assignment_number.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/eci/message_header.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/eci/view.h

    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/qr.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/adjacent_score.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/alignment_pattern.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/best_version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/raw_code.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/code_bits.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/code_capacity.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/dark_module_score.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/data_encoding.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/data_length.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/symbol_designator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/encoders.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/error_correction.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/finalize_symbol.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/finder_like_score.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/finder_pattern.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/format_information.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/generator_degree.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/mask_pattern.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/optimized_data_encoding.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/optimized_data_length.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/penalty_score.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/penalty_weight.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/same_color_score.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/separator_pattern.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/symbol_version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/timing_pattern.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/total_data_bits.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/total_blocks.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/version_category.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qr/version_information.h

    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/micro_qr/micro_qr.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/micro_qr/best_version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/micro_qr/raw_code.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/micro_qr/code_bits.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/micro_qr/code_capacity.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/micro_qr/symbol_designator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/micro_qr/encoders.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/micro_qr/error_correction.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/micro_qr/finalize_symbol.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/micro_qr/finder_pattern.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/micro_qr/format_information.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/micro_qr/generator_degree.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/micro_qr/mask_pattern.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/micro_qr/penalty_score.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/micro_qr/separator_pattern.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/micro_qr/symbol_number.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/micro_qr/symbol_version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/micro_qr/timing_pattern.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/micro_qr/total_data_bits.h

    ${CMAKE_CURRENT_SOURCE_DIR}/include/qrcode/qrcode.h
)

target_include_directories(qrcode INTERFACE "include")
target_link_libraries(qrcode INTERFACE $<$<CXX_COMPILER_ID:Clang>:stdc++ m>)
target_link_libraries(qrcode INTERFACE $<$<CXX_COMPILER_ID:AppleClang>:m>)

if (QRCODE_TESTS_ENABLED)
    add_subdirectory(test)
endif ()
