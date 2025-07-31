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

#include <concepts>

#include <qrcode/structure/timing_pattern.h>

namespace qrcode::micro_qr
{
    using qrcode::structure::matrix;

    template<class T>
    [[nodiscard]] constexpr auto place_timing_patterns(matrix<T>& matrix) noexcept
    {
        auto const timing_location = 0;
        using qrcode::structure::place_horizontal_timing_pattern;
        using qrcode::structure::place_vertical_timing_pattern;
        place_horizontal_timing_pattern(matrix, timing_location);
        place_vertical_timing_pattern(matrix, timing_location);
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>
#include <qrcode/structure/make_matrix.h>

namespace qrcode::micro_qr::test
{
    constexpr auto timing_pattern_are_placed_horizontally_and_vertically_next_to_finder_patterns_for_micro_qr_symbols()
    {
        auto f = []
        {
            using namespace std::literals;
            using namespace qrcode::structure;
            auto any_matrix = make_matrix<char>({17,17}, 
                "*******-........."
                "*-----*-........."
                "*-***-*-........."
                "*-***-*-........."
                "*-***-*-........."
                "*-----*-........."
                "*******-........."
                "--------........."
                "................."
                "................."
                "................."
                "................."
                "................."
                "................."
                "................."
                "................."
                "................."sv
            );

            place_timing_patterns(any_matrix);

            return any_matrix == make_matrix<char>({17,17},
                "*******-*-*-*-*-*"
                "*-----*-........."
                "*-***-*-........."
                "*-***-*-........."
                "*-***-*-........."
                "*-----*-........."
                "*******-........."
                "--------........."
                "*................"
                "-................"
                "*................"
                "-................"
                "*................"
                "-................"
                "*................"
                "-................"
                "*................"sv
            );
        };
        static_assert(f());
    }
}
#endif
