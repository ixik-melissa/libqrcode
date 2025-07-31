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
#include <cstddef> 
 
namespace qrcode::code
{
    class gf2p8
    {
        std::uint8_t value{0x00}; 
    
    public:
        constexpr gf2p8(std::uint8_t number) noexcept 
        : value{static_cast<decltype(value)>(number)} 
        {
        }

        constexpr gf2p8(std::byte number) noexcept 
        : gf2p8{std::to_integer<std::uint8_t>(number)} 
        {
        }

        constexpr gf2p8() noexcept = default;

        [[nodiscard]] static constexpr auto primitive_element() noexcept
        {
            return gf2p8{2};
        }
        
        [[nodiscard]] static constexpr auto prime_polynomial() noexcept
        {
            return 0b100011101u; // x^8 + x^4 + x^3 + x^2 + 1
        }

        [[nodiscard]] constexpr explicit operator std::byte() const noexcept
        {
            return std::byte{value};
        }
            
        constexpr auto& operator*=(gf2p8 rhs) noexcept
        {  
            // russian peasant multiplication with additional polynomial reduction
            auto product = 0u;
            
            while(rhs.value)
            {
                if (rhs.value & 0x01)
                    product ^= value; 
                    
                rhs.value >>= 1;
                value = value & 0x80 ? (value << 1) ^ prime_polynomial() : value << 1;
            }
    
            value = product;
            return *this;
        }
        
        constexpr auto& operator+=(gf2p8 rhs) noexcept
        {
            value ^= rhs.value;
            return *this;
        }

        [[nodiscard]] constexpr auto operator==(gf2p8 const&) const noexcept -> bool = default;
        [[nodiscard]] constexpr auto operator!=(gf2p8 const&) const noexcept -> bool = default;
    };
    
    [[nodiscard]] constexpr auto operator+(gf2p8 a, gf2p8 b) noexcept
    {
        a += b;
        return a;
    }
    
    [[nodiscard]] constexpr auto operator*(gf2p8 a, gf2p8 b) noexcept
    {
        a *= b;
        return a;
    }

    inline namespace literals
    {
        [[nodiscard]] constexpr auto operator"" _gf(unsigned long long int value) noexcept
        {
            return gf2p8{static_cast<std::uint8_t>(value)};
        }
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <bitset>
#include <ios>
#include <iomanip>

namespace qrcode::code::test
{
    constexpr auto zero() { return std::uint8_t{0}; }
    constexpr auto one() { return std::uint8_t{1}; }
    constexpr auto two() { return std::uint8_t{2}; }
    constexpr auto three() { return std::uint8_t{3}; }
    constexpr auto four() { return std::uint8_t{4}; }
    constexpr auto any_number() { return std::uint8_t{0x17}; }
    constexpr auto other_number() { return std::uint8_t{0x2E}; }
    constexpr auto all_one() { return std::uint8_t{0xFF}; }
    constexpr auto its_complement() { return std::uint8_t{0xE8}; }
    constexpr auto their_xored_result() { return std::uint8_t{0x39}; }
    constexpr auto overflow_result() { return std::uint8_t{0x1D}; }
    constexpr auto any_number_times_three() { return std::uint8_t{0x39}; }
    constexpr auto any_number_times_four() { return std::uint8_t{0x5C}; }
    constexpr auto any_number_which_will_overflow() { return std::uint8_t{0x80}; }
    constexpr auto any_gf2p8() { return gf2p8{0x12}; }
    constexpr auto other_gf2p8() { return gf2p8{0xFA}; }

    constexpr auto gf2p8_is_zero_by_default()
    {
        static_assert(gf2p8{} == gf2p8{0});
    }

    constexpr auto gf2p8_can_be_converted_to_a_byte()
    {
        static_assert(static_cast<std::byte>(gf2p8{any_number()}) == std::byte{any_number()});
        static_assert(static_cast<std::byte>(gf2p8{other_number()}) == std::byte{other_number()});
    }

    constexpr auto gf2p8_can_be_compared_to_another()
    {
        static_assert(any_gf2p8() == any_gf2p8());
        static_assert(any_gf2p8() != other_gf2p8());
    }

    constexpr auto gf2p8_has_its_own_literal()
    {
        static_assert(0_gf == gf2p8{0});
        static_assert(1_gf == gf2p8{1});
        static_assert(0x7F_gf == gf2p8{0x7F});
    }
    
    constexpr auto gf2p8_addition_is_done_by_xoring_bitwise()
    {
        static_assert((gf2p8{any_number()} + gf2p8{zero()}) == gf2p8{any_number()});
        static_assert((gf2p8{any_number()} + gf2p8{any_number()}) == gf2p8{zero()});
        static_assert((gf2p8{any_number()} + gf2p8{all_one()}) == gf2p8{its_complement()});
        static_assert((gf2p8{any_number()} + gf2p8{other_number()}) == gf2p8{their_xored_result()});
    }

    constexpr auto gf2p8_multiplication_is_done_using_modulo_qr_code_specific_irreducible_polynomial()
    {
        static_assert((gf2p8{any_number()} * gf2p8{zero()}) == gf2p8{zero()});
        static_assert((gf2p8{any_number()} * gf2p8{one()}) == gf2p8{any_number()});
        static_assert((gf2p8{any_number()} * gf2p8{three()}) == gf2p8{any_number_times_three()});
        static_assert((gf2p8{any_number()} * gf2p8{four()}) == gf2p8{any_number_times_four()});
        static_assert((gf2p8{any_number_which_will_overflow()} * gf2p8{two()}) ==  gf2p8{overflow_result()});
    }

    [[nodiscard]] constexpr auto primitive_element_table() noexcept
    {
        std::array<gf2p8, 256> result;
            
        auto current = 1_gf;    
            
        for (int i = 0; i != 255; ++i, current *= gf2p8::primitive_element())
            result[i] = current;
            
        return result;
    }

    template<class Stream>
    constexpr auto gf2p8_primitive_element_table_print_out(Stream& s)
    {
        auto const t = primitive_element_table();
        for (auto i = 0u; i != size(t); ++i)
        {
            auto const value = static_cast<unsigned>(static_cast<std::byte>(t[i]));
            s 
                << std::dec << std::setfill(' ') << std::setw(3) << i << " " 
                << std::hex << std::setfill('0') << std::setw(2) << value << " " 
                << std::bitset<8>(value) << "\n";
        }
    }
}
#endif
