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

#include <array>
#include <utility>

#include <qrcode/micro_qr/symbol_designator.h>

namespace qrcode::micro_qr
{
    [[nodiscard]] constexpr auto data_capacities() noexcept
    {
        constexpr auto configurations = std::array{
            std::pair{*make_designator(symbol_version::M1, std::nullopt), 20},

            std::pair{*make_designator(symbol_version::M2, error_correction::level_L), 40},
            std::pair{*make_designator(symbol_version::M2, error_correction::level_M), 32},
            
            std::pair{*make_designator(symbol_version::M3, error_correction::level_L), 84},
            std::pair{*make_designator(symbol_version::M3, error_correction::level_M), 68},
            
            std::pair{*make_designator(symbol_version::M4, error_correction::level_L), 128},
            std::pair{*make_designator(symbol_version::M4, error_correction::level_M), 112},
            std::pair{*make_designator(symbol_version::M4, error_correction::level_Q), 80},
        };
        return configurations;
    }

    [[nodiscard]] constexpr auto total_data_bits(symbol_designator const& designator) noexcept
    {
        constexpr auto configurations = data_capacities();
        auto const found = std::ranges::find_if(
            configurations, [=](auto i){ return i.first == designator; }
        );
        return found != end(configurations) ? found->second : 0;
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::micro_qr::test
{
    constexpr auto total_data_bits_returns_the_numbers_of_data_bits_available_for_given_symbol_version_and_error_correction_level()
    {
        static_assert(total_data_bits(*make_designator(symbol_version::M1, std::nullopt)) == 20);

        static_assert(total_data_bits(*make_designator(symbol_version::M2, error_correction::level_L)) == 40);
        static_assert(total_data_bits(*make_designator(symbol_version::M2, error_correction::level_M)) == 32);

        static_assert(total_data_bits(*make_designator(symbol_version::M3, error_correction::level_L)) == 84);
        static_assert(total_data_bits(*make_designator(symbol_version::M3, error_correction::level_M)) == 68);

        static_assert(total_data_bits(*make_designator(symbol_version::M4, error_correction::level_L)) == 128);
        static_assert(total_data_bits(*make_designator(symbol_version::M4, error_correction::level_M)) == 112);
        static_assert(total_data_bits(*make_designator(symbol_version::M4, error_correction::level_Q)) == 80);
    }
}
#endif