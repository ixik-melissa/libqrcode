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

#include <ranges>
#include <algorithm>
#include <optional>

#include <qrcode/qr/error_correction.h>
#include <qrcode/qr/symbol_version.h>
#include <qrcode/qr/total_data_bits.h>

namespace qrcode::qr
{
    [[nodiscard]] constexpr auto best_version(
        error_correction error_level, int data_bit_count) noexcept
    {
        auto const bit_table = total_data_bits(error_level);
        auto const found = std::ranges::upper_bound(bit_table, data_bit_count, std::less_equal{});
        auto const version_number = static_cast<int>(1+std::distance(begin(bit_table), found));
        return found != end(bit_table) 
            ? std::optional<symbol_version>{version_number} 
            : std::optional<symbol_version>{};
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::qr::test
{
    constexpr auto best_version_returns_the_version_which_fits_given_data_size_best()
    {
        static_assert(best_version(error_correction::level_L, 152) == symbol_version{1});
        static_assert(best_version(error_correction::level_L, 260) == symbol_version{2});
        static_assert(best_version(error_correction::level_L, 1200) == symbol_version{7});

        static_assert(best_version(error_correction::level_M, 1) == symbol_version{1});
        static_assert(best_version(error_correction::level_M, 260) == symbol_version{3});
        static_assert(best_version(error_correction::level_M, 1200) == symbol_version{8});

        static_assert(best_version(error_correction::level_Q, 1) == symbol_version{1});
        static_assert(best_version(error_correction::level_Q, 360) == symbol_version{4});
        static_assert(best_version(error_correction::level_Q, 1200) == symbol_version{10});

        static_assert(best_version(error_correction::level_H, 1) == symbol_version{1});
        static_assert(best_version(error_correction::level_H, 360) == symbol_version{5});
        static_assert(best_version(error_correction::level_H, 1200) == symbol_version{12});

        static_assert(best_version(error_correction::level_L, 23648) == symbol_version{40});
        static_assert(best_version(error_correction::level_M, 18672) == symbol_version{40});
        static_assert(best_version(error_correction::level_Q, 13328) == symbol_version{40});
        static_assert(best_version(error_correction::level_H, 10208) == symbol_version{40});
    }

    constexpr auto best_version_returns_nothing_when_symbol_cannot_cover_given_number_of_data_bits()
    {
        static_assert(best_version(error_correction::level_L, 23648+1) == std::nullopt);
        static_assert(best_version(error_correction::level_M, 18672+1) == std::nullopt);
        static_assert(best_version(error_correction::level_Q, 13328+1) == std::nullopt);
        static_assert(best_version(error_correction::level_H, 10208+1) == std::nullopt);
    }
}
#endif
