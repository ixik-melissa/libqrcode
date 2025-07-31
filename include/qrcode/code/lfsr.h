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

#include <concepts>

namespace qrcode::code::detail
{
    template<std::unsigned_integral T>
    [[nodiscard]] constexpr auto msb_index(T v) noexcept
    {
        auto index = -1;
        for(; v != 0; v >>= 1, ++index) {}
        return index;
    }
}

namespace qrcode::code
{   
    template<class T>
    requires std::unsigned_integral<T>
    class lfsr
    {
    public:
        explicit constexpr lfsr(T generator_polynomial, T initial_state) noexcept
        : generator{generator_polynomial}, state{initial_state}
        {
        }
        
        [[nodiscard]] constexpr auto advance(bool bit) noexcept
        {
            using qrcode::code::detail::msb_index;
            auto const last_bit = state >> (msb_index(generator)-1);
            state = ((state << 1) | static_cast<T>(bit)) ^ (last_bit * generator);
        }
        
        [[nodiscard]] friend constexpr auto state(lfsr const& instance) noexcept
        {
            return instance.state;
        }
        
    private:
        T generator;
        T state;
    };
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::code::detail::test
{
    constexpr auto msb_index_returns_the_bit_position_of_the_most_significant_bit_being_set()
    {
        static_assert(msb_index(0b0001u) == 0);
        static_assert(msb_index(0b0010u) == 1);
        static_assert(msb_index(0b0011u) == 1);
        static_assert(msb_index(0b0100u) == 2);
        static_assert(msb_index(0b1100u) == 3);
    }
}

namespace qrcode::code::test
{
    constexpr auto lfsr_can_be_advanced()
    {
        // lfsr<0b110> => polynomial: x^2 + x
        static_assert([]{ lfsr l{0b110u, 0u}; l.advance(1); return state(l); }() == 0b01);
        static_assert([]{ lfsr l{0b110u, 0u}; l.advance(1); l.advance(0); return state(l); }() == 0b10);
        static_assert([]{ lfsr l{0b110u, 0u}; l.advance(1); l.advance(0); l.advance(1); return state(l); }() == 0b11);
        static_assert([]{ lfsr l{0b110u, 0u}; l.advance(1); l.advance(0); l.advance(1); l.advance(1); return state(l); }() == 0b01);
    }

    constexpr auto lfsr_can_be_initialized_with_given_state()
    {
        // lfsr<0b110> => polynomial: x^2 + x
        static_assert([]{ lfsr l{0b110u, 0b11u}; l.advance(1); return state(l); }() == 0b01);
    }
}
#endif
