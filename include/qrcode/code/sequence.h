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

#include <ranges>
#include <algorithm>
#include <qrcode/code/sequence_permutation.h>

namespace qrcode::code::detail
{
    template<class T>
    concept index_permutation = requires(T t)
    {
        { t(int{}) } -> std::convertible_to<int>;
    };

    template<class Iterator, class Lookup>
    requires std::random_access_iterator<Iterator> && index_permutation<Lookup>
    class storage
    {
    public:
        using value_type = std::iterator_traits<Iterator>::value_type;

        constexpr storage(Iterator storage_iterator, Lookup index_lookup) noexcept
        : iterator{std::move(storage_iterator)}
        , lookup{std::move(index_lookup)}
        , index{0} 
        {
        }

        template<class Range>
        requires std::ranges::range<Range> 
            && std::convertible_to<std::ranges::range_value_t<Range>, value_type>
        [[nodiscard]] constexpr auto store(Range const& codewords) noexcept
        {
            for (auto i : codewords)
            {
                auto address = iterator + lookup(index++);
                *address = i;
            }
        }

    private:
        Iterator iterator;
        Lookup lookup;
        int index;
    };
} 

namespace qrcode::code
{
    template<class T>
    class sequence
    {
    public:
        constexpr sequence(sequence_description const& description) noexcept
        : output(total_codewords(description))
        , data{begin(output), data_codeword_permutation(description)}
        , error{begin(output) + total_data_codewords(description), error_codeword_permutation(description)}
        {
        }
        
        template<std::ranges::range Data_Range, std::ranges::range Error_Range>
        requires std::convertible_to<std::ranges::range_value_t<Data_Range>, T>
            && std::convertible_to<std::ranges::range_value_t<Error_Range>, T>
        [[nodiscard]] constexpr auto next(
            Data_Range const& data_codewords, Error_Range const& error_codewords) noexcept
        {
            data.store(data_codewords);
            error.store(error_codewords);
        }
        
        [[nodiscard]] constexpr auto get() const noexcept
        {
            return output;
        }
        
    private:
        cx::vector<T> output;

        using output_iterator = typename decltype(output)::iterator;

        using data_lookup = decltype(data_codeword_permutation(std::declval<sequence_description>()));
        using error_lookup = decltype(error_codeword_permutation(std::declval<sequence_description>()));

        qrcode::code::detail::storage<output_iterator, data_lookup> data; 
        qrcode::code::detail::storage<output_iterator, error_lookup> error;
    };
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::code::detail::test
{
    constexpr auto storage_stores_single_value_data_at_identity()
    {
        auto f = []
        {
            auto const identity = [](auto i){ return i; };
            auto const any_codeword = std::array{9};
            auto memory = std::array{0};

            storage{begin(memory), identity}.store(any_codeword);

            return std::ranges::equal(memory, std::array{9});
        };
        static_assert(f());
    }

    constexpr auto storage_stores_codewords_in_identical_order_when_permutation_is_the_identity()
    {
        auto f = []
        {
            auto const identity = [](auto i){ return i; };
            auto const some_codewords = std::array{176, 8, 4, 1, 3};
            auto memory = std::array{0,0,0,0,0};

            storage{begin(memory), identity}.store(some_codewords);

            return std::ranges::equal(memory, some_codewords);
        };
        static_assert(f());
    }

    constexpr auto storage_stores_codewords_in_permutated_order()
    {
        auto f = []
        {
            auto const any_permutation = [p=std::array{2,1,4,3,0}](auto i){ return p[i]; };
            auto const some_codewords = std::array{176, 8, 4, 1, 3};
            auto memory = std::array{0,0,0,0,0};

            storage{begin(memory), any_permutation}.store(some_codewords);

            return std::ranges::equal(memory, std::array{3, 8, 176, 1, 4});
        };
        static_assert(f());
    }
}

namespace qrcode::code::test
{
    constexpr auto sequence_writes_data_in_standardized_order()
    {
        auto f = []
        {
            constexpr auto any_short_block_size = 3;
            constexpr auto any_short_block_count = 2;
            constexpr auto any_normal_block_size = 4;
            constexpr auto any_normal_block_count = 1;
            constexpr auto any_error_size = 2;

            auto seq = sequence<int>{sequence_description{
                {any_short_block_size, any_short_block_count}, 
                {any_normal_block_size, any_normal_block_count}, 
                any_error_size
            }};

            seq.next(std::array{0,1,2}, std::array{10,11});
            seq.next(std::array{3,4,5}, std::array{12,13});
            seq.next(std::array{6,7,8,9}, std::array{14,15});

            return std::ranges::equal(
                seq.get(),
                std::array
                {
                    0, 3, 6, 1, 4, 7,
                    2, 5, 8, 9, 10, 12,
                    14, 11, 13, 15
                }
            );
        };
        static_assert(f());
    }
}
#endif
