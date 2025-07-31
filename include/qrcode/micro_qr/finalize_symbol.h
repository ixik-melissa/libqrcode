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

#include <concepts>
#include <ranges>

#include <qrcode/symbol.h>

#include <qrcode/micro_qr/symbol_designator.h>
#include <qrcode/micro_qr/error_correction.h>
#include <qrcode/micro_qr/format_information.h>
#include <qrcode/micro_qr/mask_pattern.h>
#include <qrcode/micro_qr/penalty_score.h>

#include <qrcode/structure/data_masking.h>
#include <qrcode/structure/matrix.h>

namespace qrcode::micro_qr
{
    using qrcode::structure::matrix;

    template<class T>
    [[nodiscard]] constexpr auto finalize(matrix<T> code, symbol_designator designator) noexcept
    {
        auto const masks = available_masks();
        auto const score = [](matrix<T> const& code) { return penalty_score(code); };
        auto const mask_id = static_cast<int>(data_masking(code, masks, score) - begin(masks));
        place_format_information(code, designator, mask_id);
        return symbol{std::move(designator), mask_id, std::move(code)};
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>
#include <qrcode/structure/make_matrix.h>

namespace qrcode::micro_qr
{
    constexpr auto finalize_determines_the_best_mask_and_stores_its_id_in_given_symbol()
    {
        using qrcode::structure::make_matrix;
        using namespace std::literals;
        constexpr auto any_designator = *make_designator(symbol_version::M2, error_correction::level_L);
        constexpr auto any_unmasked = make_matrix<char>({13,13},
            "*******-*-*-*"
            "*-----*--++,,"
            "*-***-*--,,++"
            "*-***-*--,,,+"
            "*-***-*--++,+"
            "*-----*--,,,,"
            "*******--,,,+"
            "---------,,+,"
            "*--------,,,,"
            "-,,,+,,+,,+,,"
            "*++++,+++,,,,"
            "-,,,+,,,,+,,,"
            "*,,,+,+,,,++,"sv
        );
        constexpr auto selected_mask_id = 1;

        static_assert(finalize(any_unmasked, any_designator) == symbol{
            any_designator, selected_mask_id,
            make_matrix<char>({13,13},
                "*******-*-*-*"
                "*-----*-*++,+"
                "*-***-*--++,+"
                "*-***-*--++++"
                "*-***-*-*++,,"
                "*-----*-*,,,+"
                "*******--++++"
                "---------++,,"
                "**-*----*,,,+"
                "-++,+,+,+,+,+"
                "*++,,+++++++,"
                "-,,+,+,,,,++,"
                "*++,+,,++,+++"sv
            )
        });
    }
}
#endif
