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
    [[nodiscard]] constexpr auto generator_degree(symbol_designator const& designator) noexcept
    {
        constexpr auto configurations = std::array{
            std::pair{*make_designator(symbol_version::M1, std::nullopt), 2},

            std::pair{*make_designator(symbol_version::M2, error_correction::level_L), 5},
            std::pair{*make_designator(symbol_version::M2, error_correction::level_M), 6},
            
            std::pair{*make_designator(symbol_version::M3, error_correction::level_L), 6},
            std::pair{*make_designator(symbol_version::M3, error_correction::level_M), 8},
            
            std::pair{*make_designator(symbol_version::M4, error_correction::level_L), 8},
            std::pair{*make_designator(symbol_version::M4, error_correction::level_M), 10},
            std::pair{*make_designator(symbol_version::M4, error_correction::level_Q), 14},
        };
        
        auto const found = std::ranges::find_if(
            configurations, [=](auto i){ return i.first == designator; }
        );

        return found != end(configurations) ? found->second : 0;
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::micro_qr::test
{
    constexpr auto generator_degree_returns_degree_of_error_correction_polynomial_depending_on_micro_qr_symbol_version_and_error_correction()
    {
        static_assert(generator_degree(*make_designator(symbol_version::M1, std::nullopt)) == 2);

        static_assert(generator_degree(*make_designator(symbol_version::M2, error_correction::level_L)) == 5);
        static_assert(generator_degree(*make_designator(symbol_version::M2, error_correction::level_M)) == 6);

        static_assert(generator_degree(*make_designator(symbol_version::M3, error_correction::level_L)) == 6);
        static_assert(generator_degree(*make_designator(symbol_version::M3, error_correction::level_M)) == 8);

        static_assert(generator_degree(*make_designator(symbol_version::M4, error_correction::level_L)) == 8);
        static_assert(generator_degree(*make_designator(symbol_version::M4, error_correction::level_M)) == 10);
        static_assert(generator_degree(*make_designator(symbol_version::M4, error_correction::level_Q)) == 14);
    }
}
#endif
