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
#include <qrcode/data/numeric_encoding.h>

namespace qrcode::data
{
    class numeric_encoder
    {
    public:
        constexpr numeric_encoder(mode_indicator mode_, count_indicator count_) noexcept
        : mode{std::move(mode_)}, count{std::move(count_)}
        {
        }

        template<std::ranges::sized_range Message>
        [[nodiscard]] constexpr auto bit_count(Message&& message) const noexcept
        {
            return mode.bit_count + count.bit_count + numeric_bit_count(message);
        }

        template<std::ranges::forward_range Message>
        requires std::same_as<std::ranges::range_value_t<Message>, numeric>
        [[nodiscard]] constexpr auto write_segment(bit_stream& stream, Message&& message) const noexcept
        {
            using qrcode::data::encode;
            encode(stream, mode);
            encode(stream, count, numeric_character_count(message));
            encode_numerics(stream, std::forward<Message>(message));
        }

        template<std::ranges::forward_range Message>
        requires std::convertible_to<std::ranges::range_value_t<Message>, char>
        [[nodiscard]] constexpr auto write_segment(bit_stream& stream, Message&& message) const noexcept
        {
            using qrcode::data::encode;
            auto numerics = message | std::views::transform([](auto c)
            { 
                auto const numeric = make_numeric(static_cast<char>(c));
                assert(numeric);
                return *numeric;
            });
            encode(stream, mode);
            encode(stream, count, numeric_character_count(message));
            encode_numerics(stream, numerics); 
        }

    private:
        mode_indicator mode;
        count_indicator count;
    };
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>

namespace qrcode::data::test
{
    constexpr auto numeric_encoder_knows_how_many_bits_given_message_would_take_to_be_encoded()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_mode_indicator = mode_indicator{.value = 0b0101, .bit_count = 4};
            auto const any_count_indicator = count_indicator{.bit_count = 5};

            auto const encoder = numeric_encoder{any_mode_indicator, any_count_indicator};
            return encoder.bit_count("01234567"sv) == (4+5+10+10+7);
        };
        static_assert(f());
    }

    constexpr auto numeric_encoder_can_encode_given_numeric_string()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_mode_indicator = mode_indicator{.value = 0b0101, .bit_count = 4};
            auto const any_count_indicator = count_indicator{.bit_count = 5};
            auto stream = bit_stream{};
            
            auto const encoder = numeric_encoder{any_mode_indicator, any_count_indicator};
            encoder.write_segment(stream, "01234567"sv);

            return std::ranges::equal(
                stream.get(),
                std::array<bool,36>{
                    0,1,0,1, // mode
                    0,1,0,0,0, // count
                    0,0,0,0,0,0,1,1,0,0, 0,1,0,1,0,1,1,0,0,1, 1,0,0,0,0,1,1
                }
            );
        };
        static_assert(f());
    }

    constexpr auto numeric_encoder_can_encode_given_numeric_message()
    {
        auto f = []
        {
            using n = numeric;
            auto const any_mode_indicator = mode_indicator{.value = 0b0101, .bit_count = 4};
            auto const any_count_indicator = count_indicator{.bit_count = 5};
            auto stream = bit_stream{};
            
            auto const encoder = numeric_encoder{any_mode_indicator, any_count_indicator};
            encoder.write_segment(
                stream, 
                std::array{
                    n::n0, n::n1, n::n2, n::n3, n::n4, n::n5, n::n6, n::n7, n::n8, n::n9,
                    n::n0, n::n1, n::n2, n::n3, n::n4, n::n5
                }
            );

            return std::ranges::equal(
                stream.get(),
                std::array<bool,63>{
                    0,1,0,1, // mode
                    1,0,0,0,0, // count = 16
                    0,0,0,0,0,0,1,1,0,0, 0,1,0,1,0,1,1,0,0,1, 1,0,1,0,1,0,0,1,1,0, 
                    1,1,1,0,0,0,0,1,0,1, 0,0,1,1,1,0,1,0,1,0, 0,1,0,1
                }
            );
        };
        static_assert(f());
    }
}
#endif
