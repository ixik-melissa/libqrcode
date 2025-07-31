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

#include <optional>

#include <qrcode/micro_qr/symbol_version.h>

#include <qrcode/data/encoders.h>
#include <qrcode/data/indicator.h>

namespace qrcode::micro_qr::detail
{
    using qrcode::data::mode_indicator;
    using qrcode::data::count_indicator;
    using qrcode::data::numeric_encoder;
    using qrcode::data::alphanumeric_encoder;
    using qrcode::data::byte_encoder;
    using qrcode::data::kanji_encoder;
    
    [[nodiscard]] constexpr auto make_numeric_encoder(symbol_version version) noexcept
    {
        return numeric_encoder{
            mode_indicator{.value=0b000,.bit_count=number(version)}, 
            count_indicator{.bit_count=3+number(version)}
        };
    }

    [[nodiscard]] constexpr auto make_alphanumeric_encoder(symbol_version version) noexcept
    {
        return version > symbol_version::M1 ? std::optional{alphanumeric_encoder{
            mode_indicator{.value=0b001,.bit_count=number(version)}, 
            count_indicator{.bit_count=2+number(version)}
        }} : std::optional<alphanumeric_encoder>{};
    }

    [[nodiscard]] constexpr auto make_byte_encoder(symbol_version version) noexcept
    {
        return version > symbol_version::M2 ? std::optional{byte_encoder{
            mode_indicator{.value=0b010,.bit_count=number(version)}, 
            count_indicator{.bit_count=2+number(version)}
        }} : std::optional<byte_encoder>{};
    }

    [[nodiscard]] constexpr auto make_kanji_encoder(symbol_version version) noexcept
    {
        return version > symbol_version::M2 ? std::optional{kanji_encoder{
            mode_indicator{.value=0b011,.bit_count=number(version)}, 
            count_indicator{.bit_count=1+number(version)}
        }} : std::optional<kanji_encoder>{};
    }
}

namespace qrcode::micro_qr
{
    using qrcode::data::nullable_encoders;

    [[nodiscard]] constexpr auto available_encoders(symbol_version version)
    {
        using qrcode::micro_qr::detail::make_numeric_encoder;
        using qrcode::micro_qr::detail::make_alphanumeric_encoder;
        using qrcode::micro_qr::detail::make_byte_encoder;
        using qrcode::micro_qr::detail::make_kanji_encoder;

        return nullable_encoders{
            .numeric = make_numeric_encoder(version),
            .alphanumeric = make_alphanumeric_encoder(version),
            .byte = make_byte_encoder(version),
            .kanji = make_kanji_encoder(version)
        };
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>
#include <qrcode/data/bit_stream.h>

namespace qrcode::micro_qr::detail::test
{
    constexpr auto make_numeric_encoder_returns_encoder_which_encodes_mode_count_and_numeric_message()
    {
        auto f = []
        {
            using namespace std::literals;
            using qrcode::data::bit_stream;
            constexpr auto any_version = symbol_version::M1;
            constexpr auto any_message = "012"sv;
            auto stream = bit_stream{};
            
            auto const encoder = make_numeric_encoder(any_version);
            encoder.write_segment(stream, any_message);

            return std::ranges::equal(
                stream.get(),
                std::array<bool,13>{
                    0,1,1,                      // no mode in M1, count: 3
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
            constexpr auto any_version = symbol_version::M2;
            constexpr auto any_message = "AC-42"sv;
            auto stream = bit_stream{};
            
            auto const encoder = make_alphanumeric_encoder(any_version);
            encoder->write_segment(stream, any_message);

            return std::ranges::equal(
                stream.get(),
                std::array<bool,32>{
                    1,                                                         // mode: alphanumeric
                    1,0,1,                                                     // count: 5
                    0,0,1,1,1,0,0,1,1,1,0, 1,1,1,0,0,1,1,1,0,0,1, 0,0,0,0,1,0  // data
                }
            );
        };
        static_assert(f());
    }

    constexpr auto make_alphanumeric_encoder_returns_nothing_if_version_is_below_M2()
    {
        static_assert(make_alphanumeric_encoder(symbol_version::M1) == std::nullopt);
    }

    constexpr auto make_byte_encoder_returns_encoder_which_encodes_mode_count_and_byte_message()
    {
        auto f = []
        {
            using namespace std::literals;
            using qrcode::data::bit_stream;
            constexpr auto any_version = symbol_version::M3;
            constexpr auto any_message = "\xF1"sv;
            auto stream = bit_stream{};
            
            auto const encoder = make_byte_encoder(any_version);
            encoder->write_segment(stream, any_message);

            return std::ranges::equal(
                stream.get(),
                std::array<bool,14>{
                    1,0,                              // mode: byte
                    0,0,0,1,                          // count: 1
                    1,1,1,1,0,0,0,1                   // data
                }
            );
        };
        static_assert(f());
    }

    constexpr auto make_byte_encoder_returns_nothing_if_version_is_below_M3()
    {
        static_assert(make_byte_encoder(symbol_version::M1) == std::nullopt);
        static_assert(make_byte_encoder(symbol_version::M2) == std::nullopt);
    }

    constexpr auto make_kanji_encoder_returns_encoder_which_encodes_mode_count_and_kanji_message()
    {
        auto f = []
        {
            using namespace std::literals;
            using qrcode::data::bit_stream;
            constexpr auto any_version = symbol_version::M4;
            constexpr auto any_message = "\x93\x5F\xE4\xAA"sv;
            auto stream = bit_stream{};
            
            auto const encoder = make_kanji_encoder(any_version);
            encoder->write_segment(stream, any_message);

            return std::ranges::equal(
                stream.get(),
                std::array<bool,33>{
                    0,1,1,                                                // mode: kanji
                    0,0,1,0,                                              // count: 2
                    0,1,1,0,1,1,0,0,1,1,1,1,1, 1,1,0,1,0,1,0,1,0,1,0,1,0  // data
                }
            );
        };
        static_assert(f());
    }

    constexpr auto make_kanji_encoder_returns_nothing_if_version_is_below_M3()
    {
        static_assert(make_kanji_encoder(symbol_version::M1) == std::nullopt);
        static_assert(make_kanji_encoder(symbol_version::M2) == std::nullopt);
    }
}
#endif
