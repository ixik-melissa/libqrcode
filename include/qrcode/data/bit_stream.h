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

#include <cx/vector.h>

namespace qrcode::data
{
    struct number_bits 
    { 
        int value = 0; 
    };

    inline namespace literals
    {
        [[nodiscard]] constexpr auto operator"" _bits(unsigned long long int value) noexcept
        {
            return number_bits{static_cast<int>(value)};
        }
    }

    class bit_stream
    {
    public:
        constexpr bit_stream() noexcept = default;

        template<class T>
        constexpr auto add(T data, number_bits size) noexcept -> bit_stream&
        {
            auto mask = T{1} << (std::max(size.value,1)-1);

            for (auto i = 0; i != size.value; ++i, mask >>= 1)
                stream.push_back(static_cast<bool>(data & mask) ? 1 : 0);
                
            return *this;
        }

        [[nodiscard]] constexpr auto get() const noexcept
        {
            return stream;
        }

        [[nodiscard]] constexpr auto size() const noexcept
        {
            using std::ranges::size;
            return static_cast<int>(size(stream));
        }

    private:
        cx::vector<bool> stream;
    };

    using std::ranges::size;
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::data::test
{
    constexpr auto bit_streams_accept_integers_with_their_bit_count_and_convert_it_to_a_binary_stream_msb_first()
    {
        static_assert(std::ranges::equal(bit_stream{}.add(0, 0_bits).get(), std::array<bool,0>{}));
        static_assert(std::ranges::equal(bit_stream{}.add(0b0, 1_bits).get(), std::array<bool,1>{0}));
        static_assert(std::ranges::equal(bit_stream{}.add(0b1, 1_bits).get(), std::array<bool,1>{1}));
        static_assert(std::ranges::equal(bit_stream{}.add(0b10, 2_bits).get(), std::array<bool,2>{1,0}));
        static_assert(std::ranges::equal(bit_stream{}.add(0b110, 3_bits).get(), std::array<bool,3>{1,1,0}));
        static_assert(std::ranges::equal(bit_stream{}.add(0b110, 3_bits).add(0b00010, 5_bits).get(), std::array<bool,8>{1,1,0,0,0,0,1,0}));
        static_assert(std::ranges::equal(bit_stream{}.add(6, 16_bits).get(), std::array<bool,16>{0,0,0,0,  0,0,0,0,  0,0,0,0,  0,1,1,0}));
    }

    constexpr auto bit_streams_have_a_size()
    {
        static_assert(size(bit_stream{}.add(0, 0_bits)) == 0);
        static_assert(size(bit_stream{}.add(0, 1_bits)) == 1);
        static_assert(size(bit_stream{}.add(2, 2_bits)) == 2);
        static_assert(size(bit_stream{}.add(6, 3_bits)) == 3);
        static_assert(size(bit_stream{}.add(6, 3_bits).add(2, 5_bits)) == 8);
    }
}
#endif