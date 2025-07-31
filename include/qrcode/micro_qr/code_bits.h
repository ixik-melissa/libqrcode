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
#include <qrcode/code/sequence_view.h>

namespace qrcode::micro_qr::detail
{
    [[nodiscard]] constexpr auto skip_range(int start_index, int count) noexcept
    {
        return std::views::filter([index=start_index, count=count](auto) mutable
        {
            --index;
            if (index >= 0)
                return true;
            
            --count;
            return count < 0;
        });
    }
}

namespace qrcode::micro_qr
{
    using qrcode::code::code_capacity;

    template<std::ranges::range Range>
    requires std::convertible_to<std::ranges::range_value_t<Range>, bool>
    [[nodiscard]] constexpr auto code_bits(
        Range&& data_bits, code_capacity const& capacity) noexcept
    {
        using qrcode::micro_qr::detail::skip_range;
        using qrcode::code::gf2p8;
        using qrcode::code::polynomial_info_t;
        using qrcode::code::error_correction_polynomial;
        using qrcode::code::bits_per_codeword;

        auto const data_padding = []() { return std::byte{0}; };
        auto const description = make_sequence_description(capacity);
        auto const data_bit_count = total_data_bits(capacity);

        auto as_gf2p8 = std::views::transform([](auto v) { return gf2p8{v}; });
        auto as_byte = std::views::transform([](auto v) { return static_cast<std::byte>(v); });

        auto const polynomial_info = polynomial_info_t{
            .degree=description.normal_block.data_size-1, 
            .short_polynomials=description.short_block.block_count
        };

        return data_bits
            | views::byte
            | views::padding(data_padding)
            | as_gf2p8
            | views::polynomial(polynomial_info)
            | views::codeword(error_correction_polynomial(error_degree(capacity)))
            | views::sequence(description)
            | as_byte
            | views::bit
            | skip_range(data_bit_count, data_bit_count % bits_per_codeword());
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>
#include <qrcode/structure/make_matrix.h>

namespace qrcode::micro_qr::test
{
    constexpr auto code_bits_adds_terminator_and_padding_and_error_correction_to_given_data_bits()
    {
        constexpr auto any_data = std::array<bool, 40>{{ 
            0, 1, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 1, 1, 0, 0, 0,
            1, 0, 1, 0, 1, 1, 0, 0,
            1, 1, 0, 0, 0, 0, 1, 1,
            0, 0, 0, 0, 0, 0, 0, 0
        }}; // 01234567
        constexpr auto any_capacity = code_capacity{{
            .total_data_bits = 40,
            .error_degree = 5,
            .total_blocks = 1,
        }};

        static_assert(std::ranges::equal(
            code_bits(any_data, any_capacity),
            std::array<bool,80>{
                0,1,0,0,0,0,0,0, 0,0,0,1,1,0,0,0, 1,0,1,0,1,1,0,0, 1,1,0,0,0,0,1,1, 0,0,0,0,0,0,0,0,
                1,0,0,0,0,1,1,0, 0,0,0,0,1,1,0,1, 0,0,1,0,0,0,1,0, 1,0,1,0,1,1,1,0, 0,0,1,1,0,0,0,0
            }
        ));
    }
}
#endif