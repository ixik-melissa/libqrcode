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

#include <qrcode/qr/version_category.h>

#include <qrcode/data/encoders.h>
#include <qrcode/data/indicator.h>

namespace qrcode::qr::detail
{
    using qrcode::data::mode_indicator;
    using qrcode::data::count_indicator;
    using qrcode::data::numeric_encoder;
    using qrcode::data::alphanumeric_encoder;
    using qrcode::data::byte_encoder;
    using qrcode::data::kanji_encoder;

    [[nodiscard]] constexpr auto make_numeric_encoder(version_category category) noexcept
    {
        using qrcode::data::numeric_encoder;
        constexpr auto count_sizes = std::array{10, 12, 14};
        return numeric_encoder{
            mode_indicator{.value=0b0001,.bit_count=4}, 
            count_indicator{.bit_count=count_sizes[number(category)]}
        };
    }

    [[nodiscard]] constexpr auto make_alphanumeric_encoder(version_category category) noexcept
    {
        constexpr auto count_sizes = std::array{9, 11, 13};
        return alphanumeric_encoder{
            mode_indicator{.value=0b0010,.bit_count=4}, 
            count_indicator{.bit_count=count_sizes[number(category)]}
        };
    }

    [[nodiscard]] constexpr auto make_byte_encoder(version_category category) noexcept
    {
        constexpr auto count_sizes = std::array{8, 16, 16};
        return byte_encoder{
            mode_indicator{.value=0b0100,.bit_count=4}, 
            count_indicator{.bit_count=count_sizes[number(category)]}
        };
    }

    [[nodiscard]] constexpr auto make_kanji_encoder(version_category category) noexcept
    {
        constexpr auto count_sizes = std::array{8, 10, 12};
        return kanji_encoder{
            mode_indicator{.value=0b1000,.bit_count=4}, 
            count_indicator{.bit_count=count_sizes[number(category)]}
        };
    }
}

namespace qrcode::qr
{
    using qrcode::data::nonull_encoders;

    [[nodiscard]] constexpr auto available_encoders(version_category category) noexcept
    {
        using qrcode::qr::detail::make_numeric_encoder;
        using qrcode::qr::detail::make_alphanumeric_encoder;
        using qrcode::qr::detail::make_byte_encoder;
        using qrcode::qr::detail::make_kanji_encoder;

        return nonull_encoders{
            .numeric = make_numeric_encoder(category),
            .alphanumeric = make_alphanumeric_encoder(category),
            .byte = make_byte_encoder(category),
            .kanji = make_kanji_encoder(category)
        };
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>
#include <qrcode/data/bit_stream.h>

namespace qrcode::qr::detail::test
{
    constexpr auto make_numeric_encoder_returns_encoder_which_encodes_mode_count_and_numeric_message()
    {
        auto f = []
        {
            using namespace std::literals;
            using qrcode::data::bit_stream;
            constexpr auto any_version_category = version_category::medium;
            constexpr auto any_message = "012"sv;
            auto stream = bit_stream{};
            
            auto const encoder = make_numeric_encoder(any_version_category);
            encoder.write_segment(stream, any_message);

            return std::ranges::equal(
                stream.get(),
                std::array<bool,26>{
                    0,0,0,1,                    // mode: numeric
                    0,0,0,0,0,0,0,0,0,0,1,1,    // count: 3
                    0,0,0,0,0,0,1,1,0,0         // data
                }
            );
        };
        static_assert(f());
    }

    constexpr auto make_alphanumeric_encoder_returns_encoder_which_encodes_mode_count_and_alphanumeric_message()
    {
        auto f = []
        {
            using namespace std::literals;
            using qrcode::data::bit_stream;
            constexpr auto any_version_category = version_category::medium;
            constexpr auto any_message = "AC-42"sv;
            auto stream = bit_stream{};
            
            auto const encoder = make_alphanumeric_encoder(any_version_category);
            encoder.write_segment(stream, any_message);

            return std::ranges::equal(
                stream.get(),
                std::array<bool,43>{
                    0,0,1,0,                                                   // mode: alphanumeric
                    0,0,0,0,0,0,0,0,1,0,1,                                     // count: 5
                    0,0,1,1,1,0,0,1,1,1,0, 1,1,1,0,0,1,1,1,0,0,1, 0,0,0,0,1,0  // data
                }
            );
        };
        static_assert(f());
    }

    constexpr auto make_byte_encoder_returns_encoder_which_encodes_mode_count_and_byte_message()
    {
        auto f = []
        {
            using namespace std::literals;
            using qrcode::data::bit_stream;
            constexpr auto any_version_category = version_category::medium;
            constexpr auto any_message = "\xF1"sv;
            auto stream = bit_stream{};
            
            auto const encoder = make_byte_encoder(any_version_category);
            encoder.write_segment(stream, any_message);

            return std::ranges::equal(
                stream.get(),
                std::array<bool,28>{
                    0,1,0,0,                          // mode: byte
                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,  // count: 1
                    1,1,1,1,0,0,0,1                   // data
                }
            );
        };
        static_assert(f());
    }

    constexpr auto make_kanji_encoder_returns_encoder_which_encodes_mode_count_and_kanji_message()
    {
        auto f = []
        {
            using namespace std::literals;
            using qrcode::data::bit_stream;
            constexpr auto any_version_category = version_category::medium;
            constexpr auto any_message = "\x93\x5F\xE4\xAA"sv;
            auto stream = bit_stream{};
            
            auto const encoder = make_kanji_encoder(any_version_category);
            encoder.write_segment(stream, any_message);

            return std::ranges::equal(
                stream.get(),
                std::array<bool,40>{
                    1,0,0,0,                                              // mode: kanji
                    0,0,0,0,0,0,0,0,1,0,                                  // count: 2
                    0,1,1,0,1,1,0,0,1,1,1,1,1, 1,1,0,1,0,1,0,1,0,1,0,1,0  // data
                }
            );
        };
        static_assert(f());
    }
}
#endif
