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
#include <concepts>
#include <optional>
#include <ranges>

namespace qrcode::data
{
    enum class alphanumeric
    {
        code_0, code_1, code_2, code_3, code_4, code_5, code_6, code_7, code_8, code_9,
        code_A, code_B, code_C, code_D, code_E, code_F, code_G, code_H, code_I, code_J,
        code_K, code_L, code_M, code_N, code_O, code_P, code_Q, code_R, code_S, code_T,
        code_U, code_V, code_W, code_X, code_Y, code_Z, code_space , code_dollar, code_percent, 
        code_asterisk, code_plus, code_minus, code_dot, code_slash, code_colon
    };

    [[nodiscard]] constexpr auto number(alphanumeric character) noexcept
    {
        return static_cast<int>(character);
    }

    [[nodiscard]] constexpr auto is_alphanumeric(char symbol) noexcept 
    {
        auto result = false;
        result = result || symbol == 0x20;
        result = result || symbol == 0x24;
        result = result || symbol == 0x25;
        result = result || symbol == 0x2A;
        result = result || symbol == 0x2B;
        result = result || (symbol >= 0x2D && symbol <= 0x3A);
        result = result || (symbol >= 0x41 && symbol <= 0x5A);
        return result;
    }

    [[nodiscard]] constexpr auto make_alphanumeric(char symbol) noexcept 
    {
        constexpr auto set = std::array{
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 
            'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 
            'U', 'V', 'W', 'X', 'Y', 'Z', ' ', '$', '%', '*', 
            '+', '-', '.', '/', ':'
        };

        auto const found = std::ranges::find(set, symbol);

        return found == end(set)
            ? std::optional<alphanumeric>{}
            : std::optional<alphanumeric>{static_cast<alphanumeric>(std::distance(begin(set), found))};
    }

    template<std::ranges::forward_range Message>
    requires std::convertible_to<std::ranges::range_value_t<Message>, char>
    [[nodiscard]] constexpr auto all_alphanumerics(Message&& message) noexcept
    {
        return std::ranges::all_of(message, [](auto c){ return is_alphanumeric(c); });
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>

namespace qrcode::data::test
{
    constexpr auto number_returns_the_integer_value_of_given_alphanumeric_character()
    {
        static_assert(number(alphanumeric::code_0) == 0);
        static_assert(number(alphanumeric::code_1) == 1);
        static_assert(number(alphanumeric::code_2) == 2);
        static_assert(number(alphanumeric::code_3) == 3);
        static_assert(number(alphanumeric::code_4) == 4);
        static_assert(number(alphanumeric::code_5) == 5);
        static_assert(number(alphanumeric::code_6) == 6);
        static_assert(number(alphanumeric::code_7) == 7);
        static_assert(number(alphanumeric::code_8) == 8);
        static_assert(number(alphanumeric::code_9) == 9);
        
        static_assert(number(alphanumeric::code_A) == 10);
        static_assert(number(alphanumeric::code_B) == 11);
        static_assert(number(alphanumeric::code_C) == 12);
        static_assert(number(alphanumeric::code_D) == 13);
        static_assert(number(alphanumeric::code_E) == 14);
        static_assert(number(alphanumeric::code_F) == 15);
        static_assert(number(alphanumeric::code_G) == 16);
        static_assert(number(alphanumeric::code_H) == 17);
        static_assert(number(alphanumeric::code_I) == 18);
        static_assert(number(alphanumeric::code_J) == 19);
        static_assert(number(alphanumeric::code_K) == 20);
        static_assert(number(alphanumeric::code_L) == 21);
        static_assert(number(alphanumeric::code_M) == 22);
        static_assert(number(alphanumeric::code_N) == 23);
        static_assert(number(alphanumeric::code_O) == 24);
        static_assert(number(alphanumeric::code_P) == 25);
        static_assert(number(alphanumeric::code_Q) == 26);
        static_assert(number(alphanumeric::code_R) == 27);
        static_assert(number(alphanumeric::code_S) == 28);
        static_assert(number(alphanumeric::code_T) == 29);
        static_assert(number(alphanumeric::code_U) == 30);
        static_assert(number(alphanumeric::code_V) == 31);
        static_assert(number(alphanumeric::code_W) == 32);
        static_assert(number(alphanumeric::code_X) == 33);
        static_assert(number(alphanumeric::code_Y) == 34);
        static_assert(number(alphanumeric::code_Z) == 35);

        static_assert(number(alphanumeric::code_space) == 36);
        static_assert(number(alphanumeric::code_dollar) == 37);
        static_assert(number(alphanumeric::code_percent) == 38);
        static_assert(number(alphanumeric::code_asterisk) == 39);
        static_assert(number(alphanumeric::code_plus) == 40);
        static_assert(number(alphanumeric::code_minus) == 41);
        static_assert(number(alphanumeric::code_dot) == 42);
        static_assert(number(alphanumeric::code_slash) == 43);
        static_assert(number(alphanumeric::code_colon) == 44);
    }

    constexpr auto is_alphanumeric_returns_true_if_given_symbol_is_a_valid_alphanumeric_symbol()
    {
        static_assert(is_alphanumeric('0'));
        static_assert(is_alphanumeric('1'));
        static_assert(is_alphanumeric('2'));
        static_assert(is_alphanumeric('3'));
        static_assert(is_alphanumeric('4'));
        static_assert(is_alphanumeric('5'));
        static_assert(is_alphanumeric('6'));
        static_assert(is_alphanumeric('7'));
        static_assert(is_alphanumeric('8'));
        static_assert(is_alphanumeric('9'));
        static_assert(is_alphanumeric('A'));
        static_assert(is_alphanumeric('B'));
        static_assert(is_alphanumeric('C'));
        static_assert(is_alphanumeric('D'));
        static_assert(is_alphanumeric('E'));
        static_assert(is_alphanumeric('F'));
        static_assert(is_alphanumeric('G'));
        static_assert(is_alphanumeric('H'));
        static_assert(is_alphanumeric('I'));
        static_assert(is_alphanumeric('J'));
        static_assert(is_alphanumeric('K'));
        static_assert(is_alphanumeric('L'));
        static_assert(is_alphanumeric('M'));
        static_assert(is_alphanumeric('N'));
        static_assert(is_alphanumeric('O'));
        static_assert(is_alphanumeric('P'));
        static_assert(is_alphanumeric('Q'));
        static_assert(is_alphanumeric('R'));
        static_assert(is_alphanumeric('S'));
        static_assert(is_alphanumeric('T'));
        static_assert(is_alphanumeric('U'));
        static_assert(is_alphanumeric('V'));
        static_assert(is_alphanumeric('W'));
        static_assert(is_alphanumeric('X'));
        static_assert(is_alphanumeric('Y'));
        static_assert(is_alphanumeric('Z'));
        static_assert(is_alphanumeric(' '));
        static_assert(is_alphanumeric('$'));
        static_assert(is_alphanumeric('%'));
        static_assert(is_alphanumeric('*'));
        static_assert(is_alphanumeric('+'));
        static_assert(is_alphanumeric('-'));
        static_assert(is_alphanumeric('.'));
        static_assert(is_alphanumeric('/'));
        static_assert(is_alphanumeric(':'));
    }

    constexpr auto is_alphanumeric_returns_false_if_given_symbol_is_an_invalid_symbol()
    {
        constexpr auto any_invalid_symbol = '&';
        static_assert(!is_alphanumeric(any_invalid_symbol));
    }

    constexpr auto alphanumeric_can_be_created_from_valid_alphanumeric_symbols()
    {
        static_assert(make_alphanumeric('0') == alphanumeric::code_0);
        static_assert(make_alphanumeric('1') == alphanumeric::code_1);
        static_assert(make_alphanumeric('2') == alphanumeric::code_2);
        static_assert(make_alphanumeric('3') == alphanumeric::code_3);
        static_assert(make_alphanumeric('4') == alphanumeric::code_4);
        static_assert(make_alphanumeric('5') == alphanumeric::code_5);
        static_assert(make_alphanumeric('6') == alphanumeric::code_6);
        static_assert(make_alphanumeric('7') == alphanumeric::code_7);
        static_assert(make_alphanumeric('8') == alphanumeric::code_8);
        static_assert(make_alphanumeric('9') == alphanumeric::code_9);
        static_assert(make_alphanumeric('A') == alphanumeric::code_A);
        static_assert(make_alphanumeric('B') == alphanumeric::code_B);
        static_assert(make_alphanumeric('C') == alphanumeric::code_C);
        static_assert(make_alphanumeric('D') == alphanumeric::code_D);
        static_assert(make_alphanumeric('E') == alphanumeric::code_E);
        static_assert(make_alphanumeric('F') == alphanumeric::code_F);
        static_assert(make_alphanumeric('G') == alphanumeric::code_G);
        static_assert(make_alphanumeric('H') == alphanumeric::code_H);
        static_assert(make_alphanumeric('I') == alphanumeric::code_I);
        static_assert(make_alphanumeric('J') == alphanumeric::code_J);
        static_assert(make_alphanumeric('K') == alphanumeric::code_K);
        static_assert(make_alphanumeric('L') == alphanumeric::code_L);
        static_assert(make_alphanumeric('M') == alphanumeric::code_M);
        static_assert(make_alphanumeric('N') == alphanumeric::code_N);
        static_assert(make_alphanumeric('O') == alphanumeric::code_O);
        static_assert(make_alphanumeric('P') == alphanumeric::code_P);
        static_assert(make_alphanumeric('Q') == alphanumeric::code_Q);
        static_assert(make_alphanumeric('R') == alphanumeric::code_R);
        static_assert(make_alphanumeric('S') == alphanumeric::code_S);
        static_assert(make_alphanumeric('T') == alphanumeric::code_T);
        static_assert(make_alphanumeric('U') == alphanumeric::code_U);
        static_assert(make_alphanumeric('V') == alphanumeric::code_V);
        static_assert(make_alphanumeric('W') == alphanumeric::code_W);
        static_assert(make_alphanumeric('X') == alphanumeric::code_X);
        static_assert(make_alphanumeric('Y') == alphanumeric::code_Y);
        static_assert(make_alphanumeric('Z') == alphanumeric::code_Z);
        static_assert(make_alphanumeric(' ') == alphanumeric::code_space);
        static_assert(make_alphanumeric('$') == alphanumeric::code_dollar);
        static_assert(make_alphanumeric('%') == alphanumeric::code_percent);
        static_assert(make_alphanumeric('*') == alphanumeric::code_asterisk);
        static_assert(make_alphanumeric('+') == alphanumeric::code_plus);
        static_assert(make_alphanumeric('-') == alphanumeric::code_minus);
        static_assert(make_alphanumeric('.') == alphanumeric::code_dot);
        static_assert(make_alphanumeric('/') == alphanumeric::code_slash);
        static_assert(make_alphanumeric(':') == alphanumeric::code_colon);
    }

    constexpr auto alphanumeric_cannot_be_created_from_invalid_symbols()
    {
        constexpr auto any_invalid_symbol = '&';

        constexpr auto result = make_alphanumeric(any_invalid_symbol);

        static_assert(!result.has_value());
    }

    constexpr auto all_alphanumerics_checks_if_given_sequence_of_symbols_can_be_converted_to_alphanumerics()
    {
        using namespace std::literals;
        static_assert(all_alphanumerics("ABC123"sv));
        static_assert(!all_alphanumerics("ABC&23"sv));
    }
}
#endif
