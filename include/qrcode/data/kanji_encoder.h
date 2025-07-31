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
#include <qrcode/data/indicator.h>
#include <qrcode/data/kanji_encoding.h>

namespace qrcode::data
{
    class kanji_encoder
    {
    public:
        constexpr kanji_encoder(mode_indicator mode_, count_indicator count_) noexcept
        : mode{std::move(mode_)}, count{std::move(count_)}
        {
        }

        template<std::ranges::sized_range Message>
        [[nodiscard]] constexpr auto bit_count(Message&& message) const noexcept
        {
            return kanji_bit_count(std::forward<Message>(message));
        }

        template<std::ranges::forward_range Message>
        requires std::convertible_to<std::ranges::range_value_t<Message>, std::uint8_t> 
            || std::same_as<std::ranges::range_value_t<Message>, std::byte>
        [[nodiscard]] constexpr auto write_segment(bit_stream& stream, Message&& message) const noexcept
        {
            using qrcode::data::encode;
            encode(stream, mode);
            encode(stream, count, kanji_character_count(message));
            encode_kanjis(stream, std::forward<Message>(message));
        }

    private:
        mode_indicator mode;
        count_indicator count;
    };
}

#ifdef QRCODE_TESTS_ENABLED

namespace qrcode::data::test
{
    constexpr auto kanji_encoder_encodes_given_message()
    {
        auto f = []
        {
            using namespace std::literals;
            constexpr auto any_message = std::array{0x93u, 0x5Fu, 0xE4u, 0xAAu};
            auto const any_mode_indicator = mode_indicator{.value = 0b101, .bit_count = 3};
            auto const any_count_indicator = count_indicator{.bit_count = 5};
            auto stream = bit_stream{};
            
            auto const encoder = kanji_encoder{any_mode_indicator, any_count_indicator};
            encoder.write_segment(stream, any_message);
            
            return std::ranges::equal(
                stream.get(), 
                std::array<bool,34>{
                    1,0,1, // mode
                    0,0,0,1,0, // count                         
                    0,1,1,0,1,1,0,0,1,1,1,1,1, 1,1,0,1,0,1,0,1,0,1,0,1,0
                }
            );
        };
        static_assert(f());
    }
}
#endif