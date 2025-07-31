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
#include <qrcode/structure/matrix.h>

namespace qrcode::structure
{
    template<class T>
    concept less_comparable = std::relation<std::less<T>, T, T>;

    template<class T, std::ranges::range Masks, class Penalty_Score>
    requires less_comparable<std::invoke_result_t<Penalty_Score, matrix<T> const&>>
    [[nodiscard]] constexpr auto data_masking(
        matrix<T>& data, Masks&& masks, Penalty_Score&& penalty_score) noexcept
        -> std::ranges::borrowed_iterator_t<Masks>
    {
        auto const original = data;

        auto best_mask = begin(masks);
        data = (*best_mask)(original);
        auto best_score = penalty_score(data);

        for (auto mask = begin(masks)+1; mask != end(masks); ++mask)
        {
            auto const masked = (*mask)(original);
            auto const current_score = penalty_score(masked);

            if (current_score < best_score)
            {
                best_score = current_score;
                best_mask = mask;
                data = masked;
            }
        }
        return best_mask;
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::structure::test
{
    constexpr auto data_masking_applies_the_data_mask_to_given_matrix_derived_from_available_patterns_with_the_lowest_score()
    {
        auto f = []() constexpr
        {
            struct mask_stub
            {
                constexpr auto operator()(matrix<int> const&) const noexcept 
                { 
                    return matrix<int>{{type,1}}; 
                }

                int type = -1;
            };  

            constexpr auto mask_types = std::array<mask_stub, 3>{{{4}, {3}, {7}}};
            constexpr auto penalty_score = [](auto const& matrix){ return width(matrix); };

            auto any_matrix = matrix<int>{{10,10}};
            auto const used_mask = data_masking(any_matrix, mask_types, penalty_score);

            return any_matrix == matrix<int>{{3,1}} && used_mask->type == 3;
        };
        static_assert(f());
    }
}
#endif
