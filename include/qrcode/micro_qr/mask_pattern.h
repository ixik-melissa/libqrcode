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

#include <array>
#include <ranges>

#include <qrcode/structure/apply_mask.h>
#include <qrcode/structure/matrix.h>

namespace qrcode::micro_qr
{   
    [[nodiscard]] constexpr auto mask_patterns() noexcept
    {
        using fn = bool(*)(int,int);
        return std::array<fn, 4>{
            [](int i, int /*j*/) { return (i % 2) == 0; },
            [](int i, int j) { return (i / 2 + j / 3) % 2 == 0; },
            [](int i, int j) { return (((i*j) % 2 + (i*j) % 3) % 2) == 0; },
            [](int i, int j) { return (((i+j) % 2 + (i*j) % 3) % 2) == 0; }
        };
    }

    [[nodiscard]] constexpr auto available_masks() noexcept
    {
        using qrcode::structure::apply_mask_array;
        return apply_mask_array(mask_patterns());
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::micro_qr::test
{
    constexpr auto micro_qr_mask_provides_data_masks_for_micro_qr_symbols()
    {
        using qrcode::structure::dimension;

        auto f = [](auto mask_type, auto nominal_size, auto nominal_data)
        {
            auto nominal = nominal_data;

            auto pattern = mask_patterns()[mask_type];
            
            for (auto i = 0; i != height(nominal_size); ++i)
            for (auto j = 0; j != width(nominal_size); ++j)
                if (pattern(i,j) != (*(nominal++) == '*'))
                    return false;
            return true;
        };
        static_assert(f(
            0, dimension{6,6}, 
            "******"
            "......"
            "******"
            "......"
            "******"
            "......"
        ));
        static_assert(f(
            1, dimension{9,6}, 
            "***...***"
            "***...***"
            "...***..."
            "...***..."
            "***...***"
            "***...***"
        ));
        static_assert(f(
            2, dimension{6,6}, 
            "******"
            "***..."
            "**.**."
            "*.*.*."
            "*.**.*"
            "*...**"
        ));
        static_assert(f(
            3, dimension{9,9}, 
            "*.*.*.*.*"
            "...***..."
            "*...***.."
            ".*.*.*.*."
            "***...***"
            ".***...**"
            "*.*.*.*.*"
            "...***..."
            "*...***.."
        ));
    }
}
#endif