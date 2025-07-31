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

#include <qrcode/qr/adjacent_score.h>
#include <qrcode/qr/same_color_score.h>
#include <qrcode/qr/finder_like_score.h>
#include <qrcode/qr/dark_module_score.h>

namespace qrcode::qr
{
    template<class T>
    [[nodiscard]] constexpr auto penalty_score(matrix<T> const& matrix)
    {
        return 
            adjacent_score(matrix)
            + same_color_score(matrix)
            + finder_like_score(matrix)
            + dark_module_score(matrix);
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>
#include <qrcode/structure/make_matrix.h>

namespace qrcode::qr::test
{
    constexpr auto penalty_scores_consist_of_adjacent_score_and_same_color_score_and_finder_like_score_and_dark_module_score()
    {
        using qrcode::structure::make_matrix;
        using namespace std::literals;

        constexpr auto any_matrix = make_matrix<char>({21,21},
            "*******.**....*******"   
            "*.....*.*..*..*.....*"   
            "*.***.*.*..**.*.***.*"   
            "*.***.*.*.....*.***.*"   
            "*.***.*.*.*...*.***.*"   
            "*.....*...*...*.....*"   
            "*******.*.*.*.*******"   
            "........*............"   
            ".**.*.**....*.*.*****"   
            ".*......****....*...*"   
            "..**.***.**...*.**..."   
            ".**.**.*..**.*.*.***."   
            "*...*.*.*.***.***.*.*"   
            "........**.*..*...*.*"   
            "*******.*.*....*.**.."   
            "*.....*..*.**.**.*..."   
            "*.***.*.*.*...*******"   
            "*.***.*..*.*.*.*...*."   
            "*.***.*.*...****.*..*"   
            "*.....*.*.**.*...*.**"   
            "*******.....****....*"sv
        );

        constexpr auto s1 = adjacent_score(any_matrix);
        constexpr auto s2 = same_color_score(any_matrix);
        constexpr auto s3 = finder_like_score(any_matrix);
        constexpr auto s4 = dark_module_score(any_matrix);

        static_assert(s1 == 180);
        static_assert(s2 == 90);
        static_assert(s3 == 80);
        static_assert(s4 == 0);
        static_assert(penalty_score(any_matrix) == (s1+s2+s3+s4));
    }
}
#endif