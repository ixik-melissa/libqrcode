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

#include <algorithm>
#include <optional>
#include <ranges>

#include <qrcode/micro_qr/error_correction.h>
#include <qrcode/micro_qr/symbol_version.h>
#include <qrcode/micro_qr/total_data_bits.h>

namespace qrcode::micro_qr
{
    [[nodiscard]] constexpr auto best_version(
        std::optional<error_correction> const& error_correction_level, int data_bit_count) noexcept
    {
        auto const bit_table = data_capacities();
        auto matching = bit_table 
            | std::views::filter([=](auto& v){ return error_correction_level == error_level(v.first); })
            | std::views::filter([=](auto& v){ return data_bit_count <= v.second; });

        return begin(matching) == end(matching)
            ? std::optional<symbol_version>{}
            : std::optional<symbol_version>{version(begin(matching)->first)};
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::micro_qr::test
{
    constexpr auto best_version_returns_the_version_which_fits_given_data_size_best()
    {
        static_assert(best_version(std::nullopt, 20) == symbol_version::M1);

        static_assert(best_version(error_correction::level_L, 20) == symbol_version::M2);
        static_assert(best_version(error_correction::level_L, 40) == symbol_version::M2);
        static_assert(best_version(error_correction::level_M, 32) == symbol_version::M2);
  
        static_assert(best_version(error_correction::level_L, 84) == symbol_version::M3);
        static_assert(best_version(error_correction::level_M, 68) == symbol_version::M3);
     
        static_assert(best_version(error_correction::level_L, 128) == symbol_version::M4);
        static_assert(best_version(error_correction::level_M, 112) == symbol_version::M4);
        static_assert(best_version(error_correction::level_Q, 80) == symbol_version::M4);
    }

    constexpr auto best_version_returns_nothing_when_symbol_cannot_cover_given_number_of_data_bits()
    {
        static_assert(best_version(error_correction::level_L, 128+1) == std::nullopt);
        static_assert(best_version(error_correction::level_M, 112+1) == std::nullopt);
        static_assert(best_version(error_correction::level_Q, 80+1) == std::nullopt);
    }
}
#endif
