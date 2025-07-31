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

#include <qrcode/symbol.h>

#include <qrcode/qr/alignment_pattern.h>
#include <qrcode/qr/finder_pattern.h>
#include <qrcode/qr/format_information.h>
#include <qrcode/qr/separator_pattern.h>
#include <qrcode/qr/symbol_version.h>
#include <qrcode/qr/timing_pattern.h>
#include <qrcode/qr/version_information.h>

#include <qrcode/structure/matrix.h>
#include <qrcode/structure/module_traits.h>
#include <qrcode/structure/place_data.h>

namespace qrcode::qr
{
    using qrcode::structure::matrix;

    template<class T, std::ranges::range Range>
    requires std::convertible_to<std::ranges::range_value_t<Range>, bool>
    [[nodiscard]] constexpr auto make_raw_code(symbol_version version, Range&& bits) noexcept
    {
        using qrcode::structure::module_traits;
        using qrcode::structure::place_data;

        auto code = matrix<T>{size(version), module_traits<T>::make_free()};

        place_finder_patterns(code);
        place_separator_patterns(code);
        place_timing_patterns(code);
        place_alignment_patterns(code);
        place_version_information(code, version);
        reserve_format_information(code);
        place_data(code, bits | std::views::transform(module_traits<T>::make_data));

        return code;
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>
#include <qrcode/structure/make_matrix.h>

namespace qrcode::qr
{
    constexpr auto qr_codes_can_be_generated_from_given_version_and_content_bits()
    {
        using qrcode::structure::make_matrix;
        using namespace std::literals;

        constexpr auto any_content = std::array<bool,208>{{
            0,0,0,1,0,0,0,0, 0,0,1,0,0,0,0,0, 0,0,0,0,1,1,0,0, 0,1,0,1,0,1,1,0, 0,1,1,0,0,0,0,1, 
            1,0,0,0,0,0,0,0, 1,1,1,0,1,1,0,0, 0,0,0,1,0,0,0,1, 1,1,1,0,1,1,0,0, 0,0,0,1,0,0,0,1, 
            1,1,1,0,1,1,0,0, 0,0,0,1,0,0,0,1, 1,1,1,0,1,1,0,0, 0,0,0,1,0,0,0,1, 1,1,1,0,1,1,0,0, 
            0,0,0,1,0,0,0,1, 1,0,1,0,0,1,0,1, 0,0,1,0,0,1,0,0, 1,1,0,1,0,1,0,0, 1,1,0,0,0,0,0,1, 
            1,1,1,0,1,1,0,1, 0,0,1,1,0,1,1,0, 1,1,0,0,0,1,1,1, 1,0,0,0,0,1,1,1, 0,0,1,0,1,1,0,0, 
            0,1,0,1,0,1,0,1
        }}; // 01234567
        constexpr auto any_version = symbol_version{1};
        constexpr auto code = make_raw_code<char>(any_version, any_content);

        static_assert(code == make_matrix<char>({21,21},
            "*******--,,+,-*******"
            "*-----*--,++,-*-----*"
            "*-***-*--+,,+-*-***-*"
            "*-***-*--,,,+-*-***-*"
            "*-***-*--+++,-*-***-*"
            "*-----*--+,,,-*-----*"
            "*******-*-*-*-*******"
            "---------+,+,--------"
            "------*--,,,,--------"
            "+,,,,+-++++,,,,,,+,,,"
            "+,++,,*+,,,+++,+++,++"
            "+,,++,-,,,,,+,,,++,,,"
            "+,,,++*+++,+++,++,+,,"
            "--------*+++,+++,+,,,"
            "*******--,+,,,+,,,+,,"
            "*-----*--+++,+++,,,,+"
            "*-***-*--+,,,,,,,+,,,"
            "*-***-*--,,,,,,,,,+,,"
            "*-***-*--+++++,++,,,,"
            "*-----*--+,,+,,,+,,+,"
            "*******--,++++,++,,,,"sv
        ));
    }
}
#endif
