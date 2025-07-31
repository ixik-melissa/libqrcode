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

#include <array>

#include <qrcode/qr/symbol_designator.h>

namespace qrcode::qr
{
    [[nodiscard]] constexpr auto total_data_bits(error_correction error_level) noexcept
    {
        constexpr auto level_L = std::array{
            152, 272, 440, 640, 864, 1088, 1248, 1552, 1856, 2192, 2592, 2960, 3424, 3688, 4184, 
            4712, 5176, 5768, 6360, 6888, 7456, 8048, 8752, 9392, 10208, 10960, 11744, 12248, 
            13048, 13880, 14744, 15640, 16568, 17528, 18448, 19472, 20528, 21616, 22496, 23648
        };

        constexpr auto level_M = std::array{
            128, 224, 352, 512, 688, 864, 992, 1232, 1456, 1728, 2032, 2320, 2672, 2920, 3320, 
            3624, 4056, 4504, 5016, 5352, 5712, 6256, 6880, 7312, 8000, 8496, 9024, 9544, 10136, 
            10984, 11640, 12328, 13048, 13800, 14496, 15312, 15936, 16816, 17728, 18672
        };

        constexpr auto level_Q = std::array{
            104, 176, 272, 384, 496, 608, 704, 880, 1056, 1232, 1440, 1648, 1952, 2088, 2360, 
            2600, 2936, 3176, 3560, 3880, 4096, 4544, 4912, 5312, 5744, 6032, 6464, 6968, 7288, 
            7880, 8264, 8920, 9368, 9848, 10288, 10832, 11408, 12016, 12656, 13328
        };

        constexpr auto level_H = std::array{
            72, 128, 208, 288, 368, 480, 528, 688, 800, 976, 1120, 1264, 1440, 1576, 1784, 2024, 
            2264, 2504, 2728, 3080, 3248, 3536, 3712, 4112, 4304, 4768, 5024, 5288, 5608, 5960, 
            6344, 6760, 7208, 7688, 7888, 8432, 8768, 9136, 9776, 10208
        };

        switch(error_level)
        {
            case error_correction::level_L: return level_L;
            case error_correction::level_M: return level_M;
            case error_correction::level_Q: return level_Q;
            case error_correction::level_H: return level_H;
        }
        return level_L;
    }

    [[nodiscard]] constexpr auto total_data_bits(symbol_designator const& designator) noexcept
    {
        return total_data_bits(error_level(designator))[version(designator).number-1];
    }
}


#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::qr::test
{
    constexpr auto total_data_bits_returns_the_numbers_of_data_bits_available_for_given_symbol_version_and_error_correction_level()
    {
        static_assert(total_data_bits({symbol_version{ 1}, error_correction::level_L}) == 152);
        static_assert(total_data_bits({symbol_version{ 8}, error_correction::level_L}) == 1552);
        static_assert(total_data_bits({symbol_version{19}, error_correction::level_L}) == 6360);
        static_assert(total_data_bits({symbol_version{24}, error_correction::level_L}) == 9392);
        static_assert(total_data_bits({symbol_version{39}, error_correction::level_L}) == 22496);

        static_assert(total_data_bits({symbol_version{ 1}, error_correction::level_M}) == 128);
        static_assert(total_data_bits({symbol_version{ 8}, error_correction::level_M}) == 1232);
        static_assert(total_data_bits({symbol_version{19}, error_correction::level_M}) == 5016);
        static_assert(total_data_bits({symbol_version{24}, error_correction::level_M}) == 7312);
        static_assert(total_data_bits({symbol_version{39}, error_correction::level_M}) == 17728);

        static_assert(total_data_bits({symbol_version{ 1}, error_correction::level_Q}) == 104);
        static_assert(total_data_bits({symbol_version{ 8}, error_correction::level_Q}) == 880);
        static_assert(total_data_bits({symbol_version{19}, error_correction::level_Q}) == 3560);
        static_assert(total_data_bits({symbol_version{24}, error_correction::level_Q}) == 5312);
        static_assert(total_data_bits({symbol_version{39}, error_correction::level_Q}) == 12656);

        static_assert(total_data_bits({symbol_version{ 1}, error_correction::level_H}) == 72);
        static_assert(total_data_bits({symbol_version{ 8}, error_correction::level_H}) == 688);
        static_assert(total_data_bits({symbol_version{19}, error_correction::level_H}) == 2728);
        static_assert(total_data_bits({symbol_version{24}, error_correction::level_H}) == 4112);
        static_assert(total_data_bits({symbol_version{39}, error_correction::level_H}) == 9776);
    }
}
#endif
