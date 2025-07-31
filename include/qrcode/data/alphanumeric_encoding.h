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

#include <qrcode/data/alphanumeric.h>
#include <qrcode/data/bit_stream.h>

namespace qrcode::data::detail
{
    [[nodiscard]] constexpr auto alphanumeric_pair_size()
    {
        return 11_bits;
    }

    [[nodiscard]] constexpr auto alphanumeric_single_size()
    {
        return 6_bits;
    }

    [[nodiscard]] constexpr auto alphanumeric_pair(
        bit_stream& stream, alphanumeric first, alphanumeric second) noexcept
    {
        stream.add(45 * number(first) + number(second), alphanumeric_pair_size());
    }

    [[nodiscard]] constexpr auto single_alphanumeric(
        bit_stream& stream, alphanumeric symbol) noexcept
    {
        stream.add(number(symbol), alphanumeric_single_size());
    }
}

namespace qrcode::data
{
    template<std::ranges::forward_range Message>
    requires std::same_as<std::ranges::range_value_t<Message>, alphanumeric>
    [[nodiscard]] constexpr auto encode_alphanumerics(bit_stream& stream, Message&& message) noexcept
    {
        using qrcode::data::detail::single_alphanumeric;
        using qrcode::data::detail::alphanumeric_pair;
        using std::ranges::begin;
        using std::ranges::end;

        auto current = begin(message);
        for (auto i = begin(message); i != end(message); ++i, ++current)
        {
            ++i;
            if (i == end(message))
                break;
            alphanumeric_pair(stream, *current, *i);
            current = i;
        }
        if (current != end(message))
            single_alphanumeric(stream, *current);
    }

    template<std::ranges::sized_range Message>
    [[nodiscard]] static constexpr auto alphanumeric_character_count(Message&& message) noexcept
    {
        using std::ranges::size;
        return static_cast<int>(size(message));
    }

    template<std::ranges::sized_range Message>
    [[nodiscard]] static constexpr auto alphanumeric_bit_count(Message&& message) noexcept
    {
        using qrcode::data::detail::alphanumeric_single_size;
        using qrcode::data::detail::alphanumeric_pair_size;

        auto const number_characters = alphanumeric_character_count(std::forward<Message>(message));
        
        return (number_characters / 2) * alphanumeric_pair_size().value 
            + (number_characters % 2) * alphanumeric_single_size().value;
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>

namespace qrcode::data::detail::test
{
    constexpr auto alphanumeric_pair_groups_two_alphanummerics_and_returns_their_bitwise_representation()
    {
        auto f = [](auto a, auto b)
        {
            auto stream = bit_stream{}; 

            alphanumeric_pair(stream, a, b);

            return stream.get();
        };

        using a = alphanumeric;

        static_assert(std::ranges::equal(f(a::code_0, a::code_0), std::array<bool,11>{0,0,0,0,0,0,0,0,0,0,0}));
        static_assert(std::ranges::equal(f(a::code_0, a::code_1), std::array<bool,11>{0,0,0,0,0,0,0,0,0,0,1}));
        static_assert(std::ranges::equal(f(a::code_0, a::code_2), std::array<bool,11>{0,0,0,0,0,0,0,0,0,1,0}));
        static_assert(std::ranges::equal(f(a::code_0, a::code_A), std::array<bool,11>{0,0,0,0,0,0,0,1,0,1,0}));
        static_assert(std::ranges::equal(f(a::code_1, a::code_A), std::array<bool,11>{0,0,0,0,0,1,1,0,1,1,1}));
        static_assert(std::ranges::equal(f(a::code_2, a::code_A), std::array<bool,11>{0,0,0,0,1,1,0,0,1,0,0}));
        static_assert(std::ranges::equal(f(a::code_Z, a::code_B), std::array<bool,11>{1,1,0,0,0,1,1,0,0,1,0}));
    }

    constexpr auto single_alphanumeric_encodes_given_alphanummeric_as_six_bits()
    {
        auto f = [](auto a)
        {
            auto stream = bit_stream{}; 

            single_alphanumeric(stream, a);

            return stream.get();
        };

        using a = alphanumeric;
        static_assert(std::ranges::equal(f(a::code_0), std::array<bool,6>{0,0,0,0,0,0}));
        static_assert(std::ranges::equal(f(a::code_1), std::array<bool,6>{0,0,0,0,0,1}));
        static_assert(std::ranges::equal(f(a::code_2), std::array<bool,6>{0,0,0,0,1,0}));
        static_assert(std::ranges::equal(f(a::code_A), std::array<bool,6>{0,0,1,0,1,0}));
        static_assert(std::ranges::equal(f(a::code_asterisk), std::array<bool,6>{1,0,0,1,1,1}));
        static_assert(std::ranges::equal(f(a::code_percent), std::array<bool,6>{1,0,0,1,1,0}));
        static_assert(std::ranges::equal(f(a::code_dollar), std::array<bool,6>{1,0,0,1,0,1}));
    }
}

namespace qrcode::data::test
{
    constexpr auto encode_alphanumeric_groups_two_alphanumerics_digits_and_converts_them_to_their_bitwise_representation()
    {
        auto f = []
        {
            auto stream = bit_stream{}; 
        
            using a = alphanumeric;
            encode_alphanumerics(
                stream, std::array{a::code_0, a::code_1,a::code_1, a::code_A,a::code_Z, a::code_B}); 
            
            return std::ranges::equal(
                stream.get(), 
                std::array<bool,33>{0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,1,0,1,1,1,1,1,0,0,0,1,1,0,0,1,0});
        };
        static_assert(f());
    }

    constexpr auto encode_alphanumeric_just_appends_the_last_alphanumeric_when_number_of_characters_is_not_a_multiple_of_two()
    {
        auto f = []
        {
            auto stream = bit_stream{}; 
        
            using a = alphanumeric;
            encode_alphanumerics(stream, std::array{a::code_0, a::code_1,a::code_1, a::code_A,a::code_Z}); 
            
            return std::ranges::equal(
                stream.get(), 
                std::array<bool,28>{0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,1,0,1,1,1,1,0,0,0,1,1});
        };
        static_assert(f());
    }

    constexpr auto encode_alphanumeric_ACminus42()
    {
        auto f = []
        {
            auto stream = bit_stream{}; 
        
            using a = alphanumeric;
            encode_alphanumerics(
                stream, std::array{a::code_A, a::code_C,a::code_minus, a::code_4,a::code_2}); 
            
            return std::ranges::equal(
                stream.get(), 
                std::array<bool,28>{0,0,1,1,1,0,0,1,1,1,0,1,1,1,0,0,1,1,1,0,0,1,0,0,0,0,1,0});
        };
        static_assert(f());
    }

    constexpr auto alphanumeric_character_count_returns_the_number_of_characters_which_can_be_encoded_from_given_message() noexcept
    {
        using namespace std::literals;
        static_assert(alphanumeric_character_count(""sv) == 0);
        static_assert(alphanumeric_character_count("A"sv) == 1);
        static_assert(alphanumeric_character_count("AB"sv) == 2);
        static_assert(alphanumeric_character_count("ABC"sv) == 3);
        static_assert(alphanumeric_character_count(std::array{'H','E','L','L','O'}) == 5);
    }

    constexpr auto alphanumeric_bit_count_returns_the_number_of_bits_needed_to_represent_given_message() noexcept
    {
        using namespace std::literals;
        static_assert(alphanumeric_bit_count(""sv) == 0);
        static_assert(alphanumeric_bit_count("A"sv) == 6);
        static_assert(alphanumeric_bit_count("AB"sv) == 11);
        static_assert(alphanumeric_bit_count("ABC"sv) == (11+6));
        static_assert(alphanumeric_bit_count("ABCD"sv) == (11+11));
        static_assert(alphanumeric_bit_count("ABCDE"sv) == (11+11+6));
        static_assert(alphanumeric_bit_count("ABCDEF"sv) == (11+11+11));
    }
}
#endif
