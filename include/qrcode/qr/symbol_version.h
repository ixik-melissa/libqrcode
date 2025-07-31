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

namespace qrcode::qr
{
    using qrcode::structure::dimension;

    struct symbol_version
    {
        int number;

        [[nodiscard]] constexpr auto operator==(symbol_version const&) const noexcept -> bool = default;
        [[nodiscard]] constexpr auto operator!=(symbol_version const&) const noexcept -> bool = default;
    };

    [[nodiscard]] constexpr auto size(symbol_version version) noexcept
    {
        return dimension{17 + 4 * version.number};
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::qr::test
{
    constexpr auto symbol_size_can_be_determined_from_qr_code_version()
    {
        static_assert(size(symbol_version{1}) == dimension{21,21});
        static_assert(size(symbol_version{2}) == dimension{25,25});
        static_assert(size(symbol_version{3}) == dimension{29,29});
        static_assert(size(symbol_version{6}) == dimension{41,41});
        static_assert(size(symbol_version{7}) == dimension{45,45});
        static_assert(size(symbol_version{14}) == dimension{73,73});
        static_assert(size(symbol_version{21}) == dimension{101,101});
        static_assert(size(symbol_version{40}) == dimension{177,177});
    }
}
#endif