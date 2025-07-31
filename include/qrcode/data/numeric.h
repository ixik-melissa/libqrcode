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

#include <algorithm>
#include <optional>
#include <ranges>

namespace qrcode::data
{
    enum class numeric { n0, n1, n2, n3, n4, n5, n6, n7, n8, n9 };

    [[nodiscard]] constexpr auto number(numeric digit) noexcept
    {
        return static_cast<int>(digit);
    }

    [[nodiscard]] constexpr auto is_numeric(char ascii_digit) noexcept
    {
        return ascii_digit >= 0x30 && ascii_digit <= 0x39;
    }

    [[nodiscard]] constexpr auto make_numeric(char ascii_digit) noexcept
    {
        return is_numeric(ascii_digit) 
            ? std::optional<numeric>{static_cast<numeric>(ascii_digit - 0x30)} 
            : std::optional<numeric>{};
    }

    template<std::ranges::forward_range Message>
    requires std::convertible_to<std::ranges::range_value_t<Message>, char>
    [[nodiscard]] constexpr auto all_numerics(Message&& message) noexcept
    {
        return std::ranges::all_of(message, [](auto c){ return is_numeric(c); });
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::data::test
{
    constexpr auto number_returns_the_integer_value_of_given_numeric_digit()
    {
        static_assert(number(numeric::n0) == 0);
        static_assert(number(numeric::n1) == 1);
        static_assert(number(numeric::n2) == 2);
        static_assert(number(numeric::n3) == 3);
        static_assert(number(numeric::n4) == 4);
        static_assert(number(numeric::n5) == 5);
        static_assert(number(numeric::n6) == 6);
        static_assert(number(numeric::n7) == 7);
        static_assert(number(numeric::n8) == 8);
        static_assert(number(numeric::n9) == 9);
    }

    constexpr auto is_numeric_returns_true_if_given_symbol_is_a_digit()
    {
        constexpr auto any_non_digit = 'A';
        static_assert(!is_numeric(any_non_digit));
    }

    constexpr auto is_numeric_returns_false_if_given_symbol_is_not_a_digit()
    {
        constexpr auto any_non_digit = 'A';
        static_assert(!is_numeric(any_non_digit));
    }

    constexpr auto make_numeric_converts_given_ascii_to_integer()
    {
        static_assert(make_numeric('0') == numeric::n0);
        static_assert(make_numeric('1') == numeric::n1);
        static_assert(make_numeric('2') == numeric::n2);
        static_assert(make_numeric('3') == numeric::n3);
        static_assert(make_numeric('4') == numeric::n4);
        static_assert(make_numeric('5') == numeric::n5);
        static_assert(make_numeric('6') == numeric::n6);
        static_assert(make_numeric('7') == numeric::n7);
        static_assert(make_numeric('8') == numeric::n8);
        static_assert(make_numeric('9') == numeric::n9);
    }

    constexpr auto make_numeric_returns_an_error_when_given_character_is_not_a_digit()
    {
        constexpr auto any_non_digit = 'A';

        constexpr auto result = make_numeric(any_non_digit);

        static_assert(!result.has_value());
    }

    constexpr auto all_anumerics_checks_if_given_sequence_of_symbols_can_be_converted_to_numerics()
    {
        using namespace std::literals;
        static_assert(all_numerics("0152"sv));
        static_assert(!all_numerics("0152A"sv));
    }
}
#endif