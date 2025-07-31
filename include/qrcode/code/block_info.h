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

namespace qrcode::code
{
    struct block_info
    {
        int data_size = 0;
        int block_count = 0;

        [[nodiscard]] constexpr auto operator==(block_info const&) const noexcept -> bool = default;
        [[nodiscard]] constexpr auto operator!=(block_info const&) const noexcept -> bool = default;
    };

    [[nodiscard]] constexpr auto total_data(block_info const& info) 
    { 
        return info.data_size * info.block_count; 
    }

    [[nodiscard]] constexpr auto total(block_info const& info, int error_size) 
    { 
        return (info.data_size + error_size) * info.block_count; 
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::code::test
{
    constexpr auto total_data_returns_the_total_number_of_data_codewords_in_the_sequence()
    {
        static_assert(total_data({0,0}) == 0);
        static_assert(total_data({1,1}) == 1);
        static_assert(total_data({2,1}) == 2);
        static_assert(total_data({3,1}) == 3);
        static_assert(total_data({1,2}) == 2);
        static_assert(total_data({1,3}) == 3);
        static_assert(total_data({6,3}) == 18);
        static_assert(total_data({6,7}) == 42);
    }

    constexpr auto total_returns_the_total_number_of_codewords_including_data_and_error_codewords_in_the_sequence()
    {
        static_assert(total({0,0},0) == 0);
        static_assert(total({1,1},0) == 1);
        static_assert(total({1,1},1) == 2);
        static_assert(total({1,1},2) == 3);
        static_assert(total({2,1},2) == 4);
        static_assert(total({3,1},2) == 5);
        static_assert(total({3,5},2) == 25);
        static_assert(total({7,8},2) == 72);
        static_assert(total({7,8},3) == 80);
    }
}
#endif