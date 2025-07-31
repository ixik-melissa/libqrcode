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

#include <qrcode/qr/encoders.h>
#include <qrcode/qr/version_category.h>
#include <qrcode/data/optimizer/data_encoding.h>

namespace qrcode::qr
{
    using qrcode::data::bit_stream;

    template<class Message>
    requires std::convertible_to<std::ranges::range_value_t<Message>, char>
    [[nodiscard]] constexpr auto optimized_encode_data(
        bit_stream& stream, Message&& message, version_category category) noexcept
    {
        using qrcode::data::optimizer::encode_data;
        encode_data(stream, std::forward<Message>(message), available_encoders(category));
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>

namespace qrcode::qr::test
{
    template<class...Ts>
    constexpr auto optimized_encode_data_wrapper(Ts&&...ts)
    {
        auto stream = bit_stream{};
        optimized_encode_data(stream, std::forward<Ts>(ts)...);
        return stream;
    }

    constexpr auto optimized_encode_data_encodes_single_segments()
    {
        using namespace std::literals;

        static_assert(std::ranges::equal(
            optimized_encode_data_wrapper("27182818284"sv, version_category::small).get(), 
            [] 
            {
                auto const encoders = available_encoders(version_category::small);
                auto stream = bit_stream{};
                encoders.numeric.write_segment(stream, "27182818284"sv);
                return stream.get();
            }()
        ));

        static_assert(std::ranges::equal(
            optimized_encode_data_wrapper("HUHU"sv, version_category::medium).get(), 
            [] 
            {
                auto const encoders = available_encoders(version_category::medium);
                auto stream = bit_stream{};
                encoders.alphanumeric.write_segment(stream, "HUHU"sv);
                return stream.get();
            }()
        ));

        static_assert(std::ranges::equal(
            optimized_encode_data_wrapper("some bytes"sv, version_category::large).get(), 
            [] 
            {
                auto const encoders = available_encoders(version_category::large);
                auto stream = bit_stream{};
                encoders.byte.write_segment(stream, "some bytes"sv);
                return stream.get();
            }()
        ));

        static_assert(std::ranges::equal(
            optimized_encode_data_wrapper("\x93\x5F\xE4\xAA"sv, version_category::large).get(), 
            [] 
            {
                auto const encoders = available_encoders(version_category::large);
                auto stream = bit_stream{};
                encoders.kanji.write_segment(stream, "\x93\x5F\xE4\xAA"sv);
                return stream.get();
            }()
        ));
    }

    constexpr auto optimized_encode_data_encodes_two_segments()
    {
        using namespace std::literals;

        static_assert(std::ranges::equal(
            optimized_encode_data_wrapper("9876543210-AC-42"sv, version_category::small).get(), 
            [] 
            {
                auto const encoders = available_encoders(version_category::small);
                auto stream = bit_stream{};
                encoders.numeric.write_segment(stream, "9876543210"sv);
                encoders.alphanumeric.write_segment(stream, "-AC-42"sv);
                return stream.get();
            }()
        ));

        static_assert(std::ranges::equal(
            optimized_encode_data_wrapper("\x93\x5F\xE4\xAA\x93\x5F\xE4\xAA\x93\x5F\xE4\xAAHELLO-WORLD"sv, version_category::medium).get(),
            [] 
            {
                auto const encoders = available_encoders(version_category::medium);
                auto stream = bit_stream{};
                encoders.kanji.write_segment(stream, "\x93\x5F\xE4\xAA\x93\x5F\xE4\xAA\x93\x5F\xE4\xAA"sv);
                encoders.alphanumeric.write_segment(stream, "HELLO-WORLD"sv);
                return stream.get();
            }()
        ));
    }

    constexpr auto optimized_encode_data_encodes_three_segments()
    {
        using namespace std::literals;

        static_assert(std::ranges::equal(
            optimized_encode_data_wrapper("https://not.existing.org/3141592635.html?name=foo"sv, version_category::large).get(),
            [] 
            {
                auto const encoders = available_encoders(version_category::large);
                auto stream = bit_stream{};
                encoders.byte.write_segment(stream, "https://not.existing.org"sv);
                encoders.alphanumeric.write_segment(stream, "/3141592635."sv);
                encoders.byte.write_segment(stream, "html?name=foo"sv);
                return stream.get();
            }()
        ));
    }

    constexpr auto optimized_encode_data_encodes_four_segments()
    {
        using namespace std::literals;

        static_assert(std::ranges::equal(
            optimized_encode_data_wrapper("JAPANESE KANJI \x93\x5F\xE4\xAA\x93\x5F\xE4\xAA\x93\x5F\xE4\xAA and some numbers#111222333444"sv, version_category::large).get(),
            [] 
            {
                auto const encoders = available_encoders(version_category::large);
                auto stream = bit_stream{};
                encoders.alphanumeric.write_segment(stream, "JAPANESE KANJI "sv);
                encoders.kanji.write_segment(stream, "\x93\x5F\xE4\xAA\x93\x5F\xE4\xAA\x93\x5F\xE4\xAA"sv);
                encoders.byte.write_segment(stream, " and some numbers#"sv);
                encoders.numeric.write_segment(stream, "111222333444"sv);
                return stream.get();
            }()
        ));
    }
}
#endif
