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

#include <qrcode/qr/penalty_weight.h>
#include <qrcode/structure/element_view.h>
#include <qrcode/structure/horizontal_view.h>
#include <qrcode/structure/module_traits.h>

namespace qrcode::qr::detail
{
    using qrcode::structure::position;
    using qrcode::structure::matrix;

    template<std::ranges::viewable_range Range>
    [[nodiscard]] constexpr auto adjacent_score(Range&& line)
    {
        using std::ranges::empty;
        using std::ranges::begin;
        using std::ranges::end;

        if (empty(line))
            return 0;

        auto previous = *begin(line);
        auto next = begin(line); 
        ++next;

        auto count = 1;
        auto score = 0;

        using qrcode::structure::module_traits;
        using value_type = std::ranges::range_value_t<Range>;
        auto is_set = module_traits<value_type>::is_set;

        for (auto i : std::ranges::subrange{next, end(line)})
        {
            count = is_set(i) == is_set(previous) ? count + 1 : 1;
            previous = i;

            if (count == 5)
                score += penalty_weight(1);

            if (count > 5)
                ++score;
        }

        return score;
    }
    
    template<class T>
    [[nodiscard]] constexpr auto horizontal_adjacent_score(matrix<T> const& modules)
    {
        auto score = 0;
        for (auto y = 0; y != height(modules); ++y)
        {
            auto range = views::horizontal({0,y}, {width(modules),1}) | views::element(modules);
            score += adjacent_score(range);
        }
        return score;
    }

    template<class T>
    [[nodiscard]] constexpr auto vertical_adjacent_score(matrix<T> const& modules)
    {
        auto score = 0;
        for (auto x = 0; x != width(modules); ++x)
        {
            auto range = views::horizontal({x,0}, {1,height(modules)}) | views::element(modules);
            score += adjacent_score(range);
        }
        return score;
    }
}

namespace qrcode::qr
{
    using qrcode::structure::matrix;

    template<class T>
    [[nodiscard]] constexpr auto adjacent_score(matrix<T> const& modules)
    {
        using qrcode::qr::detail::horizontal_adjacent_score;
        using qrcode::qr::detail::vertical_adjacent_score;
        return horizontal_adjacent_score(modules) + vertical_adjacent_score(modules);
    }
}   

#ifdef QRCODE_TESTS_ENABLED
#include <qrcode/structure/make_matrix.h>

namespace qrcode::qr::detail::test
{
    constexpr auto horizontal_adjacent_score_calculates_penalty_score_of_adjacent_modules_in_column_having_the_same_value_for_given_matrix()
    {
        auto f = []()
        {
            using namespace qrcode::structure;

            auto some_modules = make_matrix<int>({7,3}, std::array{
                1,0,0,0,0,0,1,
                1,0,1,1,0,0,1,
                0,0,0,0,0,0,1
            });

            return horizontal_adjacent_score(some_modules) == (2*penalty_weight(1)+1);
        };
        static_assert(f());
    }

    constexpr auto vertical_adjacent_score_calculates_penalty_score_of_adjacent_modules_in_row_having_the_same_value_for_given_matrix()
    {
        auto f = []()
        {
            using namespace qrcode::structure;

            auto some_modules = make_matrix<int>({3,7}, std::array{
                1,1,0,
                0,0,0,
                0,1,0,
                0,1,0,
                0,0,0,
                0,0,0,
                1,1,1
            });

            return vertical_adjacent_score(some_modules) == (2*penalty_weight(1)+1);
        };
        static_assert(f());
    }

    constexpr auto adjacent_score_calculates_penalty_score_of_adjacent_modules_having_the_same_value_in_given_range()
    {
        constexpr auto no_penalty = std::array{1,1,1,0,0,0,0};
        constexpr auto single_penalty = std::array{1,1,1,1,1};
        constexpr auto single_penalty_and_once_more = std::array{0,0,0,0,0,0};
        constexpr auto single_penalty_and_two_more = std::array{1,1,1,1,1,1,1};
        constexpr auto double_penalty = std::array{1,1,1,1,1,0,0,0,0,0};
        constexpr auto double_penalty_and_once_more = std::array{0,0,0,0,0,1,1,1,1,1,1};
        constexpr auto double_penalty_and_two_more = std::array{0,0,0,0,0,1,1,1,1,1,1,1};

        constexpr auto more_sophisticated = std::array{0, 1,1, 0,0,0,0, 1,1,1,1,1, 0,0,1, 0,0,0,0,0,0, 1,1,1,1,1,1,1};

        static_assert(adjacent_score(no_penalty) == 0);
        static_assert(adjacent_score(single_penalty) == penalty_weight(1));
        static_assert(adjacent_score(single_penalty_and_once_more) == penalty_weight(1)+1);
        static_assert(adjacent_score(single_penalty_and_two_more) == penalty_weight(1)+2);

        static_assert(adjacent_score(double_penalty) == (2*penalty_weight(1)));
        static_assert(adjacent_score(double_penalty_and_once_more) == (2*penalty_weight(1)+1));
        static_assert(adjacent_score(double_penalty_and_two_more) == (2*penalty_weight(1)+2));

        static_assert(adjacent_score(more_sophisticated) == (3*penalty_weight(1) + 3));
    }
}

namespace qrcode::qr::test
{
    constexpr auto adjacent_score_calculates_overvall_score_using_the_vertical_adjacent_score_and_horizontal_adjacent_score()
    {
        using qrcode::structure::make_matrix;

        static_assert(adjacent_score(make_matrix<int>({6,3}, std::array{
            1,1,1,1,1,0,
            1,0,1,0,0,0,
            0,0,0,1,0,1
        })) == penalty_weight(1));

        static_assert(adjacent_score(make_matrix<int>({6,6}, std::array{
            1,1,0,1,1,0,
            1,1,1,0,0,0,
            0,1,0,1,0,1,
            0,1,0,0,0,0,
            0,1,1,0,1,0,
            1,1,0,0,0,1
        })) == (penalty_weight(1)+1));

        static_assert(adjacent_score(make_matrix<int>({6,6}, std::array{
            1,1,1,1,1,0,
            1,1,1,0,0,0,
            0,1,0,1,0,1,
            0,1,0,0,0,0,
            0,1,1,0,1,0,
            1,1,0,0,0,1
        })) == (penalty_weight(1)+1+penalty_weight(1)));
    }
}
#endif