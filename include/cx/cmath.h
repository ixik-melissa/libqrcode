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

#include <cstdint>

// this header is obsolete when standard library is more constexpr
namespace cx
{   
    // constexpr version of std::ceil
    [[nodiscard]] constexpr auto ceil(float value) noexcept
    {
        auto const floored = static_cast<std::int64_t>(-value);
        auto f = static_cast<float>(-floored);
        return (value > 0) ? f+1 : f;
    }

    // constexpr version of std::abs
    template<class T>
    [[nodiscard]] constexpr auto abs(T value) noexcept
    {
        return value >= 0 ? value : -value;
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace cx::test
{
    constexpr auto cx_ceil_is_the_constexpr_version_of_std_ceil()
    {
        static_assert(cx::ceil(1.2f) == 2.f);
        static_assert(cx::ceil(-7.4f) == -7.f);
    }

    constexpr auto cx_abs_is_the_constexpr_version_of_std_abs()
    {
        static_assert(cx::abs(1.2f) == 1.2f);
        static_assert(cx::abs(-7.4f) == 7.4f);
    }
}
#endif
