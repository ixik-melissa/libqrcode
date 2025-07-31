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

#include <qrcode/data/bit_stream.h>

namespace qrcode::data
{
    struct mode_indicator
    {
        int value = 0;
        int bit_count = 0;

        [[nodiscard]] constexpr auto operator==(mode_indicator const&) const noexcept -> bool = default;
    };

    struct count_indicator
    {
        int bit_count = 0;

        [[nodiscard]] constexpr auto operator==(count_indicator const&) const noexcept -> bool = default;
    };

    [[nodiscard]] constexpr auto encode(bit_stream& stream, mode_indicator const& mode) noexcept
    {
        stream.add(mode.value, number_bits{mode.bit_count});
    }

    [[nodiscard]] constexpr auto encode(
        bit_stream& stream, count_indicator const& count, int character_count) noexcept
    {
        stream.add(character_count, number_bits{count.bit_count});
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::data::test
{
    constexpr auto mode_indicators_can_be_encoded_to_given_bit_stream()
    {
        auto f = []
        {
            auto const any_mode_indicator = mode_indicator{.value=0b10,.bit_count=5};
            auto stream = bit_stream{};

            encode(stream, any_mode_indicator);

            return std::ranges::equal(stream.get(), std::array<bool,5>{0,0,0,1,0});
        };
        static_assert(f());
    }

    constexpr auto count_indicators_take_given_count_and_encode_it_to_given_bit_stream()
    {
        auto f = []
        {
            auto const any_count_indicator = count_indicator{.bit_count=10};
            auto const any_count = 0b1001'0011;
            auto stream = bit_stream{};

            encode(stream, any_count_indicator, any_count);

            return std::ranges::equal(stream.get(), std::array<bool,10>{0,0,1,0,0,1,0,0,1,1});
        };
        static_assert(f());
    }
}
#endif
