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

#include <qrcode/structure/dimension.h>
#include <qrcode/structure/element_view.h>
#include <qrcode/structure/module_traits.h>

namespace qrcode::structure
{
    template<class T>
    requires std::convertible_to<std::invoke_result_t<
        decltype(module_traits<T>::make_function), bool>, T>
    [[nodiscard]] constexpr auto place_separator_pattern(
        matrix<T>& matrix, position finder_position, dimension finder_size) noexcept
    {
        auto const pattern_size = dimension{width(finder_size)+2, height(finder_size)+2};
        auto index = std::views::iota(0, 2*width(pattern_size)+2*height(pattern_size));

        auto separator = std::views::transform([=](auto index)
        {
            if (index < (2*width(pattern_size)))
                return position{
                    index % width(pattern_size), 
                    index < width(pattern_size) ? 0 : height(pattern_size)-1};

            index -= 2*width(pattern_size);
            return position{
                index < height(pattern_size) ? 0 : width(pattern_size)-1, 
                index % height(pattern_size)};
        });

        auto to_finder = std::views::transform([finder=finder_position](auto point)
        {
            return position{point.x + finder.x - 1, point.y + finder.y - 1};
        });

        auto out_of_bounds = std::views::filter([&](auto point)
        { 
            return contains(matrix, point);
        });

        auto range = index 
            | separator
            | to_finder 
            | out_of_bounds 
            | views::element(matrix);

        auto make_function = module_traits<T>::make_function;
        for (auto& i : range)
            i = make_function(0);
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>
#include <qrcode/structure/make_matrix.h>

namespace qrcode::structure::test
{
    constexpr auto separator_patterns_are_placed_around_the_given_finder()
    {
        auto f = []
        {
            using namespace std::literals;
            using namespace qrcode::structure;
            auto const any_finder_position = position{4,7};
            auto const any_finder_size = dimension{3,5};
            auto any_matrix = matrix<char>{{25,25}, module_traits<char>::make_free()};

            place_separator_pattern(any_matrix, any_finder_position, any_finder_size);
            
            return any_matrix == make_matrix<char>({25,25},
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."
                "...-----................."
                "...-...-................."
                "...-...-................."
                "...-...-................."
                "...-...-................."
                "...-...-................."
                "...-----................."
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."sv
            );
        };
        static_assert(f());
    }

    constexpr auto separator_patterns_are_cropped_when_being_at_the_border()
    {
        auto f = []
        {
            using namespace std::literals;
            using namespace qrcode::structure;
            auto const any_finder_position = position{0,0};
            auto const any_finder_size = dimension{5,2};
            auto any_matrix = matrix<char>{{25,10}, module_traits<char>::make_free()};

            place_separator_pattern(any_matrix, any_finder_position, any_finder_size);
            
            return any_matrix == make_matrix<char>({25,10},
                ".....-..................."
                ".....-..................."
                "------..................."
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."sv
            );
        };
        static_assert(f());
    }
}
#endif