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
#include <concepts>

#include <qrcode/structure/horizontal_view.h>
#include <qrcode/structure/element_view.h>
#include <qrcode/structure/module_traits.h>
#include <qrcode/structure/vertical_view.h>

namespace qrcode::structure::views
{
    template<class T> requires requires { { !std::declval<T>() } -> std::convertible_to<T>; }
    [[nodiscard]] constexpr auto timing_pattern(T initial)
    {
        return std::views::iota(0) | std::views::transform([b=initial](auto&&...) mutable 
        {
            auto current = b;
            b = static_cast<T>(!b);
            return current;
        });
    }
}

namespace qrcode::structure
{
    template<std::ranges::viewable_range Range, class F>
    requires std::convertible_to<std::invoke_result_t<F, bool>, std::ranges::range_value_t<Range>>
    [[nodiscard]] constexpr auto place_timing_pattern(Range&& range, F make_function)
    {
        auto pattern = views::timing_pattern(1);
        auto p = begin(pattern);
        for (auto& i : range)
            i = make_function(*(p++));
    }

    template<class T>
    requires std::convertible_to<std::invoke_result_t<decltype(module_traits<T>::is_free), T>, bool>
        && std::convertible_to<std::invoke_result_t<decltype(module_traits<T>::make_function), bool>, T>
    [[nodiscard]] constexpr auto place_horizontal_timing_pattern(
        matrix<T>& matrix, int row) noexcept
    {
        auto range = 
            views::horizontal({0,row}, {width(matrix),1}) 
            | views::element(matrix)
            | std::views::filter(module_traits<T>::is_free);

        place_timing_pattern(range, module_traits<T>::make_function);
    }

    template<class T>
    requires std::convertible_to<std::invoke_result_t<decltype(module_traits<T>::is_free), T>, bool>
        && std::convertible_to<std::invoke_result_t<decltype(module_traits<T>::make_function), bool>, T>
    [[nodiscard]] constexpr auto place_vertical_timing_pattern(
        matrix<T>& matrix, int column) noexcept
    {
        auto range = 
            views::vertical({column,0}, {1,height(matrix)}) 
            | views::element(matrix)
            | std::views::filter(module_traits<T>::is_free);

        place_timing_pattern(range, module_traits<T>::make_function);
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>
#include <qrcode/structure/make_matrix.h>

namespace qrcode::structure::views::test
{
    constexpr auto timing_patterns_are_vertical_and_horizontal_lines_consisting_of_alternating_dark_and_light_function_modules()
    {
        static_assert(std::ranges::equal(views::timing_pattern(1) | std::views::take(1), std::array{1}));
        static_assert(std::ranges::equal(views::timing_pattern(1) | std::views::take(3), std::array{1,0,1}));
        static_assert(std::ranges::equal(views::timing_pattern(1) | std::views::take(5), std::array{1,0,1,0,1}));
        static_assert(std::ranges::equal(views::timing_pattern(1) | std::views::take(7), std::array{1,0,1,0,1,0,1}));
        static_assert(std::ranges::equal(views::timing_pattern(1) | std::views::take(9), std::array{1,0,1,0,1,0,1,0,1}));
        static_assert(std::ranges::equal(views::timing_pattern(0) | std::views::take(9), std::array{0,1,0,1,0,1,0,1,0}));
    }
}

namespace qrcode::structure::test
{
    constexpr auto timing_patterns_can_be_placed_in_given_range()
    {
        auto f = []()
        {
            auto any_range = std::array{3,3,3,3,3};

            place_timing_pattern(any_range, [](auto bit) { return bit ? 1 : 0; });

            return std::ranges::equal(any_range, std::array{1,0,1,0,1});
        };
        static_assert(f());
    }

    constexpr auto horizontal_timing_patterns_can_be_placed_in_given_matrix_starting_at_given_position_lasting_until_right_border_is_reached()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_row = 3;
            auto any_matrix = matrix<char>{{10,6}, module_traits<char>::make_free()};

            place_horizontal_timing_pattern(any_matrix, any_row);

            return any_matrix == make_matrix<char>({10,6},
                ".........."
                ".........."
                ".........."
                "*-*-*-*-*-"
                ".........."
                ".........."sv
            );
        };
        static_assert(f());
    }

    constexpr auto vertical_timing_patterns_can_be_placed_in_given_matrix_starting_at_given_position_lasting_until_bottom_border_is_reached()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_column = 2;
            auto any_matrix = matrix<char>{{10,6}, module_traits<char>::make_free()};

            place_vertical_timing_pattern(any_matrix, any_column);

            return any_matrix == make_matrix<char>({10,6},
                "..*......."
                "..-......."
                "..*......."
                "..-......."
                "..*......."
                "..-......."sv
            );
        };
        static_assert(f());
    }
}
#endif
