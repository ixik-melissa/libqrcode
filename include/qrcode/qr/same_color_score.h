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

#include <algorithm>
#include <ranges>

#include <qrcode/qr/penalty_weight.h>

#include <qrcode/structure/element_view.h>
#include <qrcode/structure/horizontal_view.h>
#include <qrcode/structure/matrix.h>

namespace qrcode::qr::detail
{
    template<std::ranges::range Range>
    [[nodiscard]] constexpr auto all_equal(Range&& range)
    {
        using value_type = std::ranges::range_value_t<Range>;
        using std::ranges::end;
        return end(range) == std::ranges::adjacent_find(range, std::not_equal_to<value_type>());
    }

    template<std::ranges::range Line1, std::ranges::range Line2>
    [[nodiscard]] constexpr auto same_2x2_score(Line1&& line_1, Line2&& line_2)
    {
        using std::ranges::begin;
        using std::ranges::end;

        auto i = begin(line_1);
        auto j = begin(line_2);

        using T = std::common_type_t<
            std::ranges::range_value_t<Line1>, std::ranges::range_value_t<Line1>>;
        std::array<T,4> block{{*(i++), *(j++), T{}, T{}}};

        auto score = 0;
        for (auto idx = 2; i != end(line_1); ++i, ++j)
        {
            block[idx] = *i; idx = (idx+1) % 4;
            block[idx] = *j; idx = (idx+1) % 4;

            if (all_equal(block))
                score += penalty_weight(2);
        }
        return score;
    }
}   

namespace qrcode::qr
{
    using qrcode::structure::matrix;

    template<class T>
    [[nodiscard]] constexpr auto same_color_score(matrix<T> const& matrix)
    {
        using qrcode::qr::detail::same_2x2_score;

        auto score = 0;
        for (auto y = 0; y != height(matrix)-1; ++y)
        {
            auto line_1 = views::horizontal({0,y+0}, {width(matrix),1}) | views::element(matrix);
            auto line_2 = views::horizontal({0,y+1}, {width(matrix),1}) | views::element(matrix);

            score += same_2x2_score(line_1, line_2);
        }
        return score;
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <qrcode/structure/make_matrix.h>

namespace qrcode::qr::test
{
    constexpr auto same_color_score_returns_the_penalty_score_derived_from_number_of_2x2_blocks_having_the_same_color()
    {
        using qrcode::structure::make_matrix;

        static_assert(same_color_score(make_matrix<int>({7,3}, std::array{
            1,1,0,0,0,0,1,
            1,1,1,1,0,0,0,
            0,0,0,0,1,0,0
        })) == (3 * penalty_weight(2)));
    }
}
#endif