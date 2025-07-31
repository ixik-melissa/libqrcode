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

#include <array>

#include <qrcode/qr/symbol_designator.h>

namespace qrcode::qr
{
    [[nodiscard]] constexpr auto total_blocks(symbol_designator const& designator) noexcept
    {
        constexpr auto level_L = std::array{ 
            1, 1, 1, 1, 1, 2, 2, 2, 2, 4,  4,  4,  4,  4,  6,  6,  6,  6,  7,  8,  8,  
            9,  9, 10, 12, 12, 12, 13, 14, 15, 16, 17, 18, 19, 19, 20, 21, 22, 24, 25
        };
            
        constexpr auto level_M = std::array{ 
            1, 1, 1, 2, 2, 4, 4, 4, 5, 5,  5,  8,  9,  9, 10, 10, 11, 13, 14, 16, 17, 
            17, 18, 20, 21, 23, 25, 26, 28, 29, 31, 33, 35, 37, 38, 40, 43, 45, 47, 49
        };
            
        constexpr auto level_Q = std::array{ 
            1, 1, 2, 2, 4, 4, 6, 6, 8, 8,  8, 10, 12, 16, 12, 17, 16, 18, 21, 20, 23, 
            23, 25, 27, 29, 34, 34, 35, 38, 40, 43, 45, 48, 51, 53, 56, 59, 62, 65, 68
        };
            
        constexpr auto level_H = std::array{ 
            1, 1, 2, 4, 4, 4, 5, 6, 8, 8, 11, 11, 16, 16, 18, 16, 19, 21, 25, 25, 25, 
            34, 30, 32, 35, 37, 40, 42, 45, 48, 51, 54, 57, 60, 63, 66, 70, 74, 77, 81
        };
            
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
    constexpr auto total_blocks_returns_the_number_of_blocks_depending_on_qr_version_and_error_correction()
    {
        static_assert(total_blocks({symbol_version{1}, error_correction::level_L}) == 1);
        static_assert(total_blocks({symbol_version{12}, error_correction::level_M}) == 8);
        static_assert(total_blocks({symbol_version{25}, error_correction::level_Q}) == 29);
        static_assert(total_blocks({symbol_version{6}, error_correction::level_H}) == 4);
    }
}
#endif
