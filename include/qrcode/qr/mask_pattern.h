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
#include <ranges>

#include <qrcode/structure/apply_mask.h>
#include <qrcode/structure/matrix.h>

namespace qrcode::qr
{
    [[nodiscard]] constexpr auto mask_patterns() noexcept
    {
        using fn = bool(*)(int,int);
        return std::array<fn, 8>{
            [](int i, int j) { return ((i+j) % 2) == 0; },
            [](int i, int /*j*/) { return (i % 2) == 0; },
            [](int /*i*/, int j) { return (j % 3) == 0; },
            [](int i, int j) { return ((i+j) % 3) == 0; },
            [](int i, int j) { return (i / 2 + j / 3) % 2 == 0; },
            [](int i, int j) { return ((i*j) % 2 + (i*j) % 3) == 0; },
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
#include <string_view>
#include <qrcode/structure/make_matrix.h>

namespace qrcode::qr::test
{
    constexpr auto mask_patterns_represent_the_data_masks_for_qr_symbols()
    {
        using qrcode::structure::dimension;

        auto f = [](auto pattern, auto size, auto nominal)
        {
            for (auto i = 0; i != height(size); ++i)
            for (auto j = 0; j != width(size); ++j)
                if (pattern(i,j) != (*(nominal++) == '*'))
                    return false;
            return true;
        };
        static_assert(f(mask_patterns()[0], dimension{9,6},
            "*.*.*.*.*"
            ".*.*.*.*."
            "*.*.*.*.*"
            ".*.*.*.*."
            "*.*.*.*.*"
            ".*.*.*.*."
        ));
        static_assert(f(mask_patterns()[1], dimension{6,6},
            "******"
            "......"
            "******"
            "......"
            "******"
            "......"
        ));
        static_assert(f(mask_patterns()[2], dimension{9,6},
            "*..*..*.."
            "*..*..*.."
            "*..*..*.."
            "*..*..*.."
            "*..*..*.."
            "*..*..*.."
        ));
        static_assert(f(mask_patterns()[3], dimension{9,6},
            "*..*..*.."
            "..*..*..*"
            ".*..*..*."
            "*..*..*.."
            "..*..*..*"
            ".*..*..*."
        ));
        static_assert(f(mask_patterns()[4], dimension{9,6},
            "***...***"
            "***...***"
            "...***..."
            "...***..."
            "***...***"
            "***...***"
        ));
        static_assert(f(mask_patterns()[5], dimension{7,7},
            "*******"
            "*.....*"
            "*..*..*"
            "*.*.*.*"
            "*..*..*"
            "*.....*"
            "*******"
        ));
        static_assert(f(mask_patterns()[6], dimension{6,6},
            "******"
            "***..."
            "**.**."
            "*.*.*."
            "*.**.*"
            "*...**"
        ));
        static_assert(f(mask_patterns()[7], dimension{9,9},
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

    constexpr auto available_masks_can_be_applied_to_given_matrix()
    {
        auto f = []
        {
            using namespace std::literals;
            using qrcode::structure::make_matrix;
            auto const masks = available_masks();
            auto const any_mask = masks[0];

            auto any_matrix = make_matrix<char>({4,4}, 
                "++++"
                "++++"
                "++++"
                "++++"sv
            );

            return any_mask(any_matrix) == make_matrix<char>({4,4},
                ",+,+"
                "+,+,"
                ",+,+"
                "+,+,"sv
            );
        };
        static_assert(f());
    }
}
#endif