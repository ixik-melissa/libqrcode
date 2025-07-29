/*
 * The MIT License
 *
 * Copyright (c) 2021 Sebastian Bauer
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

#include <cx/vector.h>

#include <qrcode/code/sequence_description.h>

namespace qrcode::code::detail
{
    [[nodiscard]] constexpr auto data_indices(
        block_info const& short_block, block_info const& normal_block)
    {
        auto indices = cx::vector<int>{};

        auto c = 0;
        for (auto i = 0; i != short_block.block_count; ++i, c += short_block.data_size)
            indices.push_back(c);

        for (auto i = 0; i != normal_block.block_count; ++i, c += normal_block.data_size)
            indices.push_back(c);

        return indices;
    }

    template<std::forward_iterator Iterator>
    [[nodiscard]] constexpr auto fill_short_columns(
        Iterator iterator, block_info const& short_block, block_info const& normal_block) noexcept
    {
        auto const indices = data_indices(short_block, normal_block);
        for (auto x = 0; x != short_block.data_size; ++x)
        {
            iterator = std::transform(
                begin(indices), end(indices), iterator, [x](auto v) { return v + x; });
        }
        return iterator;
    }

    template<std::forward_iterator Iterator>
    [[nodiscard]] constexpr auto fill_normal_columns(
        Iterator iterator, block_info const& short_block, block_info const& normal_block) noexcept
    {
        auto normal_index = total_data(short_block) + normal_block.data_size - 1;

        for (auto i = 0; i != normal_block.block_count; ++i, normal_index+=normal_block.data_size)
            *(iterator++) = normal_index;
    }

    [[nodiscard]] constexpr auto data_lookup(
        block_info const& short_block, block_info const& normal_block)
    {
        auto lookup = cx::vector<int>(total_data(short_block) + total_data(normal_block));

        auto iterator = fill_short_columns(begin(lookup), short_block, normal_block);
        fill_normal_columns(iterator, short_block, normal_block);

        return lookup;
    }

    template<class T>
    concept indexable_range = requires(T t)
    {
        { t[int{}] } -> std::convertible_to<typename T::value_type>;
        requires std::ranges::range<T>;
    };

    template<indexable_range Container>
    [[nodiscard]] constexpr auto inverse(Container const& permutation)
    {
        auto inverted = permutation;
        auto count = 0;
        for (auto i : permutation)
            inverted[i] = count++; 
        return inverted;
    }
}
 
namespace qrcode::code
{   
    [[nodiscard]] constexpr auto data_codeword_permutation(sequence_description const& description)
    {
        using qrcode::code::detail::inverse;
        using qrcode::code::detail::data_lookup;

        auto const table = inverse(data_lookup(description.short_block, description.normal_block));
        return [=](auto i){ return table[i]; };
    }

    [[nodiscard]] constexpr auto error_codeword_permutation(sequence_description const& description)
    {
        auto const w = description.error_size;
        auto const h = total_blocks(description);
        return [=](auto i) { return h * (i % w) + i / w; };
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::code::detail::test
{
    constexpr auto data_indices_returns_an_array_containing_indices_for_each_row()
    {
        static_assert(std::ranges::equal(data_indices({4,1},{5,0}), std::array{0}));
        static_assert(std::ranges::equal(data_indices({4,2},{5,0}), std::array{0,4}));
        static_assert(std::ranges::equal(data_indices({4,0},{5,1}), std::array{0}));
        static_assert(std::ranges::equal(data_indices({4,0},{5,2}), std::array{0,5}));
        static_assert(std::ranges::equal(data_indices({4,2},{5,3}), std::array{0,4,8,13,18}));
        static_assert(std::ranges::equal(data_indices({4,3},{5,5}), std::array{0,4,8,12,17,22,27,32}));
    }

    constexpr auto data_lookup_returns_permutation_from_destination_to_source_index()
    {
        static_assert(std::ranges::equal(data_lookup({4,1},{5,0}), std::array{0,1,2,3}));
        static_assert(std::ranges::equal(data_lookup({4,1},{5,1}), std::array{0,4,1,5,2,6,3,7,8}));
        static_assert(std::ranges::equal(data_lookup({4,2},{5,1}), std::array{0,4,8,1,5,9,2,6,10,3,7,11,12}));
        static_assert(std::ranges::equal(data_lookup({2,2},{3,3}), std::array{0,2,4,7,10,1,3,5,8,11,6,9,12}));
    }

    constexpr auto inverse_inverts_given_permutation()
    {
        static_assert(std::ranges::equal(inverse(std::array{0,1,2,3}), std::array{0,1,2,3}));
        static_assert(std::ranges::equal(inverse(std::array{3,2,1,0}), std::array{3,2,1,0}));
        static_assert(std::ranges::equal(inverse(std::array{0,2,3,1}), std::array{0,3,1,2}));
        static_assert(std::ranges::equal(inverse(std::array{3,1,0,2}), std::array{2,1,3,0}));
    }
}

namespace qrcode::code
{
    constexpr auto data_codeword_permutation_returns_an_array_representing_the_lookup_table_where_to_place_data_codewords_in_the_sequence()
    {                                                        
        constexpr auto any_sequence = sequence_description{{2,1}, {3,2}, 6};

        constexpr auto permutation = data_codeword_permutation(any_sequence);
        
        static_assert(permutation(0) == 0);              // 0 1                      
        static_assert(permutation(1) == 3);              // 2 3 4 
        static_assert(permutation(2) == 1);              // 5 6 7  
        static_assert(permutation(3) == 4);              // => 0 2 5 1 3 6 4 7
        static_assert(permutation(4) == 6);              // => 0 3 1 4 6 2 5 7
        static_assert(permutation(5) == 2);
        static_assert(permutation(6) == 5);
        static_assert(permutation(7) == 7);
    }

    constexpr auto error_codeword_permutation_returns_an_array_represening_the_lookup_table_where_to_place_error_codewords_in_the_sequence()
    {                                                        
        constexpr auto any_sequence = sequence_description{{13,4},{14,1},3};

        constexpr auto permutation = error_codeword_permutation(any_sequence);
        
        static_assert(permutation(0) == 0);               //  0  1  2                     
        static_assert(permutation(1) == 5);               //  3  4  5
        static_assert(permutation(2) == 10);              //  6  7  8
        static_assert(permutation(3) == 1);               //  9 10 11
        static_assert(permutation(4) == 6);               // 12 13 14
        static_assert(permutation(5) == 11);              // => 0 3 6 9 12 1 4 7 10 13 2 5 8 11 14
        static_assert(permutation(6) == 2);               // => 0 5 10 1 6 11 2 7 12 3 8 13 4 9 14
        static_assert(permutation(7) == 7);
        static_assert(permutation(8) == 12);
        static_assert(permutation(9) == 3);
        static_assert(permutation(10) == 8);
        static_assert(permutation(11) == 13);
        static_assert(permutation(12) == 4);
        static_assert(permutation(13) == 9);
        static_assert(permutation(14) == 14);
    }
}
#endif
