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

#include <qrcode/structure/dimension.h>

namespace qrcode::micro_qr
{
    using qrcode::structure::dimension;

    enum class symbol_version
    {
        M1, M2, M3, M4
    };

    [[nodiscard]] constexpr auto versions() noexcept 
    {
        return std::array{
            symbol_version::M1, symbol_version::M2, symbol_version::M3, symbol_version::M4};
    }

    [[nodiscard]] constexpr auto number(symbol_version const& version) noexcept 
    {
        return static_cast<int>(version);
    }

    [[nodiscard]] constexpr auto size(symbol_version const& version) noexcept 
    {
        return dimension{11 + 2 * number(version)};
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::micro_qr::test
{
    constexpr auto number_converts_given_symbol_version_into_index()
    {
        static_assert(number(symbol_version::M1) == 0);
        static_assert(number(symbol_version::M2) == 1);
        static_assert(number(symbol_version::M3) == 2);
        static_assert(number(symbol_version::M4) == 3);
    }

    constexpr auto symbol_size_can_be_determined_from_micro_qr_code_symbol_version()
    {
        static_assert(size(symbol_version::M1) == dimension{11,11});
        static_assert(size(symbol_version::M2) == dimension{13,13});
        static_assert(size(symbol_version::M3) == dimension{15,15});
        static_assert(size(symbol_version::M4) == dimension{17,17});
    }
}
#endif