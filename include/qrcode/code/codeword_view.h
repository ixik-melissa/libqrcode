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

#include <algorithm>
#include <ranges>

#include <qrcode/code/code_block.h>
#include <qrcode/code/extended_remainder.h>
#include <qrcode/code/polynomial.h>
#include <qrcode/code/gf2p8.h>

namespace qrcode::code::views
{
    template<class T>
    [[nodiscard]] constexpr auto codeword(polynomial<T> error_generator) noexcept
    {
        return std::views::transform([=](auto const& data_polynomial)
        {
            auto const error_polynomial = extended_remainder(data_polynomial, error_generator);
            return code_block<gf2p8>{.data = data_polynomial, .error = error_polynomial};
        });
    }
}

namespace qrcode::views
{
    using qrcode::code::views::codeword;
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::code::views::test
{
    constexpr auto codeword_view_calculates_code_block_consising_of_data_polynomial_and_error_correction_polynomial()
    {
        auto f = []
        {
            auto any_data = std::array{polynomial{3_gf, 5_gf, 1_gf}};

            auto const range = any_data | codeword(polynomial{0x02_gf, 0x03_gf, 0x01_gf});

            return std::ranges::equal(
                range, 
                std::array{code_block{polynomial{3_gf, 5_gf, 1_gf}, polynomial{22_gf, 17_gf}}}
            );
        };
        static_assert(f());
    }
}
#endif
