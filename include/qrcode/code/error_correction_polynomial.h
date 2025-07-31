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

#include <iterator>
#include <qrcode/code/gf2p8.h>
#include <qrcode/code/polynomial.h>

namespace qrcode::code::detail
{
    template<std::ranges::view Coefficients>
    [[nodiscard]] constexpr auto apply_element(Coefficients&& coefficients, gf2p8 element)
    {
        for (auto i = begin(coefficients), next = i+1; next != end(coefficients); ++i, ++next)
            *i = element * *i + *next;
    }
}

namespace qrcode::code
{
    [[nodiscard]] constexpr auto error_correction_polynomial(int degree) noexcept
    { 
        // Each generator polynomial is the product of the first degree polynomials: 
        // x-2^0 , x-2^1, ..., x-2^(n-1) ; where n is the degree of the generator polynomial.
        auto result = polynomial<gf2p8>(degree, 1_gf);
        auto element = 1_gf;
        
        for (auto i = 0; i != degree; ++i, element *= gf2p8::primitive_element())
        {
            using qrcode::code::detail::apply_element;
            using std::ranges::subrange;
            using std::ranges::reverse_view;
            apply_element(
                reverse_view{subrange{begin(result), begin(result)+i+1}}, 
                element
            );
            *result.begin() *= element;
        }
            
        return result;    
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::code::test
{
    constexpr auto error_correction_polynomial_can_be_constructed_from_given_degree()
    {
        static_assert(error_correction_polynomial(1) == polynomial{{0x01_gf, 0x01_gf}});
        static_assert(error_correction_polynomial(2) == polynomial{{0x02_gf, 0x03_gf, 0x01_gf}});
        static_assert(error_correction_polynomial(3) == polynomial{{0x08_gf, 0x0E_gf, 0x07_gf, 0x01_gf}});
        static_assert(error_correction_polynomial(4) == polynomial{{0x40_gf, 0x78_gf, 0x36_gf, 0x0F_gf, 0x01_gf}});
        static_assert(error_correction_polynomial(5) == polynomial{{0x74_gf, 0x93_gf, 0x3F_gf, 0xC6_gf, 0x1F_gf, 0x01_gf}});
        // it seems, ISO/IEC-18004-2006 2nd edition 2006-09-01 has a typo for degree 6
        static_assert(error_correction_polynomial(6) == polynomial{{0x26_gf, 0xE3_gf, 0x20_gf, 0xDA_gf, 0x01_gf, 0x3F_gf, 0x01_gf}}); 
        static_assert(error_correction_polynomial(7) == polynomial{{0x75_gf, 0x44_gf, 0x0B_gf, 0xA4_gf, 0x9A_gf, 0x7A_gf, 0x7F_gf, 0x01_gf}});
        static_assert(error_correction_polynomial(22) == polynomial{{
            0xF5_gf, 0x91_gf, 0x1A_gf, 0xE6_gf, 0xDA_gf, 0x56_gf, 0xFD_gf, 0x43_gf, 
            0x7B_gf, 0x1D_gf, 0x89_gf, 0x1C_gf, 0x28_gf, 0x45_gf, 0xBD_gf, 0x13_gf, 
            0xF4_gf, 0xB6_gf, 0xB0_gf, 0x83_gf, 0xB3_gf, 0x59_gf, 0x01_gf}}
        );
    }
}
#endif
