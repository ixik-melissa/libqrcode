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
#include <qrcode/data/bit_stream.h>

namespace qrcode::eci
{
    struct assignment_number
    {
        std::int32_t value;
    };

    using qrcode::data::bit_stream;

    [[nodiscard]] constexpr auto encode(
        bit_stream& stream, assignment_number assignment_number) noexcept
    {
        using namespace qrcode::data::literals;
        constexpr auto bits_in_byte = 8_bits;
        auto const value = assignment_number.value;

        if (value < 128)
            stream.add(value, bits_in_byte);

        if (value >= 128 && value < 16384)
        {
            stream.add(0b1000'0000 | ((value >> 8) & 0b0011'1111), bits_in_byte);
            stream.add(value, bits_in_byte);
        }

        if (value >= 16384)
        {
            stream.add(0b1100'0000 | ((value >> 16) & 0b0001'1111), bits_in_byte);
            stream.add(value >> 8, bits_in_byte);
            stream.add(value, bits_in_byte);
        }
    }

    [[nodiscard]] constexpr auto bit_count(assignment_number const& assignment_number) noexcept
    {
        auto const value = assignment_number.value;
        if (value < 128)
            return 8;

        if (value >= 128 && value < 16384)
            return 16;

        return 24;
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::eci::test
{
    template<class...Ts>
    constexpr auto encode_wrapper(Ts&&...ts)
    {
        auto stream = bit_stream{};
        encode(stream, std::forward<Ts>(ts)...);
        return stream;
    }

    constexpr auto assignment_numbers_can_be_encoded_to_given_bit_stream()
    {
        static_assert(std::ranges::equal(
            encode_wrapper(assignment_number{0}).get(), 
            std::array<bool,8>{0,0,0,0,0,0,0,0}
        ));

        static_assert(std::ranges::equal(
            encode_wrapper(assignment_number{127}).get(), 
            std::array<bool,8>{0,1,1,1,1,1,1,1}
        ));

        static_assert(std::ranges::equal(
            encode_wrapper(assignment_number{128}).get(), 
            std::array<bool,16>{1,0, 0,0,0,0,0,0,  1,0,0,0,0,0,0,0}
        ));

        static_assert(std::ranges::equal(
            encode_wrapper(assignment_number{16383}).get(), 
            std::array<bool,16>{1,0, 1,1,1,1,1,1,  1,1,1,1,1,1,1,1}
        ));

        static_assert(std::ranges::equal(
            encode_wrapper(assignment_number{16384}).get(), 
            std::array<bool,24>{1,1,0, 0,0,0,0,0,  0,1,0,0,0,0,0,0,  0,0,0,0,0,0,0,0}
        ));

        static_assert(std::ranges::equal(
            encode_wrapper(assignment_number{999999}).get(), 
            std::array<bool,24>{1,1,0, 0,1,1,1,1,  0,1,0,0,0,0,1,0,  0,0,1,1,1,1,1,1}
        ));
    }

    constexpr auto assignment_numbers_provide_their_bit_count()
    {
        static_assert(bit_count(assignment_number{0}) == 8);
        static_assert(bit_count(assignment_number{127}) == 8);
        static_assert(bit_count(assignment_number{128}) == 16);
        static_assert(bit_count(assignment_number{16383}) == 16);
        static_assert(bit_count(assignment_number{16384}) == 24);
        static_assert(bit_count(assignment_number{999999}) == 24);
    }
}
#endif