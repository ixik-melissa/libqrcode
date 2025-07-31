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

#include <qrcode/micro_qr/symbol_designator.h>
#include <qrcode/micro_qr/symbol_number.h>
#include <qrcode/code/format_encoding.h>
#include <qrcode/structure/dimension.h>
#include <qrcode/structure/element_view.h>
#include <qrcode/structure/horizontal_view.h>
#include <qrcode/structure/matrix.h>
#include <qrcode/structure/module_traits.h>
#include <qrcode/structure/position.h>
#include <qrcode/structure/vertical_view.h>

namespace qrcode::micro_qr::detail
{   
    using qrcode::structure::position;
    using qrcode::structure::matrix;
    using qrcode::code::raw_format;
    using qrcode::code::error_correcting_code;

    [[nodiscard]] constexpr auto bch_code() noexcept
    {
        return error_correcting_code{15, 5, 0b101'0011'0111};
    }

    [[nodiscard]] constexpr auto format(symbol_designator const& designator, int mask_type) noexcept
    {
        return raw_format{
            static_cast<std::uint16_t>(
                static_cast<std::uint8_t>(symbol_number(designator) << 2) | 
                static_cast<std::uint8_t>(mask_type))
        };
    }

    [[nodiscard]] constexpr auto format_positions() noexcept
    {
        return std::array<position, 15>{{
            {8,1}, {8,2}, {8,3}, {8,4}, {8,5}, {8,6}, {8,7}, {8,8},
            {7,8}, {6,8}, {5,8}, {4,8}, {3,8}, {2,8}, {1,8}
        }};
    }
}

namespace qrcode::micro_qr
{
    using qrcode::structure::matrix;

    template<class T>
    [[nodiscard]] constexpr auto reserve_format_information(matrix<T>& matrix) noexcept
    {
        using qrcode::structure::module_traits;
        using qrcode::micro_qr::detail::format_positions;

        auto make_function = module_traits<T>::make_function;

        auto const positions = format_positions();
        for (auto& i : positions | views::element(matrix))
            i = make_function(0);
    }

    template<class T>
    [[nodiscard]] constexpr auto place_format_information(
        matrix<T>& matrix, symbol_designator const& designator, int data_mask) noexcept
    {
        using qrcode::micro_qr::detail::bch_code;
        using qrcode::micro_qr::detail::format;
        using qrcode::micro_qr::detail::format_positions;
        using qrcode::code::format_mask;
        using qrcode::structure::module_traits;
        
        auto const mask_pattern = format_mask{0b100'0100'0100'0101};
        auto format_bits = masked(
            encode(format(designator, data_mask), bch_code()), mask_pattern).data;

        auto make_function = module_traits<T>::make_function;
        
        auto const positions = format_positions();
        for (auto& i : positions | views::element(matrix))
        {
            i = make_function(format_bits & 1);
            format_bits >>= 1;
        }
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>
#include <qrcode/structure/make_matrix.h>

namespace qrcode::micro_qr::detail::test
{
    constexpr auto format_information_consists_of_error_correction_and_symbol_number_for_micro_qr_symbols()
    {
        constexpr auto any_designator = *make_designator(symbol_version::M4, error_correction::level_L);
        
        static_assert(format(any_designator, 0b00) == raw_format{0b10100});
        static_assert(format(any_designator, 0b01) == raw_format{0b10101});
        static_assert(format(any_designator, 0b10) == raw_format{0b10110});
        static_assert(format(any_designator, 0b11) == raw_format{0b10111});
    }
}

namespace qrcode::micro_qr::test
{
    constexpr auto format_information_is_placed_at_top_left_in_micro_qr_symbols()
    {
        auto f = []()
        {
            using namespace qrcode::structure;
            using namespace std::literals;
            constexpr auto any_designator = *make_designator(symbol_version::M1, std::nullopt);

            auto const any_data_mask = 0b11;
            auto any_matrix = matrix<char>({11,11}, module_traits<char>::make_free());

            place_format_information(any_matrix, any_designator, any_data_mask);

            return any_matrix == make_matrix<char>({11,11},
                "..........."
                "........-.."
                "........-.."
                "........*.."
                "........*.."
                "........*.."
                "........-.."
                "........-.."
                ".*--*-**-.."
                "..........."
                "..........."sv
            );
        };
        static_assert(f());
    }
}
#endif