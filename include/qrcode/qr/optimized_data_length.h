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

#include <qrcode/qr/encoders.h>
#include <qrcode/qr/version_category.h>
#include <qrcode/data/optimizer/data_length.h>

namespace qrcode::qr
{
    template<class Message>
    requires std::convertible_to<std::ranges::range_value_t<Message>, char>
    [[nodiscard]] constexpr auto optimized_data_length(
        Message&& message, version_category category) noexcept
    {
        using qrcode::data::optimizer::data_length;
        return data_length(std::forward<Message>(message), available_encoders(category)); 
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>

namespace qrcode::qr::test
{
    constexpr auto optimized_data_length_returns_the_total_length_of_single_segments()
    {
        using namespace std::literals;

        static_assert(
            optimized_data_length("27182818284"sv, version_category::small) ==
            [] 
            {
                auto const encoders = available_encoders(version_category::small);
                return encoders.numeric.bit_count("27182818284"sv);
            }()
        );

        static_assert(
            optimized_data_length("HUHU"sv, version_category::medium) ==
            [] 
            {
                auto const encoders = available_encoders(version_category::medium);
                return encoders.alphanumeric.bit_count("HUHU"sv);
            }()
        );

        static_assert(
            optimized_data_length("some bytes"sv, version_category::large) ==
            [] 
            {
                auto const encoders = available_encoders(version_category::large);
                return encoders.byte.bit_count("some bytes"sv);
            }()
        );

        static_assert(
            optimized_data_length("\x93\x5F\xE4\xAA"sv, version_category::large) ==
            [] 
            {
                auto const encoders = available_encoders(version_category::large);
                return encoders.kanji.bit_count("\x93\x5F\xE4\xAA"sv);
            }()
        );
    }

    constexpr auto optimized_data_length_returns_the_total_length_of_two_segments()
    {
        using namespace std::literals;

        static_assert(
            optimized_data_length("9876543210-AC-42"sv, version_category::small) ==
            [] 
            {
                auto const encoders = available_encoders(version_category::small);
                return
                    encoders.numeric.bit_count("9876543210"sv) +
                    encoders.alphanumeric.bit_count("-AC-42"sv);
            }()
        );

        static_assert(
            optimized_data_length("\x93\x5F\xE4\xAA\x93\x5F\xE4\xAA\x93\x5F\xE4\xAAHELLO-WORLD"sv, version_category::medium) ==
            [] 
            {
                auto const encoders = available_encoders(version_category::medium);
                return 
                    encoders.kanji.bit_count("\x93\x5F\xE4\xAA\x93\x5F\xE4\xAA\x93\x5F\xE4\xAA"sv) +
                    encoders.alphanumeric.bit_count("HELLO-WORLD"sv);
            }()
        );
    }

    constexpr auto optimized_data_length_returns_the_total_length_of_three_segments()
    {
        using namespace std::literals;

        static_assert(
            optimized_data_length("https://not.existing.org/3141592635.html?name=foo"sv, version_category::large) ==
            [] 
            {
                auto const encoders = available_encoders(version_category::large);
                return
                    encoders.byte.bit_count("https://not.existing.org"sv) +
                    encoders.alphanumeric.bit_count("/3141592635."sv) +
                    encoders.byte.bit_count("html?name=foo"sv);
            }()
        );
    }

    constexpr auto optimized_data_length_returns_the_total_length_of_four_segments()
    {
        using namespace std::literals;

        static_assert(
            optimized_data_length("JAPANESE KANJI \x93\x5F\xE4\xAA\x93\x5F\xE4\xAA\x93\x5F\xE4\xAA and some numbers#111222333444"sv, version_category::large) ==
            [] 
            {
                auto const encoders = available_encoders(version_category::large);
                return
                    encoders.alphanumeric.bit_count("JAPANESE KANJI "sv) +
                    encoders.kanji.bit_count("\x93\x5F\xE4\xAA\x93\x5F\xE4\xAA\x93\x5F\xE4\xAA"sv) +
                    encoders.byte.bit_count(" and some numbers#"sv) +
                    encoders.numeric.bit_count("111222333444"sv);
            }()
        );
    }
}
#endif
