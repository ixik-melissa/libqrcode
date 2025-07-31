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

#include <concepts>

#include <qrcode/code/block_info.h>
#include <qrcode/code/code_capacity.h>

namespace qrcode::code::detail
{
    template<std::integral T>
    [[nodiscard]] constexpr auto ceiled_div(T x, T y) noexcept
    {
        return x / y + (((x < 0) ^ (y > 0)) && (x % y));
    }

    [[nodiscard]] constexpr auto derive_block_info(
        int block_count, int error_size, int total_codewords) noexcept
    {
        auto const block_size = total_codewords / block_count;
        auto const normal_block_count = block_count > 1 ? total_codewords % block_count : 0;
        auto const short_block_count = block_count > 1 ? block_count - normal_block_count : 1;

        auto const short_data_size = block_size - error_size;
        auto const normal_data_size = short_data_size + 1;

        return std::make_pair(
            block_info{.data_size = short_data_size, .block_count = short_block_count},
            block_info{.data_size = normal_data_size, .block_count = normal_block_count}
        );
    }
}

namespace qrcode::code
{
    struct sequence_description
    {
        block_info short_block{};
        block_info normal_block{};
        int error_size{0};
    };

    [[nodiscard]] constexpr auto make_sequence_description(code_capacity const& capacity) noexcept
    {
        using qrcode::code::detail::ceiled_div;
        using qrcode::code::detail::derive_block_info;
        
        auto const codeword_count = ceiled_div(total_bits(capacity), bits_per_codeword());
        auto const block_count = total_blocks(capacity);
        auto const error_data_size = error_degree(capacity);

        auto const [short_block, normal_block] = derive_block_info(
            block_count, error_data_size, codeword_count);

        return sequence_description{
            .short_block = short_block,
            .normal_block = normal_block, 
            .error_size = error_data_size
        };
    }

    [[nodiscard]] constexpr auto total_blocks(sequence_description const& sequence) noexcept
    {
        return sequence.short_block.block_count + sequence.normal_block.block_count;
    } 

    [[nodiscard]] constexpr auto total_codewords(sequence_description const& sequence) noexcept
    {
        return 
            total(sequence.short_block, sequence.error_size) 
            + total(sequence.normal_block, sequence.error_size);
    } 

    [[nodiscard]] constexpr auto total_data_codewords(sequence_description const& sequence) noexcept
    {
        return total_data(sequence.short_block) + total_data(sequence.normal_block);
    } 
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::code::detail::test
{
    constexpr auto ceiled_div_returns_the_quotient_of_two_numbers()
    {
        static_assert(ceiled_div(6, 1) == 6);
        static_assert(ceiled_div(6, 2) == 3);
        static_assert(ceiled_div(-1, 1) == -1);
        static_assert(ceiled_div(-1, -1) == 1);
        static_assert(ceiled_div(8, -4) == -2);
    }

    constexpr auto ceiled_div_returns_the_nearest_integer_not_less_than_the_quotient_of_the_two_given_numbers()
    {
        static_assert(ceiled_div(6, 7) == 1);
        static_assert(ceiled_div(6, 4) == 2);
        static_assert(ceiled_div(-2, 4) == 0);
        static_assert(ceiled_div(7, -3) == -2);
        static_assert(ceiled_div(-10, -4) == 3);
    }

    constexpr auto derive_block_info_returns_info_about_short_and_normal_blocks_in_the_sequence()
    {
        auto f = []
        {
            auto const [short_block, normal_block] = derive_block_info(8, 28, 404);
            return short_block == block_info{.data_size=22, .block_count=4}
                && normal_block == block_info{.data_size=23, .block_count=4};
        };
        static_assert(f());
    }
}

namespace qrcode::code::test
{
    constexpr auto sequence_description_must_be_initialized()
    {
        constexpr auto description = sequence_description{{1,2}, {11,12}, 5};

        static_assert(description.short_block.data_size == 1);
        static_assert(description.short_block.block_count == 2);
        static_assert(description.normal_block.data_size == 11);
        static_assert(description.normal_block.block_count == 12);
        static_assert(description.error_size == 5);
    }

    constexpr auto sequence_description_total_blocks_returns_the_number_of_blocks_existing_in_the_sequence()
    {
        constexpr auto any_error_size = 12;
        constexpr auto any_short_size = 5;
        constexpr auto any_short_normal = 6;

        static_assert(total_blocks(sequence_description{{any_short_size,0}, {any_short_normal,1}, any_error_size}) == 1);
        static_assert(total_blocks(sequence_description{{any_short_size,1}, {any_short_normal,1}, any_error_size}) == 2);
        static_assert(total_blocks(sequence_description{{any_short_size,1}, {any_short_normal,0}, any_error_size}) == 1);
        static_assert(total_blocks(sequence_description{{any_short_size,3}, {any_short_normal,18}, any_error_size}) == 21);
    }

    constexpr auto sequence_description_total_codewords_returns_the_number_of_codewords_in_the_sequence()
    {
        static_assert(total_codewords(sequence_description{{5,0}, {6,3}, 11}) == 51);
        static_assert(total_codewords(sequence_description{{5,2}, {6,0}, 11}) == 32);
        static_assert(total_codewords(sequence_description{{5,2}, {6,3}, 11}) == 83);
        static_assert(total_codewords(sequence_description{{5,2}, {6,3}, 7}) == 63);
    }

    constexpr auto sequence_description_total_data_codewords_returns_the_number_of_data_codewords_in_the_sequence()
    {
        static_assert(total_data_codewords(sequence_description{{5,0}, {6,3}, 11}) == 18);
        static_assert(total_data_codewords(sequence_description{{5,2}, {6,0}, 11}) == 10);
        static_assert(total_data_codewords(sequence_description{{5,2}, {6,3}, 11}) == 28);
    }
}
#endif
