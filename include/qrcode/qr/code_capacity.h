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
#pragma once

#include <qrcode/qr/symbol_designator.h>
#include <qrcode/qr/generator_degree.h>
#include <qrcode/qr/total_blocks.h>
#include <qrcode/qr/total_data_bits.h>
#include <qrcode/code/code_capacity.h>

namespace qrcode::qr
{
    using qrcode::code::code_capacity;

    [[nodiscard]] constexpr auto make_code_capacity(symbol_designator const& designator) noexcept
    {
        return code_capacity{{
            .total_data_bits = total_data_bits(designator),
            .error_degree = generator_degree(designator),
            .total_blocks = total_blocks(designator)
        }};
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::qr::test
{
    constexpr auto code_capacities_can_be_created_from_given_qr_symbol_version_and_error_correction()
    {
        static_assert(make_code_capacity({symbol_version{1}, error_correction::level_L}) 
            == code_capacity{{.total_data_bits=152,.error_degree=7,.total_blocks=1}});

        static_assert(make_code_capacity({symbol_version{12}, error_correction::level_M}) 
            == code_capacity{{.total_data_bits=2320,.error_degree=22,.total_blocks=8}});

        static_assert(make_code_capacity({symbol_version{25}, error_correction::level_Q}) 
            == code_capacity{{.total_data_bits=5744,.error_degree=30,.total_blocks=29}});

        static_assert(make_code_capacity({symbol_version{6}, error_correction::level_H}) 
            == code_capacity{{.total_data_bits=480,.error_degree=28,.total_blocks=4}});
    }
}
#endif
