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
#include <cassert>

#include <qrcode/data/alphanumeric_encoding.h>
#include <qrcode/data/bit_stream.h>
#include <qrcode/data/indicator.h>

namespace qrcode::data
{
    class alphanumeric_encoder
    {
    public:
        constexpr alphanumeric_encoder(mode_indicator mode_, count_indicator count_) noexcept
        : mode{std::move(mode_)}, count{std::move(count_)}
        {
        }

        template<std::ranges::sized_range Message>
        [[nodiscard]] constexpr auto bit_count(Message&& message) const noexcept
        {
            return mode.bit_count + count.bit_count + alphanumeric_bit_count(message);
        }

        template<std::ranges::forward_range Message>
        requires std::same_as<std::ranges::range_value_t<Message>, alphanumeric>
        [[nodiscard]] constexpr auto write_segment(bit_stream& stream, Message&& message) const noexcept
        {
            using qrcode::data::encode;
            encode(stream, mode);
            encode(stream, count, alphanumeric_character_count(message));
            encode_alphanumerics(stream, std::forward<Message>(message));
        }

        template<std::ranges::forward_range Message>
        requires std::convertible_to<std::ranges::range_value_t<Message>, char>
        [[nodiscard]] constexpr auto write_segment(bit_stream& stream, Message&& message) const noexcept
        {
            using qrcode::data::encode;
            auto alphanumerics = message | std::views::transform([](auto c)
            { 
                auto const alphanumeric = make_alphanumeric(static_cast<char>(c));
                assert(alphanumeric);
                return *alphanumeric;
            });
            encode(stream, mode);
            encode(stream, count, alphanumeric_character_count(message));
            encode_alphanumerics(stream, alphanumerics); 
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
    constexpr auto alphanumeric_encoder_can_encode_given_alphanumeric_string() noexcept
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_mode_indicator = mode_indicator{.value = 0b101, .bit_count = 3};
            auto const any_count_indicator = count_indicator{.bit_count = 5};
            auto stream = bit_stream{};
            
            auto const encoder = alphanumeric_encoder{any_mode_indicator, any_count_indicator};
            encoder.write_segment(stream, "HELLO WORLD"sv);

            return std::ranges::equal(
                stream.get(),
                std::array<bool,69>{
                    1,0,1,  // mode
                    0,1,0,1,1, // count = 11
                    0,1,1,0,0,0,0,1,0,1,1, 0,1,1,1,1,0,0,0,1,1,0, 1,0,0,0,1,0,1,1,1,0,0, 
                    1,0,1,1,0,1,1,1,0,0,0, 1,0,0,1,1,0,1,0,1,0,0, 0,0,1,1,0,1
                }
            );
        };
        static_assert(f());
    }

    constexpr auto alphanumeric_encoder_can_encode_given_alphanumeric_message() noexcept
    {
        auto f = []
        {
            using namespace std::literals;
            using a = alphanumeric;
            auto const any_mode_indicator = mode_indicator{.value = 0b101, .bit_count = 3};
            auto const any_count_indicator = count_indicator{.bit_count = 5};
            auto stream = bit_stream{};
            
            auto const encoder = alphanumeric_encoder{any_mode_indicator, any_count_indicator};
            encoder.write_segment(
                stream, std::array{
                    a::code_H, a::code_E, a::code_L, a::code_L, a::code_O, a::code_space,
                    a::code_W, a::code_O, a::code_R, a::code_L, a::code_D
                });

            return std::ranges::equal(
                stream.get(),
                std::array<bool,69>{
                    1,0,1,  // mode
                    0,1,0,1,1, // count = 11
                    0,1,1,0,0,0,0,1,0,1,1, 0,1,1,1,1,0,0,0,1,1,0, 1,0,0,0,1,0,1,1,1,0,0, 
                    1,0,1,1,0,1,1,1,0,0,0, 1,0,0,1,1,0,1,0,1,0,0, 0,0,1,1,0,1
                }
            );
        };
        static_assert(f());
    }

    constexpr auto alphanumeric_encoder_can_determine_number_of_bits_required_to_encode_given_message() noexcept
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_mode_indicator = mode_indicator{.value = 0b101, .bit_count = 3};
            auto const any_count_indicator = count_indicator{.bit_count = 5};
            
            auto const encoder = alphanumeric_encoder{any_mode_indicator, any_count_indicator};

            return encoder.bit_count("HELLO WORLD"sv) == (3+5+5*11+6);
        };
        static_assert(f());
    }
}
#endif
