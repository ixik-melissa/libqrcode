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

#include <qrcode/code/bit_view.h>
#include <qrcode/code/byte_view.h>
#include <qrcode/code/code_capacity.h>
#include <qrcode/code/codeword_view.h>
#include <qrcode/code/error_correction_polynomial.h>
#include <qrcode/code/padding_view.h>
#include <qrcode/code/polynomial_view.h>
#include <qrcode/code/sequence_description.h>
#include <qrcode/code/sequence_view.h>

namespace qrcode::qr::detail
{
    [[nodiscard]] constexpr auto data_padding()
    {
        return [index=0]() mutable noexcept
        {
            constexpr auto padding = std::array{std::byte{0b11101100}, std::byte{0b00010001}};
            return padding[(index++) % 2];
        };
    }

    [[nodiscard]] constexpr auto terminator_padding(int data_capacity)
    {
        return [count=4, data_capacity](auto index) mutable
        { 
            if (index >= data_capacity)
                return std::optional<bool>{};

            --count;
            return (count < 0) ? std::optional<bool>{} : std::optional<bool>{false};
        };
    }
}

namespace qrcode::qr
{
    using qrcode::code::code_capacity;

    template<std::ranges::range Range>
    requires std::convertible_to<std::ranges::range_value_t<Range>, bool>
    [[nodiscard]] constexpr auto code_bits(
        Range&& data_bits, code_capacity const& capacity) noexcept
    {
        using qrcode::qr::detail::data_padding;
        using qrcode::qr::detail::terminator_padding;
        using qrcode::code::gf2p8;
        using qrcode::code::polynomial_info_t;
        using qrcode::code::error_correction_polynomial;

        auto const description = make_sequence_description(capacity);

        auto as_gf2p8 = std::views::transform([](auto v) { return gf2p8{v}; });
        auto as_byte = std::views::transform([](auto v) { return static_cast<std::byte>(v); });
        auto terminator = code::views::padding(terminator_padding(total_data_bits(capacity)));

        auto const polynomial_info = polynomial_info_t{
            .degree=description.normal_block.data_size-1, 
            .short_polynomials=description.short_block.block_count
        };

        return data_bits
            | terminator
            | views::byte
            | views::padding(data_padding())
            | as_gf2p8
            | views::polynomial(polynomial_info)
            | views::codeword(error_correction_polynomial(error_degree(capacity)))
            | views::sequence(description)
            | as_byte
            | views::bit;
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::qr::detail::test
{
    constexpr auto qr_data_padding_returns_alternating_byte_pattern_as_described_in_the_qr_code_specification()
    {
        auto f = []
        {
            auto padding = data_padding();

            return padding() == std::byte{0b11101100}
                && padding() == std::byte{0b00010001}

                && padding() == std::byte{0b11101100}
                && padding() == std::byte{0b00010001}

                && padding() == std::byte{0b11101100}
                && padding() == std::byte{0b00010001}

                && padding() == std::byte{0b11101100}
                && padding() == std::byte{0b00010001}

                && padding() == std::byte{0b11101100}
                && padding() == std::byte{0b00010001}
                ;
        };
        static_assert(f());
    }

    constexpr auto terminator_padding_adds_up_to_four_bits_if_capacity_is_high_enough()
    {
        auto f = []
        {
            auto f = terminator_padding(15);
            return f(6) == std::optional<bool>{false}
                && f(7) == std::optional<bool>{false}
                && f(8) == std::optional<bool>{false}
                && f(9) == std::optional<bool>{false}
                && f(10) == std::optional<bool>{};
        };
        static_assert(f());
    }

    constexpr auto terminator_padding_stops_when_end_of_storage_has_been_reached()
    {
        auto f = []
        {
            auto f = terminator_padding(10);
            return f(8) == std::optional<bool>{false}
                && f(9) == std::optional<bool>{false}
                && f(10) == std::optional<bool>{};
        };
        static_assert(f());
    }
}

namespace qrcode::qr
{
    constexpr auto code_bits_adds_terminator_and_padding_and_error_correction_to_given_data_bits()
    {
        constexpr auto any_data = std::array<bool, 41>{{
            0, 0, 0, 1,
            0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
            0, 1, 0, 1, 0, 1, 1, 0, 0, 1,
            1, 0, 0, 0, 0, 1, 1
        }}; // 01234567
        constexpr auto any_capacity = code_capacity{{
            .total_data_bits = 128,
            .error_degree = 10,
            .total_blocks = 1,
        }};

        static_assert(std::ranges::equal(
            code_bits(any_data, any_capacity),
            std::array<bool,208>{
                0,0,0,1,0,0,0,0, 0,0,1,0,0,0,0,0, 0,0,0,0,1,1,0,0, 0,1,0,1,0,1,1,0, 0,1,1,0,0,0,0,1, 
                1,0,0,0,0,0,0,0, 1,1,1,0,1,1,0,0, 0,0,0,1,0,0,0,1, 1,1,1,0,1,1,0,0, 0,0,0,1,0,0,0,1, 
                1,1,1,0,1,1,0,0, 0,0,0,1,0,0,0,1, 1,1,1,0,1,1,0,0, 0,0,0,1,0,0,0,1, 1,1,1,0,1,1,0,0, 
                0,0,0,1,0,0,0,1, 1,0,1,0,0,1,0,1, 0,0,1,0,0,1,0,0, 1,1,0,1,0,1,0,0, 1,1,0,0,0,0,0,1, 
                1,1,1,0,1,1,0,1, 0,0,1,1,0,1,1,0, 1,1,0,0,0,1,1,1, 1,0,0,0,0,1,1,1, 0,0,1,0,1,1,0,0, 
                0,1,0,1,0,1,0,1
            }
        ));
    }
}
#endif
