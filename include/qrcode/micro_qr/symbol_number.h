/*
 * The MIT License
 *
 * Copyright (c) 2025 Melissa Bauer
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

#include <qrcode/micro_qr/error_correction.h>
#include <qrcode/micro_qr/symbol_version.h>

namespace qrcode::micro_qr
{   
    [[nodiscard]] constexpr auto symbol_number(symbol_designator const& designator) noexcept
    {  
        // a bit ugly
        switch(version(designator))
        {
            case symbol_version::M1: 
                return 0;
            case symbol_version::M2: 
                return error_level(designator) == error_correction::level_L ? 1 : 2;
            case symbol_version::M3: 
                return error_level(designator) == error_correction::level_L ? 3 : 4;
            case symbol_version::M4: 
                return error_level(designator) == error_correction::level_L 
                    ? 5 
                    : error_level(designator) == error_correction::level_M ? 6 : 7;
        }
        return 0;
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::micro_qr::test
{
    constexpr auto symbol_number_is_a_three_bit_field_indicating_the_symbol_version_and_error_correction_being_applied_to_micro_qr_symbols()
    {
        static_assert(symbol_number(*make_designator(symbol_version::M1, std::nullopt)) == 0);
        static_assert(symbol_number(*make_designator(symbol_version::M2, error_correction::level_L)) == 1);
        static_assert(symbol_number(*make_designator(symbol_version::M2, error_correction::level_M)) == 2);
        static_assert(symbol_number(*make_designator(symbol_version::M3, error_correction::level_L)) == 3);
        static_assert(symbol_number(*make_designator(symbol_version::M3, error_correction::level_M)) == 4);
        static_assert(symbol_number(*make_designator(symbol_version::M4, error_correction::level_L)) == 5);
        static_assert(symbol_number(*make_designator(symbol_version::M4, error_correction::level_M)) == 6);
        static_assert(symbol_number(*make_designator(symbol_version::M4, error_correction::level_Q)) == 7);
    }
}
#endif