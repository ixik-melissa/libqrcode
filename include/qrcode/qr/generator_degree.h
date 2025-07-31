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

#include <array>

#include <qrcode/qr/symbol_designator.h>

namespace qrcode::qr
{
    [[nodiscard]] constexpr auto generator_degree(symbol_designator const& designator) noexcept
    {
        constexpr auto level_L = std::array{ 
            7, 10, 15, 20, 26, 18, 20, 24, 30, 18, 20, 24, 26, 30, 22, 24, 28, 30, 28, 28, 
            28, 28, 30, 30, 26, 28, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30};
            
        constexpr auto level_M = std::array{ 
            10, 16, 26, 18, 24, 16, 18, 22, 22, 26, 30, 22, 22, 24, 24, 28, 28, 26, 26, 26, 
            26, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28};
            
        constexpr auto level_Q = std::array{ 
            13, 22, 18, 26, 18, 24, 18, 22, 20, 24, 28, 26, 24, 20, 30, 24, 28, 28, 26, 30, 
            28, 30, 30, 30, 30, 28, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30};
            
        constexpr auto level_H = std::array{ 
            17, 28, 22, 16, 22, 28, 26, 26, 24, 28, 24, 28, 22, 24, 24, 30, 28, 28, 26, 28, 
            30, 24, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30};
            
        switch(error_level(designator))
        {
            case error_correction::level_L: return level_L[version(designator).number-1];
            case error_correction::level_M: return level_M[version(designator).number-1];
            case error_correction::level_Q: return level_Q[version(designator).number-1];
            case error_correction::level_H: return level_H[version(designator).number-1];
        }
        return 0;
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::qr::test
{
    constexpr auto generator_degree_returns_degree_of_error_correction_polynomial_depending_on_qr_version_and_error_correction()
    {
        static_assert(generator_degree({symbol_version{1}, error_correction::level_L}) == 7);
        static_assert(generator_degree({symbol_version{12}, error_correction::level_M}) == 22);
        static_assert(generator_degree({symbol_version{25}, error_correction::level_Q}) == 30);
        static_assert(generator_degree({symbol_version{6}, error_correction::level_H}) == 28);
    }
}
#endif
