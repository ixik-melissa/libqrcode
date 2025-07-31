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

#include <qrcode/qr/error_correction.h>

#include <qrcode/code/format_encoding.h>

#include <qrcode/structure/dimension.h>
#include <qrcode/structure/matrix.h>
#include <qrcode/structure/module_traits.h>
#include <qrcode/structure/position.h>

namespace qrcode::qr::detail
{   
    using qrcode::structure::dimension;
    using qrcode::structure::position;
    using qrcode::structure::matrix;
    using qrcode::code::raw_format;
    using qrcode::code::error_correcting_code;

    [[nodiscard]] constexpr auto bch_code() noexcept
    {
        return error_correcting_code{15, 5, 0b101'0011'0111};
    }

    [[nodiscard]] constexpr auto format(error_correction error_level, int mask_type) noexcept
    {
        return raw_format{static_cast<std::uint16_t>(
            static_cast<std::uint8_t>(error_level) << 3 | 
            static_cast<std::uint8_t>(mask_type)
        )};
    }

    [[nodiscard]] constexpr auto format_positions(dimension symbol_size)
    {
        auto const hs = 8;
        auto const vs = 8;
        auto const w = width(symbol_size);
        auto const h = height(symbol_size);

        return std::array<std::pair<position,position>,15>{{
            {{hs,0},{w-1,vs}},
            {{hs,1},{w-2,vs}},
            {{hs,2},{w-3,vs}},
            {{hs,3},{w-4,vs}},
            {{hs,4},{w-5,vs}},
            {{hs,5},{w-6,vs}},
            {{hs,7},{w-7,vs}},
            {{hs,8},{w-8,vs}},
            //
            {{hs-1,8},{hs,h-vs+1}},
            {{hs-3,8},{hs,h-vs+2}},
            {{hs-4,8},{hs,h-vs+3}},
            {{hs-5,8},{hs,h-vs+4}},
            {{hs-6,8},{hs,h-vs+5}},
            {{hs-7,8},{hs,h-vs+6}},
            {{hs-8,8},{hs,h-vs+7}},
        }};
    }

    [[nodiscard]] constexpr auto dark_dot(dimension symbol_size)
    {
        return position{8, height(symbol_size)-8};
    }
}

namespace qrcode::qr
{
    using qrcode::structure::matrix;

    template<class T>
    [[nodiscard]] constexpr auto reserve_format_information(matrix<T>& matrix)
    {
        using qrcode::qr::detail::format_positions;
        using qrcode::qr::detail::dark_dot;
        using qrcode::structure::module_traits;

        auto make_function = module_traits<T>::make_function;

        for (auto i : format_positions(size(matrix)))
        {
            element_at(matrix, i.first) = make_function(0);
            element_at(matrix, i.second) = make_function(0);
        }
        element_at(matrix, dark_dot(size(matrix))) = make_function(1);
    }
    
    template<class T>
    [[nodiscard]] constexpr auto place_format_information(
        matrix<T>& matrix, error_correction error_level, int data_mask) noexcept
    {
        using qrcode::qr::detail::bch_code;
        using qrcode::qr::detail::format;
        using qrcode::qr::detail::dark_dot;
        using qrcode::qr::detail::format_positions;
        using qrcode::structure::module_traits;
        using qrcode::code::format_mask;

        auto const mask_pattern = format_mask{0b101'0100'0001'0010};
        auto bits = masked(encode(format(error_level, data_mask), bch_code()), mask_pattern).data;
        auto make_function = module_traits<T>::make_function;

        for (auto i : format_positions(size(matrix)))
        {
            element_at(matrix, i.first) = make_function(bits & 1);
            element_at(matrix, i.second) = make_function(bits & 1);
            bits >>= 1;
        }
        element_at(matrix, dark_dot(size(matrix))) = make_function(1);
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>
#include <qrcode/structure/make_matrix.h>

namespace qrcode::qr::detail::test
{
    constexpr auto format_information_consists_of_error_correction_and_data_mask_pattern_reference_for_qr_symbols()
    {        
        static_assert(format(error_correction::level_M, 0b000) == raw_format{0b00000});
        static_assert(format(error_correction::level_M, 0b001) == raw_format{0b00001});
        static_assert(format(error_correction::level_L, 0b001) == raw_format{0b01001});
        static_assert(format(error_correction::level_H, 0b010) == raw_format{0b10010});
        static_assert(format(error_correction::level_Q, 0b101) == raw_format{0b11101});
    }    
}

namespace qrcode::qr::test
{
    constexpr auto format_information_can_be_reserved_prior_to_placing_it_in_qr_symbols()
    {
        auto f = []
        {
            using namespace qrcode::structure;
            using namespace std::literals;
            auto any_matrix = matrix<char>{{21,21}, module_traits<char>::make_free()};

            reserve_format_information(any_matrix);
            
            return any_matrix == make_matrix<char>({21,21},
                "........-............"
                "........-............"
                "........-............"
                "........-............"
                "........-............"
                "........-............"
                "....................."
                "........-............"
                "------.--....--------"
                "....................."
                "....................."
                "....................."
                "....................."
                "........*............"
                "........-............"
                "........-............"
                "........-............"
                "........-............"
                "........-............"
                "........-............"
                "........-............"sv
            );
        };
        static_assert(f());
    }

    constexpr auto format_information_is_placed_top_left_and_bottom_right_in_qr_symbols()
    {
        auto f = []
        {
            using namespace qrcode::structure;
            using namespace std::literals;
            constexpr auto any_error_level = error_correction::level_M;
            constexpr auto any_data_mask = 0b101;

            auto any_matrix = matrix<char>{{21,21}, module_traits<char>::make_free()};

            place_format_information(any_matrix, any_error_level, any_data_mask);

            return any_matrix == make_matrix<char>({21,21},
                "........-............"
                "........*............"
                "........*............"
                "........*............"
                "........-............"
                "........-............"
                "....................."
                "........*............"
                "*-----.-*....**--***-"
                "....................."
                "....................."
                "....................."
                "....................."
                "........*............"
                "........-............"
                "........-............"
                "........-............"
                "........-............"
                "........-............"
                "........-............"
                "........*............"sv
            );
        };
        static_assert(f());
    }
}
#endif