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
#include <concepts>
#include <qrcode/structure/matrix.h>
#include <qrcode/structure/module.h>
#include <qrcode/structure/module_traits.h>
#include <qrcode/structure/occupied_columns.h>
#include <qrcode/structure/skip_column_view.h>
#include <qrcode/structure/zigzag_view.h>

namespace qrcode::structure
{
    template<class T, std::ranges::range Range> 
    requires std::convertible_to<std::ranges::range_value_t<Range>, T>
        && std::is_same_v<std::invoke_result_t<decltype(module_traits<T>::is_free), T const&>, bool>
    [[nodiscard]] constexpr auto place_data(matrix<T>& matrix, Range&& data) noexcept
    {
        using std::ranges::size;
        using std::ranges::begin;
        using std::ranges::end;

        auto const columns = occupied_columns(matrix);

        auto range = 
              views::zigzag({width(matrix)-static_cast<int>(size(columns)), height(matrix)})
            | views::skip_column(columns)
            | views::element(matrix)
            | std::views::filter(module_traits<T>::is_free);

        auto i = begin(range);
        auto d = begin(data);
        for (; i != end(range) && d != end(data); ++i, ++d)
            *i = *d;

        assert(d == end(data));
        std::ranges::fill(i, end(range), module_traits<T>::make_data(0));
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::structure::test
{
    constexpr auto place_data_places_given_data_in_zagzag_patterns_accross_given_matrix()
    {
        auto f = []
        {
            auto any_matrix = matrix<int>{{4,2}};

            auto const any_data = std::array{0,1,2,3,4,5,6,7};

            place_data(any_matrix, any_data);

            return element_at(any_matrix, {0,0}) == 5
                && element_at(any_matrix, {1,0}) == 4
                && element_at(any_matrix, {2,0}) == 3
                && element_at(any_matrix, {3,0}) == 2
                && element_at(any_matrix, {0,1}) == 7
                && element_at(any_matrix, {1,1}) == 6
                && element_at(any_matrix, {2,1}) == 1
                && element_at(any_matrix, {3,1}) == 0;
        };
        static_assert(f());
    }

    constexpr auto place_data_places_skips_entire_columns_without_interupting_the_zigzag_pattern()
    {
        auto f = []
        {
            auto any_matrix = matrix<int>{{5,2}};
            auto make_function = module_traits<int>::make_function;
            element_at(any_matrix, {3,0}) = make_function(1); 
            element_at(any_matrix, {3,1}) = make_function(1); 

            auto const any_data = std::array{0,1,2,3,4,5,6,7};

            place_data(any_matrix, any_data);

            return element_at(any_matrix, {0,0}) == 5
                && element_at(any_matrix, {1,0}) == 4
                && element_at(any_matrix, {2,0}) == 3
                && element_at(any_matrix, {3,0}) == -1
                && element_at(any_matrix, {4,0}) == 2
                && element_at(any_matrix, {0,1}) == 7
                && element_at(any_matrix, {1,1}) == 6
                && element_at(any_matrix, {2,1}) == 1
                && element_at(any_matrix, {3,1}) == -1
                && element_at(any_matrix, {4,1}) == 0;
        };
        static_assert(f());
    }

    constexpr auto place_data_places_skips_occupied_elements()
    {
        auto f = []
        {
            auto any_matrix = matrix<int>{{4,2}};
            auto make_function = module_traits<int>::make_function;
            element_at(any_matrix, {2,1}) = make_function(1); 
            element_at(any_matrix, {1,0}) = make_function(1); 

            auto const any_data = std::array{0,1,2,3,4,5};

            place_data(any_matrix, any_data);

            return element_at(any_matrix, {0,0}) == 3
                && element_at(any_matrix, {1,0}) == -1
                && element_at(any_matrix, {2,0}) == 2
                && element_at(any_matrix, {3,0}) == 1
                && element_at(any_matrix, {0,1}) == 5
                && element_at(any_matrix, {1,1}) == 4
                && element_at(any_matrix, {2,1}) == -1
                && element_at(any_matrix, {3,1}) == 0;
        };
        static_assert(f());
    }

    constexpr auto place_data_padds_zeros_if_bits_are_depleted_but_symbol_has_not_been_completed()
    {
        auto f = []
        {
            auto any_matrix = matrix<int>{{4,2}};

            auto const any_data = std::array{10,11,12,13};

            place_data(any_matrix, any_data);

            return element_at(any_matrix, {0,0}) == 0
                && element_at(any_matrix, {1,0}) == 0
                && element_at(any_matrix, {2,0}) == 13
                && element_at(any_matrix, {3,0}) == 12
                && element_at(any_matrix, {0,1}) == 0
                && element_at(any_matrix, {1,1}) == 0
                && element_at(any_matrix, {2,1}) == 11
                && element_at(any_matrix, {3,1}) == 10;
        };
        static_assert(f());
    }
}
#endif
