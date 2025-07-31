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

#include <cx/cmath.h>

#include <qrcode/qr/finder_pattern.h>
#include <qrcode/structure/cartesian_product_view.h>
#include <qrcode/structure/dimension.h>
#include <qrcode/structure/horizontal_view.h>
#include <qrcode/structure/module_traits.h>

namespace qrcode::qr::detail
{
    using qrcode::structure::dimension;

    [[nodiscard]] constexpr auto alignment_count(dimension symbol_size) noexcept
    {
        auto const symbol_width = width(symbol_size);
        if (symbol_width == 21)
            return 0;
        return 2 + (symbol_width - 17) / 28;
    }
    
    [[nodiscard]] constexpr auto alignment_step(dimension symbol_size) noexcept
    {
        auto const symbol_width = width(symbol_size);
        if (symbol_width == 145)
            return 26;

        auto const available_space = symbol_width - 14;
        
        auto const fractional_step = available_space / (alignment_count(symbol_size)-1.f);
        auto const next_larger_even_step = static_cast<int>(cx::ceil(fractional_step/2) * 2);
        
        return next_larger_even_step;
    }
    
    [[nodiscard]] constexpr auto alignment_coordinates(dimension symbol_size)
    {
        auto const count = alignment_count(symbol_size);
        auto const step = alignment_step(symbol_size);
        auto const start = width(symbol_size) - 7 - (count-1)*step;

        auto index = std::views::iota(0, count);

        auto coordinates = std::views::transform([start, step](auto index)
        {
            // the first is always six
            if (index == 0)
                return 6;

            return start + step*index;    
        });

        return index | coordinates;
    }

    [[nodiscard]] constexpr auto alignment_positions(dimension symbol_size)
    {
        auto coordinates = alignment_coordinates(symbol_size);
        auto const places = finder_locations(symbol_size);
        auto const finder = size(finder_pattern());

        auto not_near_finder = [places, finder](auto point)
        {
            for (auto i : places)
            {
                auto const distance = dimension{cx::abs(point.x - i.x), cx::abs(point.y - i.y)};
                if (width(distance) < width(finder) && height(distance) < height(finder))
                {
                    return false;
                }
            }
            return true;
        };
        
        return 
            views::cartesian_product(coordinates, coordinates) 
            | std::views::filter(not_near_finder);
    }

    class alignment_pattern
    {
        static constexpr auto extend = dimension{5,5};

    public:
        [[nodiscard]] constexpr auto size() const noexcept
        {
            return extend;
        }

        template<class T>
        [[nodiscard]] constexpr auto place(matrix<T>& matrix, position top_left) const noexcept
        {
            constexpr auto pattern = std::array<bool,width(extend)*height(extend)>{
                1,1,1,1,1,
                1,0,0,0,1,
                1,0,1,0,1,
                1,0,0,0,1,
                1,1,1,1,1
            };
            auto const bits = pattern | std::views::transform(module_traits<T>::make_function);

            using std::ranges::begin;
            auto range = views::horizontal(top_left, extend) | views::element(matrix);
            std::ranges::copy(bits, begin(range));
        }
    };
}

namespace qrcode::qr
{
    using qrcode::structure::matrix;

    template<class T>
    [[nodiscard]] constexpr auto place_alignment_patterns(matrix<T>& matrix)
    {
        using qrcode::qr::detail::alignment_pattern;
        using qrcode::qr::detail::alignment_positions;

        auto const pattern = alignment_pattern();
        constexpr auto offset = dimension{width(size(pattern))/2, height(size(pattern))/2};
        auto translate = std::views::transform([offset](auto center)
        { 
            return position{center.x - width(offset), center.y - height(offset)}; 
        });

        auto top_lefts = alignment_positions(size(matrix)) | translate;
        for (auto i : top_lefts)
            pattern.place(matrix, i);
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>
#include <qrcode/structure/make_matrix.h>

namespace qrcode::qr::detail::test
{
    constexpr auto alignment_count_returns_number_of_alignment_patterns_for_given_symbol_qr_version()
    {
        static_assert(alignment_count(dimension{21,21}) == 0);

        static_assert(alignment_count(dimension{25,25}) == 2);
        static_assert(alignment_count(dimension{29,29}) == 2);
        static_assert(alignment_count(dimension{33,33}) == 2);
        static_assert(alignment_count(dimension{37,37}) == 2);
        static_assert(alignment_count(dimension{41,41}) == 2);   
        
        static_assert(alignment_count(dimension{45,45}) == 3);
        static_assert(alignment_count(dimension{49,49}) == 3);
        static_assert(alignment_count(dimension{53,53}) == 3);
        static_assert(alignment_count(dimension{57,57}) == 3);
        static_assert(alignment_count(dimension{61,61}) == 3);
        static_assert(alignment_count(dimension{65,65}) == 3);
        static_assert(alignment_count(dimension{69,69}) == 3); 
        
        static_assert(alignment_count(dimension{73,73}) == 4);
        static_assert(alignment_count(dimension{77,77}) == 4);
        static_assert(alignment_count(dimension{81,81}) == 4);
        static_assert(alignment_count(dimension{85,85}) == 4);
        static_assert(alignment_count(dimension{89,89}) == 4);
        static_assert(alignment_count(dimension{93,93}) == 4);
        static_assert(alignment_count(dimension{97,97}) == 4);   
        
        static_assert(alignment_count(dimension{101,101}) == 5);
        static_assert(alignment_count(dimension{105,105}) == 5);
        static_assert(alignment_count(dimension{109,109}) == 5);
        static_assert(alignment_count(dimension{113,113}) == 5);
        static_assert(alignment_count(dimension{117,117}) == 5);
        static_assert(alignment_count(dimension{121,121}) == 5);
        static_assert(alignment_count(dimension{125,125}) == 5);
        
        static_assert(alignment_count(dimension{129,129}) == 6);
        static_assert(alignment_count(dimension{133,133}) == 6);
        static_assert(alignment_count(dimension{137,137}) == 6);
        static_assert(alignment_count(dimension{141,141}) == 6);
        static_assert(alignment_count(dimension{145,145}) == 6);
        static_assert(alignment_count(dimension{149,149}) == 6);
        static_assert(alignment_count(dimension{153,153}) == 6);
    
        static_assert(alignment_count(dimension{157,157}) == 7);
        static_assert(alignment_count(dimension{161,161}) == 7);
        static_assert(alignment_count(dimension{165,165}) == 7);
        static_assert(alignment_count(dimension{169,169}) == 7);
        static_assert(alignment_count(dimension{173,173}) == 7);
        static_assert(alignment_count(dimension{177,177}) == 7);
    }

    constexpr auto alignment_coordinates_returns_all_row_column_coordinates_of_center_module_of_alignment_patterns()
    {
        static_assert(std::ranges::equal(alignment_coordinates(dimension{21,21}), std::array<int,0>{}));

        static_assert(std::ranges::equal(alignment_coordinates({25,25}), std::array{6,18}));
        static_assert(std::ranges::equal(alignment_coordinates({29,29}), std::array{6,22}));
        static_assert(std::ranges::equal(alignment_coordinates({33,33}), std::array{6,26}));
        static_assert(std::ranges::equal(alignment_coordinates({37,37}), std::array{6,30}));
        static_assert(std::ranges::equal(alignment_coordinates({41,41}), std::array{6,34}));
        
        static_assert(std::ranges::equal(alignment_coordinates({45,45}), std::array{6,22,38}));
        static_assert(std::ranges::equal(alignment_coordinates({49,49}), std::array{6,24,42}));
        static_assert(std::ranges::equal(alignment_coordinates({53,53}), std::array{6,26,46}));
        static_assert(std::ranges::equal(alignment_coordinates({57,57}), std::array{6,28,50}));
        static_assert(std::ranges::equal(alignment_coordinates({61,61}), std::array{6,30,54}));
        static_assert(std::ranges::equal(alignment_coordinates({65,65}), std::array{6,32,58}));
        static_assert(std::ranges::equal(alignment_coordinates({69,69}), std::array{6,34,62}));
        
        static_assert(std::ranges::equal(alignment_coordinates({73,73}), std::array{6,26,46,66}));
        static_assert(std::ranges::equal(alignment_coordinates({77,77}), std::array{6,26,48,70}));
        static_assert(std::ranges::equal(alignment_coordinates({81,81}), std::array{6,26,50,74}));
        static_assert(std::ranges::equal(alignment_coordinates({85,85}), std::array{6,30,54,78}));
        static_assert(std::ranges::equal(alignment_coordinates({89,89}), std::array{6,30,56,82}));
        static_assert(std::ranges::equal(alignment_coordinates({93,93}), std::array{6,30,58,86}));
        static_assert(std::ranges::equal(alignment_coordinates({97,97}), std::array{6,34,62,90}));
        
        static_assert(std::ranges::equal(alignment_coordinates({101,101}), std::array{6,28,50,72,94}));
        static_assert(std::ranges::equal(alignment_coordinates({105,105}), std::array{6,26,50,74,98}));
        static_assert(std::ranges::equal(alignment_coordinates({109,109}), std::array{6,30,54,78,102}));
        static_assert(std::ranges::equal(alignment_coordinates({113,113}), std::array{6,28,54,80,106}));
        static_assert(std::ranges::equal(alignment_coordinates({117,117}), std::array{6,32,58,84,110}));
        static_assert(std::ranges::equal(alignment_coordinates({121,121}), std::array{6,30,58,86,114}));
        static_assert(std::ranges::equal(alignment_coordinates({125,125}), std::array{6,34,62,90,118}));
        
        static_assert(std::ranges::equal(alignment_coordinates({129,129}), std::array{6,26,50,74,98,122}));
        static_assert(std::ranges::equal(alignment_coordinates({133,133}), std::array{6,30,54,78,102,126}));
        static_assert(std::ranges::equal(alignment_coordinates({137,137}), std::array{6,26,52,78,104,130}));
        static_assert(std::ranges::equal(alignment_coordinates({141,141}), std::array{6,30,56,82,108,134}));
        static_assert(std::ranges::equal(alignment_coordinates({145,145}), std::array{6,34,60,86,112,138}));
        static_assert(std::ranges::equal(alignment_coordinates({149,149}), std::array{6,30,58,86,114,142}));
        static_assert(std::ranges::equal(alignment_coordinates({153,153}), std::array{6,34,62,90,118,146}));

        static_assert(std::ranges::equal(alignment_coordinates({157,157}), std::array{6,30,54,78,102,126,150}));
        static_assert(std::ranges::equal(alignment_coordinates({161,161}), std::array{6,24,50,76,102,128,154}));
        static_assert(std::ranges::equal(alignment_coordinates({165,165}), std::array{6,28,54,80,106,132,158}));
        static_assert(std::ranges::equal(alignment_coordinates({169,169}), std::array{6,32,58,84,110,136,162}));
        static_assert(std::ranges::equal(alignment_coordinates({173,173}), std::array{6,26,54,82,110,138,166}));
        static_assert(std::ranges::equal(alignment_coordinates({177,177}), std::array{6,30,58,86,114,142,170}));
    }

    constexpr auto alignment_positions_returns_all_positions_of_center_modules_of_alignment_patterns()
    {
        static_assert(std::ranges::equal(alignment_positions(dimension{21,21}), std::array<position,0>{}));
        static_assert(std::ranges::equal(alignment_positions(dimension{25,25}), std::array{position{18,18}}));
        static_assert(std::ranges::equal(alignment_positions(
            dimension{45,45}), std::array<position,6>{{{22,6},{6,22},{22,22},{38,22},{22,38},{38,38}}}));   
    }

    constexpr auto alignment_patterns_are_placed_placed_at_predefined_locations_of_qr_version_depending_on_symbol_qr_version()
    {
        auto f = []
        {
            auto any_matrix = matrix<char>{{25,25}, module_traits<char>::make_free()};
            
            place_alignment_patterns(any_matrix);
            
            using namespace qrcode::structure;
            using namespace std::literals;
            return any_matrix == make_matrix<char>({25,25},
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."
                "........................."
                "................*****...."
                "................*---*...."
                "................*-*-*...."
                "................*---*...."
                "................*****...."
                "........................."
                "........................."
                "........................."
                "........................."sv
            );
        };
        static_assert(f());
    }
}
#endif