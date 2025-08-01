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
#include <qrcode/structure/matrix.h>
#include <qrcode/structure/module_traits.h>
#include <qrcode/structure/position.h>
#include <qrcode/structure/horizontal_view.h>
#include <qrcode/structure/element_view.h>

namespace qrcode::qr
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

    [[nodiscard]] constexpr auto size(finder_pattern const& pattern) noexcept
    {
        return pattern.size();
    }

    [[nodiscard]] constexpr auto finder_locations(dimension symbol_size) noexcept
    {
        auto const pattern = size(finder_pattern{});
        auto const left = width(symbol_size) - width(pattern);
        auto const bottom = height(symbol_size) - height(pattern);
        return std::array<position,3>{{{0,0}, {left,0}, {0, bottom}}};
    }

    template<class T>
    [[nodiscard]] constexpr auto place_finder_patterns(matrix<T>& matrix) noexcept
    {
        auto const pattern = finder_pattern{};
        for (auto i : finder_locations(size(matrix)))
            pattern.place(matrix, i);
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>
#include <qrcode/structure/make_matrix.h>

namespace qrcode::qr::test
{
    constexpr auto finder_patterns_in_qr_symols_have_a_size_of_7x7()
    {
        static_assert(size(finder_pattern{}) == dimension{7,7});
    }

    constexpr auto finder_patterns_can_be_placed_on_given_matrix_at_a_given_position()
    {
        auto f = []
        {
            using namespace std::literals;
            using namespace qrcode::structure;
            auto any_matrix = matrix<char>{{10,10}, module_traits<char>::make_free()};
            auto const any_position = position{2,1};

            finder_pattern{}.place(any_matrix, any_position);

            return any_matrix == make_matrix<char>({10,10},
                ".........."
                "..*******."
                "..*-----*."
                "..*-***-*."
                "..*-***-*."
                "..*-***-*."
                "..*-----*."
                "..*******."
                ".........."
                ".........."sv
            );
        };
        static_assert(f());
    }

    constexpr auto finder_patterns_are_located_at_three_different_places_in_qr_symbols()
    {
        static_assert(size(finder_locations(dimension{21,21})) == 3);
        static_assert(size(finder_locations(dimension{25,25})) == 3);
        static_assert(size(finder_locations(dimension{29,29})) == 3);
    }

    constexpr auto finder_patterns_are_placed_at_top_left_of_qr_symbols()
    {
        static_assert(std::ranges::any_of(
            finder_locations(dimension{21,21}), [](auto v) { return v == position{0,0}; }));
        static_assert(std::ranges::any_of(
            finder_locations(dimension{25,25}), [](auto v) { return v == position{0,0}; }));
        static_assert(std::ranges::any_of(
            finder_locations(dimension{29,29}), [](auto v) { return v == position{0,0}; }));
    }

    constexpr auto finder_patterns_are_placed_at_bottom_left_of_qr_symbols()
    {
        static_assert(std::ranges::any_of(
            finder_locations(dimension{21,21}), [](auto v) { return v == position{14,0}; }));

        static_assert(std::ranges::any_of(
            finder_locations(dimension{25,25}), [](auto v) { return v == position{18,0}; }));

        static_assert(std::ranges::any_of(
            finder_locations(dimension{29,29}), [](auto v) { return v == position{22,0}; }));

        static_assert(std::ranges::any_of(
            finder_locations(dimension{33,33}), [](auto v) { return v == position{26,0}; }));

        static_assert(std::ranges::any_of(
            finder_locations(dimension{37,37}), [](auto v) { return v == position{30,0}; }));
    }

    constexpr auto finder_patterns_are_placed_at_top_right_of_qr_symbols()
    {
        static_assert(std::ranges::any_of(
            finder_locations(dimension{21,21}), [](auto v) { return v == position{0,14}; }));

        static_assert(std::ranges::any_of(
            finder_locations(dimension{25,25}), [](auto v) { return v == position{0,18}; }));

        static_assert(std::ranges::any_of(
            finder_locations(dimension{29,29}), [](auto v) { return v == position{0,22}; }));

        static_assert(std::ranges::any_of(
            finder_locations(dimension{33,33}), [](auto v) { return v == position{0,26}; }));

        static_assert(std::ranges::any_of(
            finder_locations(dimension{37,37}), [](auto v) { return v == position{0,30}; }));
    }

    constexpr auto place_finder_patterns_places_patterns_for_qr_symbols()
    {        
        auto f = []
        {
            using namespace std::literals;
            using namespace qrcode::structure;
            auto any_matrix = matrix<char>{{21,21}, module_traits<char>::make_free()};

            place_finder_patterns(any_matrix);
            
            return any_matrix == make_matrix<char>({21,21},
                "*******.......*******"
                "*-----*.......*-----*"
                "*-***-*.......*-***-*"
                "*-***-*.......*-***-*"
                "*-***-*.......*-***-*"
                "*-----*.......*-----*"
                "*******.......*******"
                "....................."
                "....................."
                "....................."
                "....................."
                "....................."
                "....................."
                "....................."
                "*******.............."
                "*-----*.............."
                "*-***-*.............."
                "*-***-*.............."
                "*-***-*.............."
                "*-----*.............."
                "*******.............."sv
            );
        };
        static_assert(f());
    }
}
#endif