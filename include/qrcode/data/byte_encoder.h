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

#include <cstddef>
#include <ranges>

#include <qrcode/data/bit_stream.h>
#include <qrcode/data/indicator.h>

namespace qrcode::data::detail
{
    constexpr auto bits_in_byte() noexcept { return 8_bits; }
}

namespace qrcode::data
{
    class byte_encoder
    {
    public:
        constexpr byte_encoder(mode_indicator mode_, count_indicator count_) noexcept
        : mode{std::move(mode_)}, count{std::move(count_)}
        {
        }

        template<std::ranges::sized_range Message>
        [[nodiscard]] constexpr auto bit_count(Message&& message) const noexcept
        {
            using qrcode::data::detail::bits_in_byte;
            auto const byte_count = static_cast<int>(size(message));
            return mode.bit_count + count.bit_count + bits_in_byte().value * byte_count;
        }

        template<std::ranges::forward_range Message>
        requires std::convertible_to<std::ranges::range_value_t<Message>, std::uint8_t> 
            || std::same_as<std::ranges::range_value_t<Message>, std::byte>
        [[nodiscard]] constexpr auto write_segment(bit_stream& stream, Message&& message) const noexcept
        {
            using qrcode::data::detail::bits_in_byte;
            using qrcode::data::encode;
            using std::ranges::size;

            encode(stream, mode);
            encode(stream, count, static_cast<int>(size(message)));

            for (auto i : message)
                stream.add(static_cast<std::uint8_t>(i), bits_in_byte());
        }

    private:
        mode_indicator mode;
        count_indicator count;
    };
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::data::detail::test
{
    constexpr auto byte_encoder_knows_how_many_bits_given_bytes_would_take_to_be_encoded()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_mode_indicator = mode_indicator{.value = 0b11, .bit_count = 2};
            auto const any_count_indicator = count_indicator{.bit_count = 3};
            
            auto const encoder = byte_encoder{any_mode_indicator, any_count_indicator};
            
            return encoder.bit_count("huhu"sv) == (2+3+4*8);
        };
        static_assert(f());
    }

    constexpr auto byte_encoder_encodes_given_bytes()
    {
        auto f = []
        {
            constexpr auto any_message = std::array{std::byte{1}, std::byte{2}};
            auto const any_mode_indicator = mode_indicator{.value = 0b11, .bit_count = 2};
            auto const any_count_indicator = count_indicator{.bit_count = 3};
            auto stream = bit_stream{};
            
            auto const encoder = byte_encoder{any_mode_indicator, any_count_indicator};
            encoder.write_segment(stream, any_message);
            
            return std::ranges::equal(
                stream.get(), 
                std::array<bool, 21>{
                    1,1, // mode
                    0,1,0, // count
                    0,0,0,0,0,0,0,1,  0,0,0,0,0,0,1,0
                }
            );
        };
        static_assert(f());
    }
}
#endif
