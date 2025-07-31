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

#include <ranges>
#include <concepts>

#include <qrcode/micro_qr/finder_pattern.h>
#include <qrcode/micro_qr/format_information.h>
#include <qrcode/micro_qr/separator_pattern.h>
#include <qrcode/micro_qr/symbol_version.h>
#include <qrcode/micro_qr/timing_pattern.h>

#include <qrcode/structure/matrix.h>
#include <qrcode/structure/module_traits.h>
#include <qrcode/structure/place_data.h>

namespace qrcode::micro_qr
{
    using qrcode::structure::matrix;

    template<class T, std::ranges::range Range>
    requires std::convertible_to<std::ranges::range_value_t<Range>, bool>
    [[nodiscard]] constexpr auto make_raw_code(symbol_version version, Range&& bits) noexcept
    {
        using qrcode::structure::module_traits;
        using qrcode::structure::place_data;

        auto code = matrix<T>{size(version), module_traits<T>::make_free()};

        place_finder_pattern(code);
        place_separator_pattern(code);
        place_timing_patterns(code);
        reserve_format_information(code);
        place_data(code, bits | std::views::transform(module_traits<T>::make_data));

        return code;
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>
#include <qrcode/structure/make_matrix.h>

namespace qrcode::micro_qr::test
{
    constexpr auto micro_qr_code_can_be_generated_from_given_version_and_error_level()
    {
        constexpr auto any_content = std::array<bool, 80>{{ 
            0,1,0,0,0,0,0,0, 0,0,0,1,1,0,0,0, 1,0,1,0,1,1,0,0, 1,1,0,0,0,0,1,1, 0,0,0,0,0,0,0,0, 
            1,0,0,0,0,1,1,0, 0,0,0,0,1,1,0,1, 0,0,1,0,0,0,1,0, 1,0,1,0,1,1,1,0, 0,0,1,1,0,0,0,0
        }}; // 01234567 (level L)
        constexpr auto any_version = symbol_version::M2;

        constexpr auto symbol = make_raw_code<char>(any_version, any_content);

        using qrcode::structure::make_matrix;
        using namespace std::literals;
        static_assert(symbol == make_matrix<char>({13,13},
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
        ));
    }
}
#endif