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

#include <concepts>
#include <optional>
#include <ranges>
#include <variant>
#include <qrcode/data/encoders.h>

namespace qrcode::data::best_fit::detail
{
    template<class Variant, class From>
    requires std::copy_constructible<From> 
        // && requires { std::holds_alternative<From>(std::declval<Variant>()); }
    [[nodiscard]] constexpr auto make_variant(From const& from) noexcept
    {
        return Variant{from};
    }

    template<class Variant, class From>
    requires std::copy_constructible<From> 
       // && requires { std::holds_alternative<From>(std::declval<Variant>()); }
    [[nodiscard]] constexpr auto make_variant(std::optional<From> const& from) noexcept
    {
        return from.has_value() ? std::optional{Variant{*from}} : std::optional<Variant>{};
    }
}

namespace qrcode::data::best_fit
{
    template<std::ranges::forward_range Message, bool encodersNullable>
    [[nodiscard]] constexpr auto best_encoder(
        Message&& message, encoders<encodersNullable> const& encoders) noexcept
    {
        using message_value_type = std::ranges::range_value_t<Message>;
        constexpr auto can_numeric = 
            std::convertible_to<message_value_type, char> ||
            std::same_as<message_value_type, numeric>;

        constexpr auto can_alphanumeric = 
            std::convertible_to<message_value_type, char> ||
            std::same_as<message_value_type, alphanumeric>;

        constexpr auto can_kanji = 
            std::convertible_to<message_value_type, std::uint8_t> || 
            std::same_as<message_value_type, std::byte>;

        using encoder = std::variant<
            numeric_encoder, alphanumeric_encoder, byte_encoder, kanji_encoder
        >;
        using qrcode::data::best_fit::detail::make_variant;
        
        if constexpr (can_numeric)
        {
            if (all_numerics(message))
                return make_variant<encoder>(encoders.numeric);
        }

        if constexpr (can_alphanumeric)
        {
            if (all_alphanumerics(message))
                return make_variant<encoder>(encoders.alphanumeric);
        }

        if constexpr (can_kanji)
        {
            if (all_kanjis(message))
                return make_variant<encoder>(encoders.kanji);
        }

        return make_variant<encoder>(encoders.byte);
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>

namespace qrcode::data::best_fit::test
{
    constexpr auto best_encoder_returns_numeric_encoder_given_message_contains_numerics_only()
    {
        auto f = [](auto text)
        {
            auto const some_encoders = nonull_encoders{
                .numeric = numeric_encoder{{.value=0b001,.bit_count=3}, {.bit_count=5}},
                .alphanumeric = alphanumeric_encoder{{.value=0b101,.bit_count=3}, {.bit_count=7}},
                .byte = byte_encoder{{.value=0b111,.bit_count=3}, {.bit_count=3}},
                .kanji = kanji_encoder{{.value=0b100,.bit_count=3}, {.bit_count=4}}
            };

            auto const selected_encoder = best_encoder(text, some_encoders);

            return std::get_if<numeric_encoder>(&selected_encoder);
        };

        using namespace std::literals;
        static_assert(f("31415"sv));
        static_assert(f("123"sv));
    }

    constexpr auto best_encoder_returns_alphanumeric_encoder_given_message_contains_alphanumerics_only()
    {
        auto f = [](auto text)
        {
            auto const some_encoders = nonull_encoders{
                .numeric = numeric_encoder{{.value=0b001,.bit_count=3}, {.bit_count=5}},
                .alphanumeric = alphanumeric_encoder{{.value=0b101,.bit_count=3}, {.bit_count=7}},
                .byte = byte_encoder{{.value=0b111,.bit_count=3}, {.bit_count=3}},
                .kanji = kanji_encoder{{.value=0b100,.bit_count=3}, {.bit_count=4}}
            };

            auto const selected_encoder = best_encoder(text, some_encoders);

            return std::get_if<alphanumeric_encoder>(&selected_encoder);
        };

        using namespace std::literals;
        static_assert(f("011AZB"sv));
        static_assert(f("HELLO WORLD"sv));
    }

    constexpr auto best_encoder_returns_kanji_encoder_given_message_contains_kanjis_only()
    {
        auto f = [](auto text)
        {
            auto const some_encoders = nonull_encoders{
                .numeric = numeric_encoder{{.value=0b001,.bit_count=3}, {.bit_count=5}},
                .alphanumeric = alphanumeric_encoder{{.value=0b101,.bit_count=3}, {.bit_count=7}},
                .byte = byte_encoder{{.value=0b111,.bit_count=3}, {.bit_count=3}},
                .kanji = kanji_encoder{{.value=0b100,.bit_count=3}, {.bit_count=4}}
            };

            auto const selected_encoder = best_encoder(text, some_encoders);

            return std::get_if<kanji_encoder>(&selected_encoder) != nullptr;
        };

        static_assert(f(std::array{0x93u, 0x5Fu, 0xE4u, 0xAAu}));
    }

    constexpr auto best_encoder_returns_byte_encoder_if_given_message_can_not_be_represented_with_any_other_segment_type()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const some_encoders = nonull_encoders{
                .numeric = numeric_encoder{{.value=0b001,.bit_count=3}, {.bit_count=5}},
                .alphanumeric = alphanumeric_encoder{{.value=0b101,.bit_count=3}, {.bit_count=7}},
                .byte = byte_encoder{{.value=0b111,.bit_count=3}, {.bit_count=3}},
                .kanji = kanji_encoder{{.value=0b100,.bit_count=3}, {.bit_count=4}}
            };

            auto const selected_encoder = best_encoder("Some text as bytes"sv, some_encoders);

            return std::get_if<byte_encoder>(&selected_encoder) != nullptr;
        };

        static_assert(f());
    }

    constexpr auto best_encoder_returns_nothing_if_given_message_cannot_be_encoded_with_given_encoders()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const some_encoders = nullable_encoders{
                .numeric = numeric_encoder{{.value=0b001,.bit_count=3}, {.bit_count=5}},
                .alphanumeric = alphanumeric_encoder{{.value=0b101,.bit_count=3}, {.bit_count=7}},
                .byte = std::nullopt,
                .kanji = kanji_encoder{{.value=0b100,.bit_count=3}, {.bit_count=4}}
            };

            auto const selected_encoder = best_encoder("Some text as bytes"sv, some_encoders);

            return !selected_encoder.has_value();
        };
        static_assert(f());
    }
}
#endif
