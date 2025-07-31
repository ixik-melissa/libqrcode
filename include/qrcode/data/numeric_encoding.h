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
#include <qrcode/data/bit_stream.h>
#include <qrcode/data/numeric.h>

namespace qrcode::data::detail
{
    template<class...Digits>
    requires std::conjunction_v<std::is_convertible<Digits, numeric>...>
    [[nodiscard]] constexpr auto decimal_number(Digits...digits) noexcept
    {
        auto result = 0;
        for (auto i : {digits...})
            result = 10 * result + number(i);
        return result;
    }

    [[nodiscard]] constexpr auto bit_count(int decimal_count) noexcept
    {
        constexpr auto log2_10 = 3.321928095;
        // below: constexpr of ceil(log2_10 * decimal_count)
        return static_cast<int>(1 + log2_10 * decimal_count); 
    }

    template<class...Digits>
    requires std::conjunction_v<std::is_convertible<Digits, numeric>...>
    [[nodiscard]] constexpr auto encode_digits(bit_stream& stream, Digits...digits) noexcept 
    {
        stream.add(decimal_number(digits...), number_bits{bit_count(sizeof...(digits))}); 
    }
}

namespace qrcode::data
{
    template<std::ranges::sized_range Message>
    requires std::convertible_to<std::ranges::range_value_t<Message>, numeric>
    [[nodiscard]] constexpr auto encode_numerics(bit_stream& stream, Message&& message) noexcept
    {
        using qrcode::data::detail::encode_digits;
        using std::ranges::begin;
        using std::ranges::end;
        using std::ranges::size;

        auto const triple_count = size(message) / 3;
        auto const left_over = size(message) % 3;

        auto const triples_end = begin(message) + 3 * triple_count;
        
        auto i = begin(message);
        for (; i != triples_end; i += 3)
            encode_digits(stream, *i, *(i+1), *(i+2));

        if (left_over == 1)
            encode_digits(stream, *i);

        if (left_over == 2)
            encode_digits(stream, *i, *(i+1));
    }

    template<std::ranges::sized_range Message>
    [[nodiscard]] constexpr auto numeric_character_count(Message&& message) noexcept
    {
        using std::ranges::size;
        return static_cast<int>(size(message));
    }

    template<std::ranges::sized_range Message>
    [[nodiscard]] constexpr auto numeric_bit_count(Message&& message) noexcept
    {
        constexpr auto digit_group = 3;
        constexpr auto remainders_bits = std::array{0,4,7};

        auto const number_characters = numeric_character_count(message);
        auto const remainder = number_characters % digit_group;

        return 10 * (number_characters/digit_group) + remainders_bits[remainder];
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>

namespace qrcode::data::detail::test
{
    constexpr auto bit_count_returns_the_number_of_bits_needed_to_represent_an_integer_with_a_given_number_of_decimal_digits()
    {
        static_assert(bit_count(1) == 4);
        static_assert(bit_count(2) == 7);
        static_assert(bit_count(3) == 10);
        static_assert(bit_count(4) == 14);
    }

    constexpr auto decimal_number_takes_a_number_of_numeric_digits_and_converts_them_to_an_integer()
    {
        using n = numeric;

        static_assert(decimal_number(n::n0) == 0);
        static_assert(decimal_number(n::n1) == 1);
        static_assert(decimal_number(n::n6) == 6);
        static_assert(decimal_number(n::n1,n::n2) == 12);
        static_assert(decimal_number(n::n8,n::n2) == 82);
        static_assert(decimal_number(n::n3,n::n4,n::n5) == 345);
        static_assert(decimal_number(n::n0,n::n1,n::n2,n::n3,n::n4,n::n5) == 12345);
    }

    constexpr auto encode_digits_takes_a_sequence_of_decimal_digits_encoded_as_characters_and_converts_them_to_their_bitwise_representation()
    {
        auto f = [](auto&&...digits)
        {
            auto stream = bit_stream{};

            encode_digits(stream, digits...);

            return stream.get();
        };

        using n = numeric;

        static_assert(std::ranges::equal(f(n::n0,n::n1,n::n2), std::array<bool,10>{0,0,0,0,0,0,1,1,0,0}));
        static_assert(std::ranges::equal(f(n::n3,n::n4,n::n5), std::array<bool,10>{0,1,0,1,0,1,1,0,0,1}));
        static_assert(std::ranges::equal(f(n::n0,n::n1,n::n2,n::n3,n::n4,n::n5), std::array<bool,20>{0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,1,0,0,1}));
        static_assert(std::ranges::equal(f(n::n6), std::array<bool,4>{0,1,1,0}));
        static_assert(std::ranges::equal(f(n::n8,n::n2), std::array<bool,7>{1,0,1,0,0,1,0}));
    }

    constexpr auto encode_numerics_groups_three_digits_to_one_number_and_converts_these_groups_to_their_bitwise_representation()
    {
        auto f = [](auto digits)
        {
            auto stream = bit_stream{}; 
            
            encode_numerics(stream, digits);

            return stream.get();
        };

        using n = numeric;
        static_assert(std::ranges::equal(f(std::array{n::n0,n::n1,n::n2}), std::array<bool,10>{0,0,0,0,0,0,1,1,0,0}));
        static_assert(std::ranges::equal(f(std::array{n::n3,n::n4,n::n5}), std::array<bool,10>{0,1,0,1,0,1,1,0,0,1}));
        static_assert(std::ranges::equal(f(std::array{n::n0,n::n1,n::n2,n::n3,n::n4,n::n5}), std::array<bool,20>{0,0,0,0,0,0,1,1,0,0,0,1,0,1,0,1,1,0,0,1}));
    }    
}

namespace qrcode::data::test
{
    constexpr auto encode_numerics_groups_the_left_overs_to_group_with_less_then_three_digits_and_converts_that_group_to_their_bitwise_representation()
    {
        auto f = [](auto digits)
        {
            auto stream = bit_stream{}; 
            
            encode_numerics(stream, digits);

            return stream.get();
        };

        using n = numeric;

        static_assert(std::ranges::equal(f(std::array{n::n6}), std::array<bool,4>{0,1,1,0}));
        static_assert(std::ranges::equal(f(std::array{n::n8,n::n2}), std::array<bool,7>{1,0,1,0,0,1,0}));
        static_assert(std::ranges::equal(f(std::array{n::n8,n::n2,n::n1,n::n3}), std::array<bool,14>{1,1,0,0,1,1,0,1,0,1,0,0,1,1}));
        static_assert(std::ranges::equal(f(std::array{n::n8,n::n2,n::n1,n::n3,n::n4}), std::array<bool,17>{1,1,0,0,1,1,0,1,0,1,0,1,0,0,0,1,0}));
        static_assert(std::ranges::equal(f(std::array{n::n8,n::n6,n::n7,n::n5,n::n3,n::n0,n::n9}), std::array<bool,24>{1,1,0,1,1,0,0,0,1,1,1,0,0,0,0,1,0,0,1,0,1,0,0,1}));
    }

    constexpr auto numeric_character_count_returns_the_number_of_characters_which_can_be_encoded_from_given_message()
    {
        using namespace std::literals;
        static_assert(numeric_character_count(""sv) == 0);
        static_assert(numeric_character_count("1"sv) == 1);
        static_assert(numeric_character_count("12"sv) == 2);
        static_assert(numeric_character_count("123"sv) == 3);
        static_assert(numeric_character_count("1234"sv) == 4);
        static_assert(numeric_character_count("12345"sv) == 5);
        static_assert(numeric_character_count("123456"sv) == 6);
        static_assert(numeric_character_count("1234567"sv) == 7);
    }

    constexpr auto numeric_bit_count_returns_the_number_of_bits_needed_to_represent_given_message()
    {
        using namespace std::literals;
        static_assert(numeric_bit_count(""sv) == 0);
        static_assert(numeric_bit_count("1"sv) == 4);
        static_assert(numeric_bit_count("12"sv) == 7);
        static_assert(numeric_bit_count("123"sv) == 10);
        static_assert(numeric_bit_count("1234"sv) == (10+4));
        static_assert(numeric_bit_count("12345"sv) == (10+7));
        static_assert(numeric_bit_count("123456"sv) == (10+10));
        static_assert(numeric_bit_count("1234567"sv) == (10+10+4));
        static_assert(numeric_bit_count("12345678"sv) == (10+10+7));
        static_assert(numeric_bit_count("123456789"sv) == (10+10+10));
    }
}
#endif
