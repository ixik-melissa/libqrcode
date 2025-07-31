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
#include <numeric>

#include <qrcode/structure/element_view.h>
#include <qrcode/structure/horizontal_view.h>
#include <qrcode/structure/matrix.h>
#include <qrcode/structure/module_traits.h>
#include <qrcode/structure/vertical_view.h>

namespace qrcode::micro_qr::detail
{
    using qrcode::structure::matrix;

    // std::accumulate cannot deal with ranges which have different types for iterator and sentinel
    template<std::ranges::forward_range Range>
    requires std::convertible_to<std::ranges::range_value_t<Range>, int>
    [[nodiscard]] constexpr auto  sum(Range&& range) noexcept
    {
        auto result = 0;
        for (auto const& i : range)
            result += i;
        return result;
    }

    template<class T>
    [[nodiscard]] constexpr auto horizontal_score(matrix<T> const& matrix) noexcept
    {
        using qrcode::structure::module_traits;

        auto range = 
            views::horizontal({1,height(matrix)-1}, {width(matrix)-1,1}) 
            | views::element(matrix)
            | std::views::transform([](auto v) { return module_traits<T>::is_set(v); });

        return sum(range);
    }

    template<class T>
    [[nodiscard]] constexpr auto vertical_score(matrix<T> const& matrix) noexcept
    {
        using qrcode::structure::module_traits;

        auto range = 
            views::vertical({width(matrix)-1,1}, {1,height(matrix)-1}) 
            | views::element(matrix)
            | std::views::transform([](auto v) { return module_traits<T>::is_set(v); });

        return sum(range);
    }

    template<class T>
    [[nodiscard]] constexpr auto high_score(T sum_1, T sum_2) noexcept
    {
        using std::swap;
        if (sum_1 > sum_2)
            swap(sum_1, sum_2);
        return 16 * sum_1 + sum_2;
    }

    struct negative_score { int value = std::numeric_limits<int>::max(); };

    [[nodiscard]] constexpr auto operator<(negative_score const& a, negative_score const& b) noexcept
    {
        return a.value > b.value;
    }
}

namespace qrcode::micro_qr
{
    using qrcode::structure::matrix;
    
    template<class T>
    [[nodiscard]] constexpr auto penalty_score(matrix<T> const& matrix) noexcept
    {
        using qrcode::micro_qr::detail::high_score;
        using qrcode::micro_qr::detail::horizontal_score;
        using qrcode::micro_qr::detail::vertical_score;
        using qrcode::micro_qr::detail::negative_score;

        return negative_score{
            high_score(horizontal_score(matrix), vertical_score(matrix))
        };
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>
#include <qrcode/structure/make_matrix.h>

namespace qrcode::micro_qr::detail::test
{
    constexpr auto sum_computes_the_integral_sum_of_given_range()
    {
        static_assert(sum(std::array<bool,0>{}) == 0);
        static_assert(sum(std::array{1}) == 1);
        static_assert(sum(std::array{0}) == 0);
        static_assert(sum(std::array{1,1}) == 2);
        static_assert(sum(std::array{1,0}) == 1);
        static_assert(sum(std::array{1,0,1,0,0,1}) == 3);
    }

    constexpr auto horizontal_score_counts_the_number_of_non_zeros_along_the_lower_side_edge()
    {
        using namespace std::literals;
        using qrcode::structure::make_matrix;
        constexpr auto any_matrix = make_matrix<char>({11,11},
            "..........."
            "..........."  
            "..........."  
            "..........."  
            "..........."  
            "..........."  
            "..........."  
            "..........."  
            "..........."  
            "..........."  
            "++,,+,+,++,"sv
        //   11001010110
        );
        static_assert(horizontal_score(any_matrix) == (/*1+*/1+0+0+1+0+1+0+1+1+0));
    }

    constexpr auto vertical_score_counts_the_number_of_non_zeros_along_the_right_side_edge()
    {
        using namespace std::literals;
        using qrcode::structure::make_matrix;
        constexpr auto any_matrix = make_matrix<char>({11,11},
            "..........+"   // 1
            "..........+"   // 1
            "..........,"   // 0
            "..........,"   // 0
            "..........+"   // 1
            "..........,"   // 0
            "..........,"   // 0
            "..........+"   // 1
            "..........+"   // 1
            "..........,"   // 0
            "..........+"sv // 1
        );
        static_assert(vertical_score(any_matrix) == (/*1+*/1+0+0+1+0+0+1+1+0+1));
    }

    constexpr auto negative_scores_are_infinite_by_default()
    {
        static_assert(negative_score{}.value == std::numeric_limits<int>::max());
    }

    constexpr auto negative_scores_are_better_when_they_are_as_small_as_possible()
    {
        static_assert(negative_score{1} < negative_score{0});
        static_assert(negative_score{2} < negative_score{1});
        static_assert(!(negative_score{3} < negative_score{10}));
    }


    constexpr auto high_score_represents_the_evaluation_score_for_micro_qr_codes()
    {
        static_assert(high_score(0, 0) == 0);
        static_assert(high_score(0, 1) == 1);
        static_assert(high_score(1, 1) == 17);
        static_assert(high_score(1, 2) == 18);
        static_assert(high_score(2, 1) == 18);
        static_assert(high_score(6, 8) == 104);
    }
}

namespace qrcode::micro_qr::test
{
    constexpr auto penalty_score_returns_the_high_score_of_given_matrix()
    {
        using namespace std::literals;
        using qrcode::structure::make_matrix;
        constexpr auto any_matrix = make_matrix<char>({17,17},
            "................+"   
            "................+"   
            "................+"   
            "................,"   
            "................,"   
            "................+"   
            "................+"   
            "................,"   
            "................+"   
            "................,"   
            "................+"
            "................,"   
            "................+"
            "................,"
            "................,"
            "................+"
            "+,,+++,,+,+,,+,,,"sv
        );
        static_assert(penalty_score(any_matrix).value == 104);
    }
}
#endif