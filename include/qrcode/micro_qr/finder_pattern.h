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

#include <qrcode/structure/dimension.h>
#include <qrcode/structure/matrix.h>
#include <qrcode/structure/module_traits.h>
#include <qrcode/structure/position.h>
#include <qrcode/structure/horizontal_view.h>
#include <qrcode/structure/element_view.h>

namespace qrcode::micro_qr
{
    using qrcode::structure::matrix;
    using qrcode::structure::position;
    using qrcode::structure::dimension;
    using qrcode::structure::module_traits;

    class finder_pattern
    {
        static constexpr auto extend = dimension{7,7};

    public:
        [[nodiscard]] constexpr auto size() const noexcept
        {
            return extend;
        }

        template<class T>
        requires std::convertible_to<std::invoke_result_t<
            decltype(module_traits<T>::make_function), bool>, T>
        [[nodiscard]] constexpr auto place(matrix<T>& matrix, position top_left) const noexcept
        {
            using qrcode::structure::module_traits;
            using std::ranges::begin;
            auto const pattern = std::array<bool,width(extend)*height(extend)>{
                1,1,1,1,1,1,1,
                1,0,0,0,0,0,1,
                1,0,1,1,1,0,1,
                1,0,1,1,1,0,1,
                1,0,1,1,1,0,1,
                1,0,0,0,0,0,1,
                1,1,1,1,1,1,1
            };
            auto const bits = pattern | std::views::transform(module_traits<T>::make_function);

            auto range = views::horizontal(top_left, extend) | views::element(matrix);

            std::ranges::copy(bits, begin(range));
        }
    };

    [[nodiscard]] constexpr auto finder_location() noexcept
    {
        return position{0,0};
    }

    template<class T>
    [[nodiscard]] constexpr auto place_finder_pattern(matrix<T>& matrix) noexcept
    {
        auto const pattern = finder_pattern{};
        pattern.place(matrix, finder_location());
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>
#include <qrcode/structure/make_matrix.h>

namespace qrcode::micro_qr::test
{
    constexpr auto place_finder_pattern_places_pattern_at_the_top_left_for_micro_qr_symbols()
    {        
        auto f = []()
        {
            using namespace std::literals;
            using namespace qrcode::structure;
            auto any_matrix = matrix<char>{{11,11}, module_traits<char>::make_free()};

            place_finder_pattern(any_matrix);

            return any_matrix == make_matrix<char>({11,11},
                "*******...."
                "*-----*...."
                "*-***-*...."
                "*-***-*...."
                "*-***-*...."
                "*-----*...."
                "*******...."
                "..........."
                "..........."
                "..........."
                "..........."sv
            );
        };
        static_assert(f());
    }
}
#endif