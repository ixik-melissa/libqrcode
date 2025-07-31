/*
 * The MIT License
 *
 * Copyright (c) 2021 Melissa Bauer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 
#include <iostream>
#include <fstream>

#include <cx/vector.h>
#include <cx/cmath.h>

#include <qrcode/result.h>
#include <qrcode/svg.h>
#include <qrcode/symbol.h>

#include <qrcode/code/bit_view.h>
#include <qrcode/code/block_info.h>
#include <qrcode/code/byte_view.h>
#include <qrcode/code/code_block.h>
#include <qrcode/code/code_capacity.h>
#include <qrcode/code/codeword_view.h>
#include <qrcode/code/error_correction_code.h>
#include <qrcode/code/error_correction_polynomial.h>
#include <qrcode/code/extended_remainder.h>
#include <qrcode/code/format_encoding.h>
#include <qrcode/code/gf2p8.h>
#include <qrcode/code/lfsr.h>
#include <qrcode/code/padding_view.h>
#include <qrcode/code/polynomial.h>
#include <qrcode/code/polynomial_view.h>
#include <qrcode/code/sequence.h>
#include <qrcode/code/sequence_description.h>
#include <qrcode/code/sequence_permutation.h>
#include <qrcode/code/sequence_view.h>

#include <qrcode/data/alphanumeric.h>
#include <qrcode/data/alphanumeric_encoding.h>
#include <qrcode/data/alphanumeric_encoder.h>
#include <qrcode/data/bit_stream.h>
#include <qrcode/data/byte.h>
#include <qrcode/data/byte_encoder.h>
#include <qrcode/data/encoders.h>
#include <qrcode/data/indicator.h>
#include <qrcode/data/kanji.h>
#include <qrcode/data/kanji_encoding.h>
#include <qrcode/data/kanji_encoder.h>
#include <qrcode/data/numeric.h>
#include <qrcode/data/numeric_encoding.h>
#include <qrcode/data/numeric_encoder.h>

#include <qrcode/data/best_fit/best_encoder.h>
#include <qrcode/data/best_fit/data_encoding.h>
#include <qrcode/data/best_fit/data_length.h>

#include <qrcode/data/optimizer/data_encoding.h>
#include <qrcode/data/optimizer/data_length.h>
#include <qrcode/data/optimizer/dispatch_mode.h>
#include <qrcode/data/optimizer/mode.h>
#include <qrcode/data/optimizer/optimizer_state.h>
#include <qrcode/data/optimizer/optimize.h>

#include <qrcode/structure/apply_mask.h>
#include <qrcode/structure/cartesian_product_view.h>
#include <qrcode/structure/data_masking.h>
#include <qrcode/structure/dimension.h>
#include <qrcode/structure/element_view.h>
#include <qrcode/structure/horizontal_view.h>
#include <qrcode/structure/make_matrix.h>
#include <qrcode/structure/matrix.h>
#include <qrcode/structure/module.h>
#include <qrcode/structure/module_traits.h>
#include <qrcode/structure/occupied_columns.h>
#include <qrcode/structure/place_data.h>
#include <qrcode/structure/position.h>
#include <qrcode/structure/separator_pattern.h>
#include <qrcode/structure/skip_column_view.h>
#include <qrcode/structure/timing_pattern.h>
#include <qrcode/structure/vertical_view.h>
#include <qrcode/structure/zigzag_view.h>

#include <qrcode/eci/assignment_number.h>
#include <qrcode/eci/message_header.h>
#include <qrcode/eci/view.h>

#include <qrcode/qr/qr.h>
#include <qrcode/qr/adjacent_score.h>
#include <qrcode/qr/alignment_pattern.h>
#include <qrcode/qr/best_version.h>
#include <qrcode/qr/code_bits.h>
#include <qrcode/qr/code_capacity.h>
#include <qrcode/qr/dark_module_score.h>
#include <qrcode/qr/data_encoding.h>
#include <qrcode/qr/data_length.h>
#include <qrcode/qr/symbol_designator.h>
#include <qrcode/qr/encoders.h>
#include <qrcode/qr/error_correction.h>
#include <qrcode/qr/finalize_symbol.h>
#include <qrcode/qr/finder_like_score.h>
#include <qrcode/qr/finder_pattern.h>
#include <qrcode/qr/format_information.h>
#include <qrcode/qr/generator_degree.h>
#include <qrcode/qr/mask_pattern.h>
#include <qrcode/qr/optimized_data_encoding.h>
#include <qrcode/qr/optimized_data_length.h>
#include <qrcode/qr/penalty_score.h>
#include <qrcode/qr/penalty_weight.h>
#include <qrcode/qr/same_color_score.h>
#include <qrcode/qr/raw_code.h>
#include <qrcode/qr/separator_pattern.h>
#include <qrcode/qr/symbol_version.h>
#include <qrcode/qr/timing_pattern.h>
#include <qrcode/qr/total_data_bits.h>
#include <qrcode/qr/total_blocks.h>
#include <qrcode/qr/version_information.h>

#include <qrcode/micro_qr/micro_qr.h>
#include <qrcode/micro_qr/best_version.h>
#include <qrcode/micro_qr/code_bits.h>
#include <qrcode/micro_qr/code_capacity.h>
#include <qrcode/micro_qr/symbol_designator.h>
#include <qrcode/micro_qr/encoders.h>
#include <qrcode/micro_qr/error_correction.h>
#include <qrcode/micro_qr/finalize_symbol.h>
#include <qrcode/micro_qr/finder_pattern.h>
#include <qrcode/micro_qr/format_information.h>
#include <qrcode/micro_qr/generator_degree.h>
#include <qrcode/micro_qr/mask_pattern.h>
#include <qrcode/micro_qr/penalty_score.h>
#include <qrcode/micro_qr/separator_pattern.h>
#include <qrcode/micro_qr/raw_code.h>
#include <qrcode/micro_qr/symbol_number.h>
#include <qrcode/micro_qr/symbol_version.h>
#include <qrcode/micro_qr/timing_pattern.h>
#include <qrcode/micro_qr/total_data_bits.h>

#include <qrcode/qrcode.h>

template<class Stream>
class main_frame
{
public:
    main_frame(Stream& out) : stream{&out} { *stream << "[ -- Running debug tests -- ]\n"; }
    ~main_frame() { *stream << "[ -- Running debug tests -- ]\n"; }

private:
    Stream* stream;
};

int main()
{
    auto frame = main_frame{std::cout};

    // place debug code here
}
