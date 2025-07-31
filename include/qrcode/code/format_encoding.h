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

#include <qrcode/code/lfsr.h>
#include <qrcode/code/error_correction_code.h>

namespace qrcode::code
{   
    struct raw_format 
    { 
        std::uint16_t data;
        [[nodiscard]] constexpr auto operator==(raw_format const&) const noexcept -> bool = default;
        [[nodiscard]] constexpr auto operator!=(raw_format const&) const noexcept -> bool = default;
    };
    
    struct encoded_format 
    { 
        std::uint16_t data; 
        [[nodiscard]] constexpr auto operator==(encoded_format const&) const noexcept -> bool = default;
        [[nodiscard]] constexpr auto operator!=(encoded_format const&) const noexcept -> bool = default;
    };
    
    struct masked_format 
    { 
        std::uint16_t data; 
        [[nodiscard]] constexpr auto operator==(masked_format const&) const noexcept -> bool = default;
        [[nodiscard]] constexpr auto operator!=(masked_format const&) const noexcept -> bool = default;
    };
    
    struct format_mask { std::uint16_t pattern; };

    [[nodiscard]] constexpr auto encode(raw_format raw, error_correcting_code code) noexcept
    {
        auto division_remainder = lfsr{
            code.generator, static_cast<decltype(code.generator)>(raw.data)};

        auto const power = code.total_size - code.data_size;
        
        for (auto i = 0; i != power; ++i)
            division_remainder.advance(0);
            
        return encoded_format{
            static_cast<std::uint16_t>(raw.data << power | state(division_remainder))};
    }

    [[nodiscard]] constexpr auto masked(
        encoded_format format_information, format_mask mask) noexcept
    {
        return masked_format{static_cast<std::uint16_t>(format_information.data ^ mask.pattern)};
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::code::test
{
    constexpr auto encode_adds_error_correction_to_raw_format_information()
    {
        constexpr auto code = error_correcting_code{15, 5, 0b101'0011'0111};

        static_assert(encode(raw_format{0b00000}, code) == encoded_format{0b00000'0000000000});
        static_assert(encode(raw_format{0b00001}, code) == encoded_format{0b00001'0100110111});
        static_assert(encode(raw_format{0b00010}, code) == encoded_format{0b00010'1001101110});
        static_assert(encode(raw_format{0b00011}, code) == encoded_format{0b00011'1101011001});
        static_assert(encode(raw_format{0b00100}, code) == encoded_format{0b00100'0111101011});
        static_assert(encode(raw_format{0b00101}, code) == encoded_format{0b00101'0011011100});
        static_assert(encode(raw_format{0b00110}, code) == encoded_format{0b00110'1110000101});
        static_assert(encode(raw_format{0b00111}, code) == encoded_format{0b00111'1010110010});
        static_assert(encode(raw_format{0b01000}, code) == encoded_format{0b01000'1111010110});
        static_assert(encode(raw_format{0b01001}, code) == encoded_format{0b01001'1011100001});
        static_assert(encode(raw_format{0b01010}, code) == encoded_format{0b01010'0110111000});
        static_assert(encode(raw_format{0b01011}, code) == encoded_format{0b01011'0010001111});
        static_assert(encode(raw_format{0b01100}, code) == encoded_format{0b01100'1000111101});
        static_assert(encode(raw_format{0b01101}, code) == encoded_format{0b01101'1100001010});
        static_assert(encode(raw_format{0b01110}, code) == encoded_format{0b01110'0001010011});
        static_assert(encode(raw_format{0b01111}, code) == encoded_format{0b01111'0101100100});
        static_assert(encode(raw_format{0b10000}, code) == encoded_format{0b10000'1010011011});
        static_assert(encode(raw_format{0b10001}, code) == encoded_format{0b10001'1110101100});
        static_assert(encode(raw_format{0b10010}, code) == encoded_format{0b10010'0011110101});
        static_assert(encode(raw_format{0b10011}, code) == encoded_format{0b10011'0111000010});
        static_assert(encode(raw_format{0b10100}, code) == encoded_format{0b10100'1101110000});
        static_assert(encode(raw_format{0b10101}, code) == encoded_format{0b10101'1001000111});
        static_assert(encode(raw_format{0b10110}, code) == encoded_format{0b10110'0100011110});
        static_assert(encode(raw_format{0b10111}, code) == encoded_format{0b10111'0000101001});
        static_assert(encode(raw_format{0b11000}, code) == encoded_format{0b11000'0101001101});
        static_assert(encode(raw_format{0b11001}, code) == encoded_format{0b11001'0001111010});
        static_assert(encode(raw_format{0b11010}, code) == encoded_format{0b11010'1100100011});
        static_assert(encode(raw_format{0b11011}, code) == encoded_format{0b11011'1000010100});
        static_assert(encode(raw_format{0b11100}, code) == encoded_format{0b11100'0010100110});
        static_assert(encode(raw_format{0b11101}, code) == encoded_format{0b11101'0110010001});
        static_assert(encode(raw_format{0b11110}, code) == encoded_format{0b11110'1011001000});
        static_assert(encode(raw_format{0b11111}, code) == encoded_format{0b11111'1111111111});
    }

    constexpr auto mask_applies_encoded_format_with_given_mask_pattern_by_xoring_bitwise()
    {
        static_assert(masked(encoded_format{0b1100'1010'0101'1011}, format_mask{0b0000'0000'0000'0000}) == masked_format{0b1100'1010'0101'1011});
        static_assert(masked(encoded_format{0b1100'1010'0101'1011}, format_mask{0b1100'1010'0101'1011}) == masked_format{0b0000'0000'0000'0000});
        static_assert(masked(encoded_format{0b1100'1010'0101'1011}, format_mask{0b1111'1111'1111'1111}) == masked_format{0b0011'0101'1010'0100});
    }
}
#endif
