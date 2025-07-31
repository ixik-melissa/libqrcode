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

#include <ranges>
#include <algorithm>
#include <qrcode/structure/position.h>

namespace qrcode::structure::views
{
    template<std::ranges::range Columns> 
    requires std::is_same_v<std::ranges::range_value_t<Columns>, int> 
        && std::sortable<std::ranges::iterator_t<Columns>>
    [[nodiscard]] constexpr auto skip_column(Columns columns) noexcept
    {
        std::ranges::sort(columns);
        return std::views::transform([=](position point)
        {
            for (auto i : columns)
                if (point.x >= i)
                    ++point.x;

            return point;
        });
    }
}

namespace qrcode::views
{
    using qrcode::structure::views::skip_column;
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::structure::test
{
    constexpr auto skip_column_view_adjusts_positions_to_avoid_given_column()
    {
        auto f = []
        {
            auto const any_column = std::array{5};
            auto const some_positions = std::array<position,4>{{{1,4},{5,4},{2,3},{7,8}}};
            
            auto range = some_positions | views::skip_column(any_column);

            using list = std::initializer_list<position>;
            return std::ranges::equal(range, list{{1,4}, {6,4}, {2,3}, {8,8}});
        };
        static_assert(f());
    }

    constexpr auto skip_column_view_adjusts_positions_to_avoid_given_multiple_columns()
    {
        auto f = []
        {
            auto const some_columns = std::array{6,5};
            auto const some_positions = std::array<position,4>{{{1,4},{5,4},{2,3},{7,8}}};
            
            auto range = some_positions | views::skip_column(some_columns);

            using list = std::initializer_list<position>;
            return std::ranges::equal(range, list{{1,4}, {7,4}, {2,3}, {9,8}});
        };
        static_assert(f());
    }
}
#endif