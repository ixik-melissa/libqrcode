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

#include <numeric>
#include <ranges>

#include <qrcode/qr/penalty_weight.h>

#include <qrcode/structure/matrix.h>
#include <qrcode/structure/element_view.h>
#include <qrcode/structure/horizontal_view.h>
#include <qrcode/structure/module_traits.h>

namespace qrcode::qr::detail
{
    using qrcode::structure::matrix;

    template<class T>
    requires std::convertible_to<T, bool>
    [[nodiscard]] constexpr auto dark_module_count(matrix<T> const& matrix) noexcept
    {
        using qrcode::structure::module_traits;

        auto range = 
            views::horizontal({0,0}, size(matrix)) 
            | views::element(matrix) 
            | std::views::transform([](auto v){ return module_traits<T>::is_set(v); });

        auto sum = 0;
        std::ranges::for_each(range, [&sum](auto i) { sum += i; });
        return sum;
    }

    [[nodiscard]] constexpr auto nearest_five_percentage_step(float proportion) noexcept
    {
        auto abs = [](auto value){ return value >= 0.f ? value : -value; }; // constexpr abs
        return static_cast<int>(abs(50.f-proportion) / 5.f);
    }
}   

namespace qrcode::qr
{
    using qrcode::structure::matrix;

    template<class T>
    [[nodiscard]] constexpr auto dark_module_score(matrix<T> const& modules)
    {
        using qrcode::qr::detail::dark_module_count;
        using qrcode::qr::detail::nearest_five_percentage_step;

        auto const total = width(modules) * height(modules);
        auto const count = dark_module_count(modules);
        auto const proportion = 100.f * count / total;
        
        return nearest_five_percentage_step(proportion) * penalty_weight(4);
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <qrcode/structure/make_matrix.h>

namespace qrcode::qr::detail::test
{
    constexpr auto dark_module_count_returns_the_number_of_dark_modules()
    {
        static_assert([]()
        {
            using qrcode::structure::make_matrix;

            auto const some_modules = make_matrix<int>({7,3}, std::array{
                1,0,0,0,0,0,1,
                1,0,1,1,0,0,1,
                0,0,0,0,0,0,1
            });

            return dark_module_count(some_modules) == 7;
        }());
    }

    constexpr auto nearest_five_percentage_step_returns_the_distance_to_50_percent_in_five_percent_steps()
    {
        static_assert(nearest_five_percentage_step(50.f) == 0);
        static_assert(nearest_five_percentage_step(48.f) == 0);
        static_assert(nearest_five_percentage_step(43.f) == 1);
        static_assert(nearest_five_percentage_step(39.f) == 2);
        static_assert(nearest_five_percentage_step(33.f) == 3);
        static_assert(nearest_five_percentage_step(28.f) == 4);
        static_assert(nearest_five_percentage_step(22.f) == 5);
        static_assert(nearest_five_percentage_step(53.f) == 0);
        static_assert(nearest_five_percentage_step(58.f) == 1);
        static_assert(nearest_five_percentage_step(61.f) == 2);
        static_assert(nearest_five_percentage_step(66.f) == 3);
        static_assert(nearest_five_percentage_step(72.f) == 4);
        static_assert(nearest_five_percentage_step(79.f) == 5);
    }
}

namespace qrcode::qr::test
{
    constexpr auto dark_module_score_returns_the_penalty_score_derived_from_number_of_five_percent_steps_away_from_50_percent_dark_modules()
    {
        static_assert([]()
        {
            using qrcode::structure::make_matrix;

            auto const some_modules = make_matrix<int>({7,3}, std::array{
                1,0,0,0,0,0,1,
                1,0,1,1,0,0,1,
                0,0,0,0,0,0,1
            });

            return dark_module_score(some_modules) == (3 * penalty_weight(4));
        }());
    }
}
#endif