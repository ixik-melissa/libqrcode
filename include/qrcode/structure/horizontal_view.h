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
#include <qrcode/structure/cartesian_product_view.h>
#include <qrcode/structure/dimension.h>
#include <qrcode/structure/position.h>

namespace qrcode::structure::views
{
    [[nodiscard]] constexpr auto horizontal(position top_left, dimension extend) noexcept
    {
        return cartesian_product(
            std::views::iota(top_left.x, top_left.x+width(extend)), 
            std::views::iota(top_left.y, top_left.y+height(extend))
        );
    }
}

namespace qrcode::views
{
    using qrcode::structure::views::horizontal;
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::structure::test
{
    constexpr auto horizontal_view_provides_positions_which_covers_all_positions_walking_x_axis_first()
    {
        using list = std::initializer_list<position>;

        static_assert(
            std::ranges::equal(views::horizontal(position{0,0}, dimension{0,0}), 
            list{})
        );

        static_assert(
            std::ranges::equal(views::horizontal(position{0,0}, dimension{2,3}), 
            list{
                {0,0},{1,0},
                {0,1},{1,1},
                {0,2},{1,2}})
        );

        static_assert(
            std::ranges::equal(views::horizontal(position{10,0}, dimension{2,3}), 
            list{
                {10,0},{11,0},
                {10,1},{11,1},
                {10,2},{11,2}})
        );

        static_assert(
            std::ranges::equal(views::horizontal(position{10,5}, dimension{2,3}), 
            list{
                {10,5},{11,5},
                {10,6},{11,6},
                {10,7},{11,7}})
        );

        static_assert(
            std::ranges::equal(views::horizontal(position{10,5}, dimension{3,5}), 
            list{
                {10,5},{11,5},{12,5},
                {10,6},{11,6},{12,6},
                {10,7},{11,7},{12,7},
                {10,8},{11,8},{12,8},
                {10,9},{11,9},{12,9}})
        );
    }
}
#endif
