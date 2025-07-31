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

#include <qrcode/eci/assignment_number.h>
#include <qrcode/data/bit_stream.h>

namespace qrcode::eci
{
    struct message_header
    {
        assignment_number eci_assignment_number;
    };

    using qrcode::data::bit_stream;

    [[nodiscard]] constexpr auto encode(bit_stream& stream, message_header const& header) noexcept
    {
        using namespace qrcode::data::literals;
        stream.add(0b0111, 4_bits);
        encode(stream, header.eci_assignment_number);
    }

    [[nodiscard]] constexpr auto bit_count(message_header const& header) noexcept
    {
        return 4 + bit_count(header.eci_assignment_number);
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::eci::test
{
    constexpr auto eci_headers_can_be_encoded_to_given_bit_stream()
    {
        static_assert(std::ranges::equal(
            encode_wrapper(message_header{assignment_number{0}}).get(), 
            std::array<bool,12>{0,1,1,1,  0,0,0,0,0,0,0,0}
        ));

        static_assert(std::ranges::equal(
            encode_wrapper(message_header{128}).get(), 
            std::array<bool,20>{0,1,1,1,  1,0,0,0,0,0,0,0,  1,0,0,0,0,0,0,0}
        ));
    }

    constexpr auto eci_headers_provide_their_bit_count()
    {
        static_assert(bit_count(message_header{assignment_number{0}}) == (4+8));
        static_assert(bit_count(message_header{assignment_number{127}}) == (4+8));
        static_assert(bit_count(message_header{assignment_number{128}}) == (4+16));
        static_assert(bit_count(message_header{assignment_number{16383}}) == (4+16));
        static_assert(bit_count(message_header{assignment_number{16384}}) == (4+24));
        static_assert(bit_count(message_header{assignment_number{999999}}) == (4+24));
    }
}
#endif