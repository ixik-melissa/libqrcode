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

#include <qrcode/structure/vertical_view.h>
#include <qrcode/structure/element_view.h>
#include <qrcode/structure/module_traits.h>

namespace qrcode::structure
{
    template<class T> 
    requires std::is_same_v<std::invoke_result_t<decltype(module_traits<T>::is_free), T const&>, bool>
    [[nodiscard]] constexpr auto occupied_columns(matrix<T> const& matrix) noexcept
    {
        auto is_free = module_traits<T>::is_free;
        cx::vector<int> columns;
        for (auto x = 0; x != width(matrix); ++x)
        {
            auto const range = views::vertical({x,0}, {1,height(matrix)}) | views::element(matrix);
            if (std::ranges::none_of(range, is_free))
                columns.push_back(x);
        }
        return columns;
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::structure::test
{
    constexpr auto occupied_columns_return_the_colums_which_are_completely_free()
    {
        auto f = []() constexpr
        {
            auto any_matrix = matrix<char>{{3,2}, module_traits<char>::make_free()};
            auto make_function = module_traits<char>::make_function;
            element_at(any_matrix, {1,0}) = make_function(0);
            element_at(any_matrix, {1,1}) = make_function(1);

            auto const columns = occupied_columns(any_matrix);

            return std::ranges::equal(columns, std::array{1});
        };
        static_assert(f());
    }
}
#endif