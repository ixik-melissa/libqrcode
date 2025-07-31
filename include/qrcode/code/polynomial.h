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

#include <cx/vector.h>
#include <algorithm>
#include <limits>
#include <ranges>

namespace qrcode::code
{
    template<class T>
    class polynomial
    {
        cx::vector<T> coefficients;

    public:     
        using value_type = T;
        using size_type = int;
        using const_iterator = typename decltype(coefficients)::const_iterator;

        template<class Coefficients> requires std::ranges::range<Coefficients>
        explicit constexpr polynomial(Coefficients polynomial_coefficients) noexcept 
        : coefficients{std::ranges::begin(polynomial_coefficients), std::ranges::end(polynomial_coefficients)}
        {
        }

        explicit constexpr polynomial(std::initializer_list<T> polynomial_coefficients) noexcept 
        : coefficients{std::move(polynomial_coefficients)}
        {
        }

        explicit constexpr polynomial(int degree, T fill) noexcept 
        : coefficients(degree+1, fill)
        {
        }
        
        [[nodiscard]] constexpr auto begin() const noexcept { using std::ranges::begin; return begin(coefficients); }
        [[nodiscard]] constexpr auto end() const noexcept{ using std::ranges::end; return end(coefficients); }

        [[nodiscard]] constexpr auto begin() noexcept { using std::ranges::begin; return begin(coefficients); }
        [[nodiscard]] constexpr auto end() noexcept{ using std::ranges::end; return end(coefficients); }

        [[nodiscard]] constexpr auto rbegin() const noexcept { using std::ranges::rbegin; return rbegin(coefficients); }
        [[nodiscard]] constexpr auto rend() const noexcept { using std::ranges::rend; return rend(coefficients); }

        [[nodiscard]] constexpr auto rbegin() noexcept { using std::ranges::rbegin; return rbegin(coefficients); }
        [[nodiscard]] constexpr auto rend() noexcept { using std::ranges::rend; return rend(coefficients); }

        [[nodiscard]] constexpr auto size() const noexcept { using std::ranges::size; return static_cast<size_type>(size(coefficients)); }

        [[nodiscard]] constexpr auto operator==(polynomial const&) const noexcept -> bool = default;
        [[nodiscard]] constexpr auto operator!=(polynomial const&) const noexcept -> bool = default;
    };

    template<class T>
    polynomial(std::initializer_list<T> const& coeffients) -> polynomial<T>;

    using std::ranges::begin;
    using std::ranges::end;
    using std::ranges::rbegin;
    using std::ranges::rend;
    using std::ranges::size;

    template<class T>
    [[nodiscard]] constexpr auto degree(polynomial<T> const& p) noexcept
    {
        return static_cast<int>(size(p)) - 1;
    }

    template<class T>
    [[nodiscard]] constexpr auto coeffient(polynomial<T> const& p, int index) noexcept
    {
        return *(begin(p) + index);
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::code::test
{
    constexpr auto polynomial_can_be_constructed_from_coefficients()
    {
        static_assert(size(polynomial<int>{}) == 0);
        static_assert(std::ranges::equal(polynomial<int>{std::array{9}}, std::array{9}));
        static_assert(std::ranges::equal(polynomial<int>{std::array{1, 2}}, std::array{1, 2}));
        static_assert(std::ranges::equal(polynomial<int>{std::array{4, 5, 6}}, std::array{4, 5, 6}));
    }

    constexpr auto polynomial_provides_value_type_for_its_coefficients()
    {
        using any_type = int;
        using other_type = float;
        
        static_assert(std::is_same_v<any_type, polynomial<any_type>::value_type>);
        static_assert(std::is_same_v<other_type, polynomial<other_type>::value_type>);
    }

    constexpr auto polynomial_has_a_degree()
    {
        static_assert(std::numeric_limits<decltype(degree(std::declval<polynomial<int>>()))>::is_signed);
            
        static_assert(degree(polynomial{1}) == 0);
        static_assert(degree(polynomial{1, 2}) == 1);
        static_assert(degree(polynomial{3, 2, 4, 7, 12}) == 4);
    }

    constexpr auto polynomial_has_a_size_which_indicates_the_number_of_coefficients()
    {
        static_assert(std::numeric_limits<decltype(size(std::declval<polynomial<int>>()))>::is_signed);
            
        static_assert(size(polynomial{1}) == 1);
        static_assert(size(polynomial{1, 2}) == 2);
        static_assert(size(polynomial{3, 2, 4, 7, 12}) == 5);
    }

    constexpr auto polynomial_can_be_created_with_a_given_size_and_fill_element()
    {
        auto f = [](auto polynomial, auto nominal)
        {
            return std::equal(begin(polynomial), end(polynomial), begin(nominal), end(nominal));
        };

        static_assert(f(polynomial(0, -1), std::array{-1}));
        static_assert(f(polynomial(1, -1), std::array{-1, -1}));
        static_assert(f(polynomial(1, -3), std::array{-3, -3}));
    }

    constexpr auto polynomial_supports_coefficients_read_access_via_iterators()
    {
        auto f = []
        {
            auto const some_coefficients = {3, 2, 16};

            auto const any_polynomial = polynomial(some_coefficients);

            return std::equal(
                begin(any_polynomial), end(any_polynomial), 
                begin(some_coefficients), end(some_coefficients));
        };
        static_assert(f());
    }

    constexpr auto polynomial_supports_coefficients_write_access_via_iterators()
    {
        auto f = []
        {
            auto any_polynomial = polynomial{3, 2, 16};

            std::for_each(begin(any_polynomial), end(any_polynomial), [](auto& v){ v *= v; });
            
            auto const nominal = std::array{9, 4, 256};
            return std::equal(
                begin(any_polynomial), end(any_polynomial), 
                begin(nominal), end(nominal));
        };
        static_assert(f());
    }

    constexpr auto polynomial_supports_coefficients_access_via_reverse_iterators()
    {
        auto f = []
        {
            auto const some_coefficients = {3, 2, 16};

            auto const any_polynomial = polynomial(some_coefficients);
            
            return std::equal(
                rbegin(any_polynomial), rend(any_polynomial), 
                rbegin(some_coefficients), rend(some_coefficients));
        };
        static_assert(f());
    }

    constexpr auto polynomial_supports_coefficients_write_access_via_reverse_iterators()
    {
        auto f = []
        {
            auto any_polynomial = polynomial{3, 2, 16};

            std::for_each(rbegin(any_polynomial), rend(any_polynomial), [](auto& v){ v *= 2; });
            
            auto const nominal = std::array{6, 4, 32};
            return std::equal(
                begin(any_polynomial), end(any_polynomial), 
                begin(nominal), end(nominal));
        };
        static_assert(f());
    }
}
#endif
