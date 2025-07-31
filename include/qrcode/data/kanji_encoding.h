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
#include <cassert>

#include <qrcode/data/bit_stream.h>
#include <qrcode/data/kanji.h>

namespace qrcode::data::detail
{
    constexpr auto bits_per_kanji() noexcept { return 13_bits; }

    [[nodiscard]] constexpr auto encode(bit_stream& stream, kanji const& kanji) noexcept
    {
        stream.add(kanji.compressed, bits_per_kanji());
    }    
}

namespace qrcode::data
{
    template<std::ranges::forward_range Message>
    requires std::convertible_to<std::ranges::range_value_t<Message>, std::uint8_t>
        || std::same_as<std::ranges::range_value_t<Message>, std::byte>
    [[nodiscard]] constexpr auto encode_kanjis(bit_stream& stream, Message&& shift_jis) noexcept
    {
        using qrcode::data::detail::encode;
        using std::ranges::begin;
        using std::ranges::end;

        for (auto i = begin(shift_jis); i != end(shift_jis); ++i)
        {
            auto current = static_cast<std::uint8_t>(*(i++));
            auto next = static_cast<std::uint8_t>(*i);
            assert(i != end(shift_jis));
            encode(stream, *make_kanji(current, next));
        }  
    }

    template<std::ranges::sized_range Message>
    [[nodiscard]] static constexpr auto kanji_character_count(Message&& message) noexcept
    {
        using std::ranges::size;
        return static_cast<int>(size(message) / 2);
    }

    template<std::ranges::sized_range Message>
    [[nodiscard]] static constexpr auto kanji_bit_count(Message&& message) noexcept
    {
        using qrcode::data::detail::bits_per_kanji;
        return bits_per_kanji().value * kanji_character_count(message);
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::data::detail::test
{
    constexpr auto kanjis_can_be_added_to_a_given_bit_stream_as_13_bit_representation()
    {
        auto f = []
        {
            auto stream = bit_stream{};

            encode(stream, kanji{0xD9Fu});
            
            return std::ranges::equal(stream.get(), std::array<bool,13>{0,1,1,0,1,1,0,0,1,1,1,1,1});
        };
        static_assert(f());
    }

    constexpr auto kanji_bit_count_returns_how_many_bits_given_bytes_would_take_to_be_encoded()
    {
        static_assert(kanji_bit_count(std::array<std::byte,0>{}) == 0);
        static_assert(kanji_bit_count(std::array{0x93u, 0x5Fu}) == 13);
        static_assert(kanji_bit_count(std::array{0x93u, 0x5Fu, 0xE4u, 0xAAu}) == (13+13));
    }

    constexpr auto kanji_character_count_returns_how_characters_given_bytes_represent()
    {
        static_assert(kanji_character_count(std::array<std::byte,0>{}) == 0);
        static_assert(kanji_character_count(std::array{0x93u, 0x5Fu}) == 1);
        static_assert(kanji_character_count(std::array{0x93u, 0x5Fu, 0xE4u, 0xAAu}) == 2);
    }
}

namespace qrcode::data::test
{
    constexpr auto encode_kanjis_converts_a_container_of_shift_jis_characters_into_their_corresponding_bitwise_representation()
    {
        auto f = []
        {
            auto stream = bit_stream{};

            encode_kanjis(stream, std::array{0x93u, 0x5Fu, 0xE4u, 0xAAu});
            
            return std::ranges::equal(
                stream.get(), std::array<bool,26>{0,1,1,0,1,1,0,0,1,1,1,1,1, 1,1,0,1,0,1,0,1,0,1,0,1,0});
        };
        static_assert(f());
    }

    constexpr auto kanji_bit_count_returns_how_many_bits_given_bytes_would_take_to_be_encoded()
    {
        static_assert(kanji_bit_count(std::array<std::byte,0>{}) == 0);
        static_assert(kanji_bit_count(std::array{0x93u, 0x5Fu}) == 13);
        static_assert(kanji_bit_count(std::array{0x93u, 0x5Fu, 0xE4u, 0xAAu}) == (13+13));
    }

    constexpr auto kanji_character_count_returns_how_characters_given_bytes_represent()
    {
        static_assert(kanji_character_count(std::array<std::byte,0>{}) == 0);
        static_assert(kanji_character_count(std::array{0x93u, 0x5Fu}) == 1);
        static_assert(kanji_character_count(std::array{0x93u, 0x5Fu, 0xE4u, 0xAAu}) == 2);
    }

    constexpr auto encode_kanjis_encodes_given_message()
    {
        auto f = []
        {
            constexpr auto any_message = std::array{0x93u, 0x5Fu, 0xE4u, 0xAAu};
            auto stream = bit_stream{};
            
            encode_kanjis(stream, any_message);
            
            return std::ranges::equal(
                stream.get(), 
                std::array<bool,26>{0,1,1,0,1,1,0,0,1,1,1,1,1, 1,1,0,1,0,1,0,1,0,1,0,1,0}
            );
        };
        static_assert(f());
    }
}
#endif