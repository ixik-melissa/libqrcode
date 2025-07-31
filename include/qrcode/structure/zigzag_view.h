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

#include <algorithm>
#include <cassert>
#include <ranges>

#include <qrcode/structure/dimension.h>
#include <qrcode/structure/position.h>

namespace qrcode::structure::views
{
    [[nodiscard]] constexpr auto zigzag(dimension extend) noexcept
    {
        assert((width(extend) % 2) == 0);
        auto const position_count = width(extend) * height(extend);

        return std::views::iota(0, position_count) | std::views::transform([=](auto i)
        {
            auto const w = width(extend);
            auto const h = height(extend);

            auto const cycle = i/(4*h);
            auto const origin = position{cycle*4 ,0};

            auto const j = i - cycle*4*h;

            auto const x = origin.x + (j < (2*h) ? j % 2 : 2 + j%2);
            auto const y = origin.y + (j < (2*h) ? j / 2 : h-1 - (j-2*h)/2);

            return position{w-1-x, h-1-y};
        });
    }
}

namespace qrcode::views
{
    using qrcode::structure::views::zigzag;
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::structure::test
{
    constexpr auto zigzag_view_starts_bottom_right_and_zig_zags_up_to_the_top()
    {
        auto f = []
        {
            auto r = views::zigzag({2,5});

            using list = std::initializer_list<position>;
            return std::ranges::equal(
                r, 
                list{{1,4}, {0,4}, {1,3}, {0,3}, {1,2}, {0,2}, {1,1}, {0,1}, {1,0}, {0,0}
            });
        };
        static_assert(f());
    }

    constexpr auto zigzag_view_slides_over_and_runs_down_to_buttom()
    {
        auto f = []
        {
            auto r = views::zigzag({4,5});

            using list = std::initializer_list<position>;
            return std::ranges::equal(
                r, 
                list{{3,4}, {2,4}, {3,3}, {2,3}, {3,2}, {2,2}, {3,1}, {2,1}, {3,0}, {2,0},
                     {1,0}, {0,0}, {1,1}, {0,1}, {1,2}, {0,2}, {1,3}, {0,3}, {1,4}, {0,4}
            });
        };
        static_assert(f());
    }

    constexpr auto zigzag_view_starts_over_when_cycle_has_finished()
    {
        auto f = []
        {
            auto r = views::zigzag({8,2});

            using list = std::initializer_list<position>;
            return std::ranges::equal(
                r, 
                list{{7,1},{6,1},{7,0},{6,0},
                     {5,0},{4,0},{5,1},{4,1},
                     {3,1},{2,1},{3,0},{2,0},
                     {1,0},{0,0},{1,1},{0,1}
            });
        };
        static_assert(f());
    }
}
#endif