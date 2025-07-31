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

#include <qrcode/data/optimizer/mode.h>
#include <qrcode/data/numeric.h>
#include <qrcode/data/alphanumeric.h>
#include <qrcode/data/byte.h>
#include <qrcode/data/kanji.h>

namespace qrcode::data::optimizer
{
    [[nodiscard]] constexpr auto determine_mode(char value) noexcept
    {
        using qrcode::data::is_numeric;
        using qrcode::data::is_alphanumeric;
        using qrcode::data::is_potential_kanji;

        if (is_numeric(value))
            return mode::numeric;

        if (is_alphanumeric(value))
            return mode::alphanumeric;

        if (is_potential_kanji(value))
            return mode::kanji;

        return mode::byte;
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>

namespace qrcode::data::optimizer::test
{
    constexpr auto determine_mode_returns_mode_which_fits_given_character_best()
    {
        static_assert(determine_mode('\xAB') == mode::byte);
        static_assert(determine_mode('\x30') == mode::numeric);
        static_assert(determine_mode('\x41') == mode::alphanumeric);
        static_assert(determine_mode('\x81') == mode::kanji);
    }
}
#endif
