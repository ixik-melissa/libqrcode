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

#include <algorithm>
#include <concepts>
#include <cx/vector.h>
#include <qrcode/code/polynomial.h>
#include <qrcode/code/gf2p8.h>

namespace qrcode::code::detail
{
    template<std::ranges::range Range>
    requires std::is_default_constructible_v<std::ranges::range_value_t<Range>>
    [[nodiscard]] constexpr auto shift_right(Range&& range) noexcept
    {
        using std::ranges::begin;
        using std::ranges::end;
        using value_type = std::ranges::range_value_t<Range>;
              
        auto i = end(range);      
        while((--i) != begin(range))
            *i = *(i-1);  
 
        *begin(range) = value_type{};
    }
}

namespace qrcode::code
{
    // based on 
    // ISO/IEC-18004-2006 2nd edition 2006-09-01: 6.5.2 Generating the error correction codewords
    template<class T>
    [[nodiscard]] constexpr auto extended_remainder(
        polynomial<T> const& data, polynomial<T> const& generator) noexcept
    {
        using qrcode::code::detail::shift_right;
        
        auto result = polynomial(degree(generator)-1, T{0});

        auto& last = *rbegin(result);
        
        for (auto i = rbegin(data); i != rend(data); ++i)
        {
            auto const weight = *i + last;
            shift_right(result);
            
            std::transform(
                begin(result), end(result), begin(generator), begin(result), 
                [weight](auto r, auto g){ return r + weight * g; }
            );
        }
    
        return result;
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::code::detail::test
{
    constexpr auto shift_right_constructs_a_new_container_with_elements_shifted_right_by_one_and_padded_with_zero()
    {
        auto f = [](auto range, auto nominal)
        {
            shift_right(range);
            return std::ranges::equal(range, nominal);
        };

        static_assert(f(std::array{3}, std::array{0}));
        static_assert(f(std::array{0, 1}, std::array{0, 0}));
        static_assert(f(std::array{1, 2, 3}, std::array{0, 1, 2}));
    }
}

namespace qrcode::code::test
{
    constexpr auto extended_remainder_computes_remainder_of_x_power_k_times_given_polynomial_modulo_generator_polynomial()
    {
        static_assert(
            extended_remainder(polynomial{{7_gf}}, polynomial{{0_gf, 1_gf}})
                == polynomial{{0_gf}}
        ); // x*(7) % x = 0
        
        static_assert(
            extended_remainder(polynomial{{1_gf, 1_gf}}, polynomial{{1_gf, 0_gf, 1_gf}})
                == polynomial{{1_gf, 1_gf}}
        ); // x^2*(x+1) % (x^2+1) = x+1
        
        static_assert(
            extended_remainder(polynomial{{1_gf, 1_gf, 1_gf}}, polynomial{{7_gf, 1_gf, 3_gf, 1_gf}}) 
                == polynomial{{18_gf, 8_gf, 15_gf}}
        ); // x^3(x^2+x+1) % x^3+3x^2+x+7 = 15x^2+8x+18

        static_assert(
            extended_remainder(polynomial{{1_gf, 2_gf, 3_gf}}, polynomial{{2_gf, 3_gf, 1_gf}})
                == polynomial{{28_gf, 28_gf}}
        ); // x^2(3x^2+2x+1) % x^2+3x+2 = 28x+28
    }
}
#endif
