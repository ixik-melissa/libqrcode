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

#include <cstdint>

namespace qrcode::data
{
    [[nodiscard]] constexpr auto is_byte(char c) noexcept
    {
        auto const value = static_cast<std::uint8_t>(c);

        bool result = false;
        result = result || (value <= 0x1F);
        result = result || (value >= 0x21 && value <= 0x23);
        result = result || (value >= 0x26 && value <= 0x29);
        result = result || (value == 0x2C);
        result = result || (value >= 0x3B && value <= 0x40);
        result = result || (value >= 0x5B && value <= 0x7F);
        result = result || (value >= 0xA0 && value <= 0xDF);
        return result;
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::data
{
    constexpr auto is_byte_returns_false_if_given_character_is_part_of_numeric_alphanumeric_or_kanji()
    {
        static_assert(!is_byte('0'));
        static_assert(!is_byte('1'));
        static_assert(!is_byte('2'));
        static_assert(!is_byte('3'));
        static_assert(!is_byte('4'));
        static_assert(!is_byte('5'));
        static_assert(!is_byte('6'));
        static_assert(!is_byte('7'));
        static_assert(!is_byte('8'));
        static_assert(!is_byte('9'));

        static_assert(!is_byte('A'));
        static_assert(!is_byte('B'));
        static_assert(!is_byte('C'));
        static_assert(!is_byte('D'));
        static_assert(!is_byte('E'));
        static_assert(!is_byte('F'));
        static_assert(!is_byte('G'));
        static_assert(!is_byte('H'));
        static_assert(!is_byte('I'));
        static_assert(!is_byte('J'));
        static_assert(!is_byte('K'));
        static_assert(!is_byte('L'));
        static_assert(!is_byte('M'));
        static_assert(!is_byte('N'));
        static_assert(!is_byte('O'));
        static_assert(!is_byte('P'));
        static_assert(!is_byte('Q'));
        static_assert(!is_byte('R'));
        static_assert(!is_byte('S'));
        static_assert(!is_byte('T'));
        static_assert(!is_byte('U'));
        static_assert(!is_byte('V'));
        static_assert(!is_byte('W'));
        static_assert(!is_byte('X'));
        static_assert(!is_byte('Y'));
        static_assert(!is_byte('Z'));
        static_assert(!is_byte(' '));
        static_assert(!is_byte('$'));
        static_assert(!is_byte('%'));
        static_assert(!is_byte('*'));
        static_assert(!is_byte('+'));
        static_assert(!is_byte('-'));
        static_assert(!is_byte('.'));
        static_assert(!is_byte('/'));
        static_assert(!is_byte(':'));

        static_assert(!is_byte('\x80'));
        static_assert(!is_byte('\xE0'));
    }

    constexpr auto is_byte_returns_true_if_given_character_is_not_part_of_numeric_alphanumeric_or_kanji()
    {
        static_assert(is_byte('\x00'));
        static_assert(is_byte('\x1F'));

        static_assert(is_byte('\x21'));
        static_assert(is_byte('\x23'));

        static_assert(is_byte('\x26'));
        static_assert(is_byte('\x29'));

        static_assert(is_byte('\x3B'));
        static_assert(is_byte('\x40'));

        static_assert(is_byte('\x5B'));
        static_assert(is_byte('\x7F'));

        static_assert(is_byte('\xA0'));
        static_assert(is_byte('\xDF'));
    }
}
#endif
