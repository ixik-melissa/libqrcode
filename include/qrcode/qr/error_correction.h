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

namespace qrcode::qr
{   
    enum class error_correction
    { 
        level_L = 0b01, level_M = 0b00, level_Q = 0b11, level_H = 0b10
    };

    [[nodiscard]] constexpr auto number(error_correction error_level) noexcept
    {
        return static_cast<int>(error_level);
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::qr::test
{
    constexpr auto number_returns_the_integer_value_of_given_error_level()
    {
        static_assert(number(error_correction::level_L) == 1);
        static_assert(number(error_correction::level_M) == 0);
        static_assert(number(error_correction::level_Q) == 3);
        static_assert(number(error_correction::level_H) == 2);
    }
}
#endif