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

#include <qrcode/structure/horizontal_view.h>
#include <qrcode/structure/matrix.h>
#include <qrcode/structure/module_traits.h>

namespace qrcode::structure::detail
{
    template<int...Is> struct indices {};
    template<int N, int...Is> struct gen_indices : gen_indices<N-1, N-1, Is...> {};
    template<int...Is> struct gen_indices<0, Is...> : indices<Is...> {};

    template<class T, std::size_t N, class F, int...Is>
    constexpr auto do_transformed_array(std::array<T,N> const& values, indices<Is...>, F f) noexcept 
    { 
        using result_type = std::invoke_result_t<F, T const&>;
        return std::array<result_type, sizeof...(Is)>{f(values[Is])...}; 
    }

    template<class T, std::size_t N, class F>
    constexpr auto transformed_array(std::array<T,N> const& values, F f) noexcept 
    { 
        return do_transformed_array(values, gen_indices<N>{}, f); 
    }
}

namespace qrcode::structure
{
    template<class Mask>
    requires std::convertible_to<std::invoke_result_t<Mask, int, int>, bool>
    [[nodiscard]] constexpr auto apply_mask_functor(Mask mask) noexcept
    {
        return [mask]<class T>(matrix<T> matrix)
        {
            auto is_data = module_traits<T>::is_data;
            auto apply_mask = module_traits<T>::apply_mask;
            for (auto i : views::horizontal({0,0}, size(matrix)))
            {
                auto& module = element_at(matrix, i);
                if (is_data(module))
                    module = apply_mask(module, mask(i.y, i.x)); //sic!: y and x are swapped!
            }
            return matrix;
        };
    }

    template<std::size_t N, class Mask>
    [[nodiscard]] constexpr auto apply_mask_array(std::array<Mask,N> const& masks) noexcept
    {
        using qrcode::structure::detail::transformed_array;
        auto f = apply_mask_functor<Mask>;
        return transformed_array(masks, f);
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>
#include <qrcode/structure/make_matrix.h>

namespace qrcode::structure::test
{
    constexpr auto apply_mask_returns_a_callable_which_wraps_the_given_simple_boolean_mask_to_mask_any_given_matrix()
    {
        using namespace std::literals;
        constexpr auto any_mask = [](int i, int j) { return i == 1 && j == 0; };
        constexpr auto any_matrix = make_matrix<char>({3,3}, 
            ",,,"
            ",,,"
            ",,,"sv
        );

        constexpr auto f = apply_mask_functor(any_mask);

        static_assert(f(any_matrix) == make_matrix<char>({3,3},
            ",,,"
            "+,,"
            ",,,"sv
        ));
    }
}
#endif