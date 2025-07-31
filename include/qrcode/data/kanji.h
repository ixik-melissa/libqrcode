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
#include <ranges>

namespace qrcode::data::detail
{
    [[nodiscard]] constexpr auto compress_kanji(std::uint16_t shift_jis) noexcept
    {
        auto const base = shift_jis < 0x9FFCu ? 0x8140u : 0xC140u;
        auto const intermediate = shift_jis - base;
        return static_cast<std::uint16_t>((intermediate >> 8) * 0xC0u + (intermediate & 0xFF));
    }

    [[nodiscard]] constexpr auto double_byte(std::uint8_t leading, std::uint8_t trailing) noexcept
    {
        return static_cast<std::uint16_t>(leading << 8 | trailing);
    }
}

namespace qrcode::data
{
    struct kanji
    {
        std::uint16_t compressed{0};

        [[nodiscard]] constexpr auto operator==(kanji const&) const noexcept -> bool = default;
        [[nodiscard]] constexpr auto operator!=(kanji const&) const noexcept -> bool = default;
    };

    template<class T>
    requires std::convertible_to<T, std::uint8_t> || std::same_as<T, std::byte>
    [[nodiscard]] constexpr auto is_kanji(T leading, T trailing) noexcept
    {
        auto const a = static_cast<std::uint8_t>(leading);
        auto const b = static_cast<std::uint8_t>(trailing);
        auto const trailing_valid = (b >= 0x40u && b <= 0x7Eu) || (b >= 0x80u && b <= 0xFCu);
        if (!trailing_valid)
            return false;

        auto const leading_valid = b > 0xBFu
            ? (a >= 0x81u && a <= 0x9Fu) || (a >= 0xE0u && a <= 0xEAu)
            : (a >= 0x81u && a <= 0x9Fu) || (a >= 0xE0u && a <= 0xEBu);

        return leading_valid;
    }

    template<class T>
    requires std::convertible_to<T, std::uint8_t> || std::same_as<T, std::byte>
    [[nodiscard]] constexpr auto is_potential_kanji(T leading) noexcept
    {
        auto const v = static_cast<std::uint8_t>(leading);
        return (v >= 0x81u && v <= 0x9Fu) || (v >= 0xE0u && v <= 0xEBu);
    }

    template<class T>
    requires std::convertible_to<T, std::uint8_t> || std::same_as<T, std::byte>
    [[nodiscard]] constexpr auto make_kanji(T leading, T trailing) noexcept
    {
        using qrcode::data::detail::double_byte;
        using qrcode::data::detail::compress_kanji;
        return is_kanji(leading, trailing) 
            ? std::optional<kanji>{kanji{compress_kanji(double_byte(leading, trailing))}}
            : std::optional<kanji>{};
    }

    template<std::ranges::forward_range Message>
    requires std::convertible_to<std::ranges::range_value_t<Message>, std::uint8_t>
        || std::same_as<std::ranges::range_value_t<Message>, std::byte>
    [[nodiscard]] constexpr auto all_kanjis(Message&& message) noexcept
    {
        using qrcode::data::detail::double_byte;
        using std::ranges::begin;
        using std::ranges::end;

        for (auto i = begin(message); i != end(message); ++i)
        {
            auto current = *(i++);
            if (i == end(message))
                return false;
            
            if (!is_kanji(current, *i))
                return false;
        }
        return true;
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::data::detail::test
{
    constexpr auto compress_kanji_convert_a_given_shift_jis_character_into_a_compact_13_bit_number()
    {
        static_assert(compress_kanji(0x935Fu) == 0xD9Fu);
        static_assert(compress_kanji(0xE4AAu) == 0x1AAA);
    }

    constexpr auto double_byte_returns_a_16_bit_number_derived_from_given_upper_and_lower_byte()
    {
        static_assert(double_byte(0x00u, 0x00u) == 0x0000u);
        static_assert(double_byte(0x00u, 0x10u) == 0x0010u);
        static_assert(double_byte(0x00u, 0x2Au) == 0x002Au);
        static_assert(double_byte(0xF0u, 0x2Au) == 0xF02Au);
        static_assert(double_byte(0xCBu, 0x2Au) == 0xCB2Au);
    }
}

namespace qrcode::data::test
{
    constexpr auto is_kanji_returns_true_if_given_number_is_shift_jis_range()
    {
        static_assert(!is_kanji(0x00u, 0x00u));
        static_assert(!is_kanji(0x81u, 0x3Fu));
        static_assert(is_kanji(0x81u, 0x40u));
        static_assert(is_kanji(0x9Fu, 0xFCu));
        static_assert(!is_kanji(0x9Fu, 0xFDu));
        static_assert(!is_kanji(0xE0u, 0x3Fu));
        static_assert(is_kanji(0xE0u, 0x40u));
        static_assert(is_kanji(0xEBu, 0xBFu));
        static_assert(!is_kanji(0xEBu, 0xC0u));
    }

    constexpr auto is_potential_kanji_returns_true_if_given_leading_byte_is_in_shift_jis_range()
    {
        static_assert(!is_potential_kanji(0x00u));
        static_assert(!is_potential_kanji(0x80u));
        static_assert(is_potential_kanji(0x81u));
        static_assert(is_potential_kanji(0x9Fu));
        static_assert(!is_potential_kanji(0xA0u));
        static_assert(!is_potential_kanji(0xDFu));
        static_assert(is_potential_kanji(0xE0u));
        static_assert(is_potential_kanji(0xEBu));
        static_assert(!is_potential_kanji(0xECu));

    }

    constexpr auto make_kanji_returns_compressed_kanji_if_given_bytes_are_valid_shift_jis()
    {
        static_assert(make_kanji(0x93u, 0x5Fu) == kanji{0xD9Fu});
        static_assert(make_kanji(0xE4u, 0xAAu) == kanji{0x1AAA});
    }

    constexpr auto make_kanji_returns_nothing_if_given_bytes_are_outside_of_shift_jis_range()
    {
        static_assert(make_kanji(0x23u, 0xACu) == std::nullopt);
        static_assert(make_kanji(0xF4u, 0x1Du) == std::nullopt);
    }

    constexpr auto all_kanjis_return_false_if_size_of_range_is_not_range_of_byte_pairs()
    {
        static_assert(!all_kanjis(std::array{0x93u}));
        static_assert(!all_kanjis(std::array{0x93u, 0x5Fu, 0xE4u}));
    }

    constexpr auto all_kanjis_return_true_if_all_byte_pairs_are_valid_shift_jis_values()
    {
        static_assert(all_kanjis(std::array{0x93u, 0x5Fu}));
        static_assert(all_kanjis(std::array{0x93u, 0x5Fu, 0xE4u, 0xAAu}));
    }
}
#endif