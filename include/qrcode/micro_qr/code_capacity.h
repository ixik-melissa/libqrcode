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

#include <array>

#include <qrcode/micro_qr/error_correction.h>
#include <qrcode/micro_qr/generator_degree.h>
#include <qrcode/micro_qr/symbol_version.h>
#include <qrcode/micro_qr/total_data_bits.h>

#include <qrcode/code/code_capacity.h>

namespace qrcode::micro_qr
{
    using qrcode::code::code_capacity;

    [[nodiscard]] constexpr auto make_code_capacity(symbol_designator const& designator) noexcept
    {
        return code_capacity{{
            .total_data_bits = total_data_bits(designator),
            .error_degree = generator_degree(designator),
            .total_blocks = 1
        }};
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::micro_qr::test
{
    constexpr auto code_capacities_can_be_created_from_given_micro_qr_symbol_symbol_version_and_error_correction()
    {
        // M1
        static_assert(make_code_capacity(*make_designator(symbol_version::M1, std::nullopt)) 
            == code_capacity{{.total_data_bits=20,.error_degree=2,.total_blocks=1}});

        // M2
        static_assert(make_code_capacity(*make_designator(symbol_version::M2, error_correction::level_L)) 
            == code_capacity{{.total_data_bits=40,.error_degree=5,.total_blocks=1}});

        static_assert(make_code_capacity(*make_designator(symbol_version::M2, error_correction::level_M)) 
            == code_capacity{{.total_data_bits=32,.error_degree=6,.total_blocks=1}});

        // M3
        static_assert(make_code_capacity(*make_designator(symbol_version::M3, error_correction::level_L))
            == code_capacity{{.total_data_bits=84,.error_degree=6,.total_blocks=1}});

        static_assert(make_code_capacity(*make_designator(symbol_version::M3, error_correction::level_M))
            == code_capacity{{.total_data_bits=68,.error_degree=8,.total_blocks=1}});

        // M4
        static_assert(make_code_capacity(*make_designator(symbol_version::M4, error_correction::level_L)) 
            == code_capacity{{.total_data_bits=128,.error_degree=8,.total_blocks=1}});

        static_assert(make_code_capacity(*make_designator(symbol_version::M4, error_correction::level_M)) 
            == code_capacity{{.total_data_bits=112,.error_degree=10,.total_blocks=1}});

        static_assert(make_code_capacity(*make_designator(symbol_version::M4, error_correction::level_Q)) 
            == code_capacity{{.total_data_bits=80,.error_degree=14,.total_blocks=1}});
    }
}
#endif