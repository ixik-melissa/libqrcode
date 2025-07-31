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

#include <qrcode/qr/symbol_version.h>

#include <qrcode/code/error_correction_code.h>
#include <qrcode/code/lfsr.h>

#include <qrcode/structure/element_view.h>
#include <qrcode/structure/horizontal_view.h>
#include <qrcode/structure/matrix.h>
#include <qrcode/structure/module_traits.h>
#include <qrcode/structure/vertical_view.h>

namespace qrcode::qr::detail
{   
    using qrcode::structure::position;
    using qrcode::structure::dimension;
    using qrcode::code::error_correcting_code;

    [[nodiscard]] constexpr auto golay_code() noexcept
    {
        return error_correcting_code{18, 6, 0b1'1111'0010'0101};
    }
    
    [[nodiscard]] constexpr auto version_information_bits(symbol_version version) noexcept
    {
        using code::lfsr;

        constexpr auto code = golay_code();
        constexpr auto power = code.total_size - code.data_size;

        auto division_remainder = lfsr{
            code.generator, static_cast<decltype(code.generator)>(version.number)};
        
        for (auto i = 0; i != power; ++i)
            division_remainder.advance(0);
            
        return version.number << power | state(division_remainder);
    }

    [[nodiscard]] constexpr auto horizontal_version_information(dimension symbol_size) noexcept
    {
        constexpr auto version = 3;
        constexpr auto finder = 7;
        constexpr auto separator = 1;

        return std::make_pair(
            position{width(symbol_size) - finder - separator - version, 0},
            dimension{3,6}
        );
    }

    [[nodiscard]] constexpr auto vertical_version_information(dimension symbol_size) noexcept
    {
        constexpr auto version = 3;
        constexpr auto finder = 7;
        constexpr auto separator = 1;

        return std::make_pair(
            position{0, height(symbol_size) - finder - separator - version},
            dimension{6,3}
        );
    }
}

namespace qrcode::qr
{
    using qrcode::structure::matrix;

    template<class T>
    [[nodiscard]] constexpr auto place_version_information(
        matrix<T>& matrix, symbol_version version)
    {
        using std::ranges::begin;
        using std::ranges::end;
        using qrcode::qr::detail::version_information_bits;
        using qrcode::qr::detail::vertical_version_information;
        using qrcode::qr::detail::horizontal_version_information;
        
        if (version.number < 7)
            return;
    
        auto const [horizontal_start, horizontal_size]=horizontal_version_information(size(matrix));
        auto const [vertical_start, vertical_size] = vertical_version_information(size(matrix));
        
        auto const bits = version_information_bits(version);

        auto range1 = views::horizontal(horizontal_start, horizontal_size) | views::element(matrix);
        auto range2 = views::vertical(vertical_start, vertical_size) | views::element(matrix);

        auto mask = 0b1;
        auto make_function = module_traits<T>::make_function;
        auto i = begin(range1);
        auto j = begin(range2);
        for (; i != end(range1); ++i, ++j, mask <<= 1)
        {
            auto const function_module = make_function(bits & mask);
            *i = function_module;
            *j = function_module;
        }
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>
#include <qrcode/structure/make_matrix.h>

namespace qrcode::qr::detail::test
{
    constexpr auto version_information_bits_returns_the_version_information_as_bits_encoded_as_integer()
    {
        static_assert(version_information_bits(symbol_version{7}) == 0b00'0111'1100'1001'0100);
        static_assert(version_information_bits(symbol_version{8}) == 0b00'1000'0101'1011'1100);
        static_assert(version_information_bits(symbol_version{9}) == 0b00'1001'1010'1001'1001);
        static_assert(version_information_bits(symbol_version{10}) == 0b00'1010'0100'1101'0011);
        static_assert(version_information_bits(symbol_version{11}) == 0b00'1011'1011'1111'0110);
        static_assert(version_information_bits(symbol_version{12}) == 0b00'1100'0111'0110'0010);
        static_assert(version_information_bits(symbol_version{13}) == 0b00'1101'1000'0100'0111);
        static_assert(version_information_bits(symbol_version{14}) == 0b00'1110'0110'0000'1101);
        static_assert(version_information_bits(symbol_version{15}) == 0b00'1111'1001'0010'1000);
        static_assert(version_information_bits(symbol_version{16}) == 0b01'0000'1011'0111'1000);
        static_assert(version_information_bits(symbol_version{17}) == 0b01'0001'0100'0101'1101);
        static_assert(version_information_bits(symbol_version{18}) == 0b01'0010'1010'0001'0111);
        static_assert(version_information_bits(symbol_version{19}) == 0b01'0011'0101'0011'0010);
        static_assert(version_information_bits(symbol_version{20}) == 0b01'0100'1001'1010'0110);
        static_assert(version_information_bits(symbol_version{21}) == 0b01'0101'0110'1000'0011);
        static_assert(version_information_bits(symbol_version{22}) == 0b01'0110'1000'1100'1001);
        static_assert(version_information_bits(symbol_version{23}) == 0b01'0111'0111'1110'1100);
        static_assert(version_information_bits(symbol_version{24}) == 0b01'1000'1110'1100'0100);
        static_assert(version_information_bits(symbol_version{25}) == 0b01'1001'0001'1110'0001);
        static_assert(version_information_bits(symbol_version{26}) == 0b01'1010'1111'1010'1011);
        static_assert(version_information_bits(symbol_version{27}) == 0b01'1011'0000'1000'1110);
        static_assert(version_information_bits(symbol_version{28}) == 0b01'1100'1100'0001'1010);
        static_assert(version_information_bits(symbol_version{29}) == 0b01'1101'0011'0011'1111);
        static_assert(version_information_bits(symbol_version{30}) == 0b01'1110'1101'0111'0101);
        static_assert(version_information_bits(symbol_version{31}) == 0b01'1111'0010'0101'0000);
        static_assert(version_information_bits(symbol_version{32}) == 0b10'0000'1001'1101'0101);
        static_assert(version_information_bits(symbol_version{33}) == 0b10'0001'0110'1111'0000);
        static_assert(version_information_bits(symbol_version{34}) == 0b10'0010'1000'1011'1010);
        static_assert(version_information_bits(symbol_version{35}) == 0b10'0011'0111'1001'1111);
        static_assert(version_information_bits(symbol_version{36}) == 0b10'0100'1011'0000'1011);
        static_assert(version_information_bits(symbol_version{37}) == 0b10'0101'0100'0010'1110);
        static_assert(version_information_bits(symbol_version{38}) == 0b10'0110'1010'0110'0100);
        static_assert(version_information_bits(symbol_version{39}) == 0b10'0111'0101'0100'0001);
        static_assert(version_information_bits(symbol_version{40}) == 0b10'1000'1100'0110'1001);  
    }

    constexpr auto horizontal_version_information_returns_the_start_position_of_the_version_information_located_top_right()
    {
        using pair = std::pair<position, dimension>;

        static_assert(horizontal_version_information(size(symbol_version{7})) == pair{{34,0}, {3,6}});
        static_assert(horizontal_version_information(size(symbol_version{8})) == pair{{38,0}, {3,6}});
        static_assert(horizontal_version_information(size(symbol_version{9})) == pair{{42,0}, {3,6}});
        static_assert(horizontal_version_information(size(symbol_version{10})) == pair{{46,0}, {3,6}});
        static_assert(horizontal_version_information(size(symbol_version{11})) == pair{{50,0}, {3,6}});
    }

    constexpr auto vertical_version_information_returns_the_start_position_of_the_version_information_located_bottom_left()
    {
        using pair = std::pair<position, dimension>;

        static_assert(vertical_version_information(size(symbol_version{7})) == pair{{0,34}, {6,3}});
        static_assert(vertical_version_information(size(symbol_version{8})) == pair{{0,38}, {6,3}});
        static_assert(vertical_version_information(size(symbol_version{9})) == pair{{0,42}, {6,3}});
        static_assert(vertical_version_information(size(symbol_version{10})) == pair{{0,46}, {6,3}});
        static_assert(vertical_version_information(size(symbol_version{11})) == pair{{0,50}, {6,3}});
    }
}

namespace qrcode::qr::test
{
    constexpr auto version_information_is_not_placed_if_version_lower_that_seven()
    {
        auto f = [](auto version)
        {
            using namespace qrcode::structure;
            auto any_matrix = matrix<char>{size(version), module_traits<char>::make_free()};

            place_version_information(any_matrix, version);

            return any_matrix == matrix<char>{size(version), module_traits<char>::make_free()};
        };
        static_assert(f(symbol_version{0}));
        static_assert(f(symbol_version{1}));
        static_assert(f(symbol_version{2}));
        static_assert(f(symbol_version{3}));
        static_assert(f(symbol_version{4}));
        static_assert(f(symbol_version{5}));
        static_assert(f(symbol_version{6}));
    }

    constexpr auto version_information_is_placed_top_right_next_and_bottom_left_next_to_finder_pattern()
    {        
        auto f = []
        {
            using namespace qrcode::structure;
            using namespace std::literals;
            constexpr auto any_version = symbol_version{7};
            auto any_matrix = matrix<char>{size(any_version), module_traits<char>::make_free()};

            place_version_information(any_matrix, any_version);

            return any_matrix == make_matrix<char>({45,45},
                "..................................--*........"
                "..................................-*-........"
                "..................................-*-........"
                "..................................-**........"
                "..................................***........"
                "..................................---........"
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "----*-......................................."
                "-****-......................................."
                "*--**-......................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."
                "............................................."sv
            );
        };
        static_assert(f());
    }
}
#endif
