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

namespace qrcode::code
{
    [[nodiscard]] constexpr auto bits_per_codeword() noexcept
    {
        return 8;
    }

    class code_capacity
    {
    public:
        struct init
        {
            int total_data_bits = 0;
            int error_degree = 0;
            int total_blocks = 0;
        };

        constexpr code_capacity(init const& init) noexcept
        : total_data_bits{init.total_data_bits}
        , error_degree{init.error_degree}
        , total_blocks{init.total_blocks}
        {
        }

        [[nodiscard]] friend constexpr auto total_blocks(code_capacity const& capacity) noexcept
        {
            return capacity.total_blocks;
        }

        [[nodiscard]] friend constexpr auto total_data_bits(code_capacity const& capacity) noexcept
        {
            return capacity.total_data_bits;
        }

        [[nodiscard]] friend constexpr auto error_degree(code_capacity const& capacity) noexcept
        {
            return capacity.error_degree;
        }

        [[nodiscard]] friend constexpr auto total_error_bits(code_capacity const& capacity) noexcept
        {
            return capacity.error_degree * bits_per_codeword() * capacity.total_blocks;
        }

        [[nodiscard]] constexpr auto operator==(code_capacity const&) const noexcept -> bool = default;

    private:
        int total_data_bits = 0;
        int error_degree = 0;
        int total_blocks = 0;
    };

    [[nodiscard]] constexpr auto total_bits(code_capacity const& capacity) noexcept
    {
        return total_data_bits(capacity) + total_error_bits(capacity); 
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::code::test
{
    constexpr auto code_capacity_is_all_zero_by_default()
    {
        static_assert(total_data_bits(code_capacity{{}}) == 0);
        static_assert(total_error_bits(code_capacity{{}}) == 0);
        static_assert(total_blocks(code_capacity{{}}) == 0);
        static_assert(total_bits(code_capacity{{}}) == 0);
        static_assert(error_degree(code_capacity{{}}) == 0);
    }

    constexpr auto total_data_bits_returns_the_number_of_data_bits_from_given_code_capacity()
    {
        constexpr auto any_error_degree = 2;
        constexpr auto any_total_blocks = 4;
        
        static_assert(total_data_bits(code_capacity{{
            .total_data_bits = 100, 
            .error_degree = any_error_degree, 
            .total_blocks = any_total_blocks}}) == 100);

        static_assert(total_data_bits(code_capacity{{
            .total_data_bits = 70, 
            .error_degree = any_error_degree, 
            .total_blocks = any_total_blocks}}) == 70);
    }

    constexpr auto error_degree_returns_the_degree_of_the_error_polynomial_generator_being_used_by_given_code_capacity()
    {
        constexpr auto any_total_data_bits = 10;
        constexpr auto any_total_blocks = 4;
        
        static_assert(error_degree(code_capacity{{
            .total_data_bits = any_total_data_bits, 
            .error_degree = 2, 
            .total_blocks = any_total_blocks}}) == 2);

        static_assert(error_degree(code_capacity{{
            .total_data_bits = any_total_data_bits, 
            .error_degree = 3, 
            .total_blocks = any_total_blocks}}) == 3);
    }

    constexpr auto total_blocks_returns_the_number_of_code_blocks_from_given_code_capacity()
    {
        constexpr auto any_total_data_bits = 10;
        constexpr auto any_error_degree = 5;
        
        static_assert(total_blocks(code_capacity{{
            .total_data_bits = any_total_data_bits, 
            .error_degree = any_error_degree, 
            .total_blocks = 1}}) == 1);

        static_assert(total_blocks(code_capacity{{
            .total_data_bits = any_total_data_bits, 
            .error_degree = any_error_degree, 
            .total_blocks = 2}}) == 2);
    }

    constexpr auto total_error_bits_returns_the_number_of_error_correction_bits_being_used_by_given_code_capacity()
    {
        constexpr auto any_total_data_bits = 10;
        
        static_assert(total_error_bits(code_capacity{{
            .total_data_bits = any_total_data_bits, 
            .error_degree = 4, 
            .total_blocks = 1}}) == (4*1*bits_per_codeword()));

        static_assert(total_error_bits(code_capacity{{
            .total_data_bits = any_total_data_bits, 
            .error_degree = 4, 
            .total_blocks = 2}}) == (4*2*bits_per_codeword()));

        static_assert(total_error_bits(code_capacity{{
            .total_data_bits = any_total_data_bits, 
            .error_degree = 7, 
            .total_blocks = 2}}) == (7*2*bits_per_codeword()));
    }

    constexpr auto total_bits_returns_the_total_number_of_bits_derived_from_given_code_capacity()
    {        
        static_assert(total_bits(code_capacity{{
            .total_data_bits = 10, 
            .error_degree = 4, 
            .total_blocks = 1}}) == (10 + 4*1*bits_per_codeword()));

        static_assert(total_bits(code_capacity{{
            .total_data_bits = 20, 
            .error_degree = 4, 
            .total_blocks = 1}}) == (20 + 4*1*bits_per_codeword()));

        static_assert(total_bits(code_capacity{{
            .total_data_bits = 20, 
            .error_degree = 4, 
            .total_blocks = 2}}) == (20 + 4*2*bits_per_codeword()));

        static_assert(total_bits(code_capacity{{
            .total_data_bits = 20, 
            .error_degree = 6, 
            .total_blocks = 2}}) == (20 + 6*2*bits_per_codeword()));
    }
}
#endif
