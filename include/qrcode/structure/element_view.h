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

#include <ranges>
#include <algorithm>
#include <qrcode/structure/matrix.h>
#include <qrcode/structure/position.h>

namespace qrcode::structure::views
{
    template<class T>
    [[nodiscard]] constexpr auto element(matrix<T>& matrix) noexcept
    {
        return std::views::transform([&matrix](position point) -> auto& 
        { 
            return element_at(matrix, point); 
        });
    }

    template<class T>
    [[nodiscard]] constexpr auto element(matrix<T> const& matrix) noexcept
    {
        return std::views::transform([&matrix](position point) -> auto& 
        { 
            return element_at(matrix, point); 
        });
    }
}

namespace qrcode::views
{
    using qrcode::structure::views::element;
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::structure::test
{
    constexpr auto element_views_grant_read_access_to_given_constant_matrix()
    {
        auto f = []
        {
            using qrcode::structure::views::element;
            auto const some_matrix = matrix<int>{{2,3}};
            auto const some_positions = std::array<position,2>{{{1,1}, {1,2}}};

            auto matrix_elements = some_positions | views::element(some_matrix);

            return std::ranges::all_of(matrix_elements, [](auto i){ return i == 0; });
        };
        static_assert(f());
    }

    constexpr auto element_views_grant_random_access_to_given_non_constant_matrix()
    {
        auto f = []
        {
            using qrcode::structure::views::element;
            auto some_matrix = matrix<int>{{5,3}};
            auto const some_positions = std::array<position,3>{{{1,1}, {3,2}, {0,2}}};

            auto matrix_elements = some_positions | views::element(some_matrix);
            std::ranges::for_each(matrix_elements, [c=0](auto& i) mutable { i = c++; });

            return element_at(some_matrix, {1,1}) == 0
                && element_at(some_matrix, {3,2}) == 1
                && element_at(some_matrix, {0,2}) == 2;
        };
        static_assert(f());
    }
}
#endif
