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
#include <qrcode/structure/module_traits.h>
#include <qrcode/structure/vertical_view.h>

namespace qrcode::qr::detail
{
    using qrcode::structure::matrix;

    template<std::ranges::range Line>
    [[nodiscard]] constexpr auto finder_line_score(Line&& line)
    {
        using value_type = std::ranges::range_value_t<Line>;
        constexpr auto pattern_left = std::array<value_type,11>{0,0,0,0,1,0,1,1,1,0,1};
        constexpr auto pattern_right = std::array<value_type,11>{1,0,1,1,1,0,1,0,0,0,0};
        constexpr auto pattern_both = std::array<value_type,15>{0,0,0,0,1,0,1,1,1,0,1,0,0,0,0};

        auto contains_range = [](auto&& range, auto&& subrange)
        {
            using std::ranges::begin;
            using std::ranges::end;
            using std::ranges::empty;
            return !empty(std::ranges::search(range, subrange));
        };

        auto score = 0;

        if (contains_range(line, pattern_left))
            score += penalty_weight(3);

        if (contains_range(line, pattern_right))
            score += penalty_weight(3);

        if (contains_range(line, pattern_both))
            score -= penalty_weight(3);

        return score;
    }

    template<class T>
    [[nodiscard]] constexpr auto horizontal_finder_like_score(matrix<T> const& matrix)
    {
        using qrcode::structure::module_traits;

        auto score = 0;
        for (auto y = 0; y != height(matrix); ++y)
        {
            auto range = 
                views::horizontal({0,y}, {width(matrix),1}) 
                | views::element(matrix)
                | std::views::transform([](auto v) { return module_traits<T>::is_set(v); });
            score += finder_line_score(range);
        }
        return score;
    }

    template<class T>
    [[nodiscard]] constexpr auto vertical_finder_like_score(matrix<T> const& matrix)
    {
        auto score = 0;

        for (auto x = 0; x != width(matrix); ++x)
        {
            auto range = 
                views::vertical({x,0}, {1,height(matrix)}) 
                | views::element(matrix)
                | std::views::transform([](auto v) { return module_traits<T>::is_set(v); });
            score += finder_line_score(range);
        }

        return score;
    }
}   

namespace qrcode::qr
{
    using qrcode::structure::matrix;

    template<class T>
    [[nodiscard]] constexpr auto finder_like_score(matrix<T> const& matrix)
    {
        using qrcode::qr::detail::horizontal_finder_like_score;
        using qrcode::qr::detail::vertical_finder_like_score;

        return 
            horizontal_finder_like_score(matrix) 
            + vertical_finder_like_score(matrix);
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <qrcode/structure/make_matrix.h>

namespace qrcode::qr::detail::test
{
    constexpr auto finder_line_score_searches_for_finder_pattern_in_given_row_and_returns_penalty_if_pattern_has_been_found()
    {
        static_assert(finder_line_score(std::array{1,0,0,0,1}) == 0);
        static_assert(finder_line_score(std::array{1,0,1,1,1,0,1,0,0,0,0}) == penalty_weight(3));
        static_assert(finder_line_score(std::array{0,0,0,0,1,0,1,1,1,0,1}) == penalty_weight(3));
        static_assert(finder_line_score(std::array{0,1,1,0,1,0,1,1,1,0,1,0,0,0,0,1,0}) == penalty_weight(3));
        static_assert(finder_line_score(std::array{0,0,0,0,1,0,1,1,1,0,1}) == penalty_weight(3));
        static_assert(finder_line_score(std::array{1,1,0,0,0,0,1,0,1,1,1,0,1,0,1,1}) == penalty_weight(3));
        static_assert(finder_line_score(std::array{1,0,1,1,1,0,1,0,0,0,1,0,0,0,1,0,1,1,1,0,1}) == 0);
        static_assert(finder_line_score(std::array{1,0,1,1,1,0,1,0,0,0,0,0,0,0,1,0,1,1,1,0,1}) == (2*penalty_weight(3)));
    }

    constexpr auto horizontal_finder_like_score_searches_horizontally_for_finder_pattern_in_given_matrix_and_returns_penalties_if_pattern_has_been_found()
    {
        using qrcode::structure::make_matrix;
        constexpr auto some_modules = make_matrix<int>({11,3}, std::array{
            1,0,1,1,1,0,1,0,0,0,0,
            1,0,0,0,0,0,1,0,0,0,1,
            0,0,0,0,1,0,1,1,1,0,1,
        });

        static_assert(horizontal_finder_like_score(some_modules) == (2 * penalty_weight(3)));
    }

    constexpr auto vertical_finder_like_score_searches_vertically_for_finder_pattern_in_given_matrix_and_returns_penalties_if_pattern_has_been_found()
    {
        using qrcode::structure::make_matrix;
        constexpr auto some_modules = make_matrix<int>({3,11}, std::array{
            1,1,0,
            0,0,0,
            1,0,0,
            1,0,0,
            1,0,1,
            0,0,0,
            1,1,1,
            0,0,1,
            0,0,1,
            0,0,0,
            0,1,1
        });

        static_assert(vertical_finder_like_score(some_modules) == (2 * penalty_weight(3)));
    }
}

namespace qrcode::qr::test
{
    constexpr auto finder_like_score_searches_for_finder_pattern_in_given_matrix_and_returns_penalties_if_pattern_has_been_found()
    {
        using qrcode::structure::make_matrix;
        constexpr auto some_modules = make_matrix<int>({11,11}, std::array{
            1,1,1,1,1,1,1,0,0,0,1,
            1,0,0,0,0,0,1,0,0,0,0,
            1,0,1,1,1,0,1,0,0,0,0,
            1,0,1,1,1,0,1,0,0,0,1,
            1,0,1,1,1,0,1,0,1,0,1,
            1,0,0,0,0,0,1,0,0,0,1,
            1,1,1,1,1,1,1,0,1,0,1,
            0,0,0,0,0,0,0,0,1,0,1,
            1,1,0,0,1,1,1,0,1,0,1,
            1,0,0,0,0,0,1,0,0,1,1,
            0,0,0,1,1,0,1,0,1,0,0
        });

        static_assert(finder_like_score(some_modules) == (3 * penalty_weight(3)));
    }
}
#endif