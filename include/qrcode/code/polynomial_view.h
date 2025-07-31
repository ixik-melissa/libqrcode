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
#include <optional>
#include <algorithm>

#include <qrcode/code/polynomial.h>

namespace qrcode::code
{
    struct polynomial_info_t
    {
        int degree{0};
        int short_polynomials{0};
    };
}

namespace qrcode::code::detail
{
    template<class Coefficient, std::forward_iterator Iterator>
    requires std::convertible_to<typename std::iterator_traits<Iterator>::value_type, Coefficient>
    [[nodiscard]] constexpr auto make_polynomial(int degree, Iterator iterator) noexcept
    {
        auto poly = polynomial<Coefficient>(degree, Coefficient{});
        
        for (auto i = rbegin(poly); i != rend(poly); ++i, ++iterator)
            *i = static_cast<Coefficient>(*iterator);

        return std::make_pair(poly, iterator);
    }

    template<std::forward_iterator Iterator, class Sentinel>
    class polynomial_view_iterator
    {
    public:
        using difference_type = std::iterator_traits<Iterator>::difference_type;
        using value_type = polynomial<typename std::iterator_traits<Iterator>::value_type>;
        using iterator_category = std::forward_iterator_tag;

        constexpr polynomial_view_iterator(
            Iterator iterator_, Sentinel sentinel_, polynomial_info_t* polynomial_info) 
        : iterator{std::move(iterator_)}
        , sentinel{std::move(sentinel_)}
        , polynomial_info{polynomial_info}
        {
            ++(*this);
        }

        constexpr polynomial_view_iterator() noexcept = default;

        constexpr auto& operator++() noexcept
        {
            current_value = std::optional<value_type>{};

            if (iterator != sentinel)
            {
                auto degree = polynomial_info->degree;

                if (polynomial_info->short_polynomials)
                {
                    --polynomial_info->short_polynomials;
                    --degree;
                }

                // std::tie doesn't work with optionals as assignment of values is not constexpr
                using coefficient_type = typename std::iterator_traits<Iterator>::value_type;
                auto const [new_value, new_iterator] = 
                    make_polynomial<coefficient_type>(degree, iterator);
                current_value = std::optional<value_type>{new_value};
                iterator = new_iterator;
            }
            
            return *this;
        }

        constexpr auto operator++(int) noexcept
        {
            auto copy = *this;
            ++(*this);
            return copy;
        }

        [[nodiscard]] constexpr auto operator*() const noexcept
        {
            return *current_value;
        }

        [[nodiscard]] friend constexpr auto operator==(
            polynomial_view_iterator const& a, polynomial_view_iterator const& b) noexcept -> bool
        {
            return a.iterator == b.iterator
                && a.polynomial_info == b.polynomial_info
                && a.current_value == b.current_value;
        }

        [[nodiscard]] friend constexpr auto operator==(
            polynomial_view_iterator const& iterator, std::default_sentinel_t) noexcept -> bool
        {
            return !iterator.current_value.has_value();
        }

    private:
        Iterator iterator{};
        Sentinel sentinel{};
        polynomial_info_t* polynomial_info;
        std::optional<value_type> current_value{};
    };

    template<std::forward_iterator Iterator, class Sentinel>
    [[nodiscard]] constexpr auto operator!=(
        polynomial_view_iterator<Iterator, Sentinel> const& a, 
        polynomial_view_iterator<Iterator, Sentinel> const& b) noexcept
    {
        return !(a == b);
    }

    template<std::forward_iterator Iterator, class Sentinel>
    [[nodiscard]] constexpr auto operator!=(
        polynomial_view_iterator<Iterator, Sentinel> const& iterator, 
        std::default_sentinel_t const& sentinel) noexcept
    {
        return !(iterator == sentinel);
    }
}

namespace qrcode::code
{
    template<std::ranges::viewable_range Range>
    class polynomial_view : public std::ranges::view_interface<polynomial_view<Range>>
    {
    public:
        constexpr polynomial_view(Range range_, polynomial_info_t polynomial_info_) noexcept
        : range{std::move(range_)}
        , polynomial_info{std::move(polynomial_info_)}
        {
        }

        constexpr polynomial_view() noexcept = default;

        [[nodiscard]] constexpr auto begin() noexcept
        {
            using qrcode::code::detail::polynomial_view_iterator;
            using std::ranges::begin;
            using std::ranges::end;
            return polynomial_view_iterator{begin(range), end(range), &polynomial_info}; 
        }

        [[nodiscard]] constexpr auto end() noexcept 
        { 
            return std::default_sentinel;
        }

    private:
        Range range{};
        polynomial_info_t polynomial_info;
    };

    template<std::ranges::viewable_range Range>
    polynomial_view(Range&&, polynomial_info_t) -> polynomial_view<std::ranges::views::all_t<Range>>;
}

namespace qrcode::code::views::detail
{
    struct polynomial_view_adaptor 
    {
        polynomial_info_t polynomial_info;
    };

    template<std::ranges::viewable_range Range>
    [[nodiscard]] constexpr auto operator|(Range&& range, polynomial_view_adaptor adaptor) noexcept
    {
        return polynomial_view{
            std::forward<Range>(range), 
            std::move(adaptor.polynomial_info)};
    }
}

namespace qrcode::code::views
{
    inline constexpr auto polynomial = [](polynomial_info_t polynomial_info)
    {
        using qrcode::code::views::detail::polynomial_view_adaptor;
        return polynomial_view_adaptor{std::move(polynomial_info)};
    };
}

namespace qrcode::views
{
    using qrcode::code::views::polynomial;
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::code::detail::test
{
    constexpr auto make_polynomial_interprets_data_as_coefficients_starting_with_the_highest_term_and_returns_polynomial_from_given_degree()
    {
        constexpr auto some_data = std::array{0,1,2,3,4,5,6,7,8,9,10,11,12};
        constexpr auto other_data = std::array{3,4,5,6,7,8,9,10,11,12};
        constexpr auto some_other_data = std::array{7,8,9,10,11,12};

        static_assert(make_polynomial<char>(2, begin(some_data)) == std::make_pair(polynomial<char>{{2,1,0}}, begin(some_data)+3));
        static_assert(make_polynomial<char>(3, begin(other_data)) == std::make_pair(polynomial<char>{{6,5,4,3}}, begin(other_data)+4));
        static_assert(make_polynomial<char>(5, begin(some_other_data)) == std::make_pair(polynomial<char>{{12,11,10,9,8,7}}, begin(some_other_data)+6));
    }
}

namespace qrcode::code::test
{
    constexpr auto polynomial_views_can_be_generated_from_given_coefficients_starting_with_the_highest_coefficients()
    {
        auto f = []
        {
            constexpr auto some_coefficients = std::array{3,1,4,1,5,9,2,6,5,3,5,8,9};

            auto range = polynomial_view{some_coefficients, {.degree=2, .short_polynomials=2}};

            return std::ranges::equal(
               range, std::array{
                   polynomial{1,3}, polynomial{1,4}, 
                   polynomial{2,9,5}, polynomial{3,5,6}, polynomial{9,8,5}
                }
            );
        };
        
        static_assert(f());
    }

    constexpr auto polynomial_view_can_be_used_with_adaptors()
    {
        auto f = []() constexpr
        {
            constexpr auto some_coefficients = std::array{3,1,4,1,5,9,2,6,5,3,5,8};

            auto range = some_coefficients 
                | views::polynomial({.degree=2, .short_polynomials=3});

            return std::ranges::equal(
                range, std::array{
                    polynomial{1,3}, polynomial{1,4}, polynomial{9,5}, 
                    polynomial{5,6,2}, polynomial{8,5,3}
                }
            );
        };
        static_assert(f());
    }

    constexpr auto polynomial_view_can_be_used_in_adaptor_chains()
    {
        auto f = []() constexpr
        {
            constexpr auto some_coefficients = std::array{3,1,4,1,5,9,2,6,5,3,5,8};

            auto range = some_coefficients 
                | views::polynomial({.degree=2, .short_polynomials=3})
                | std::views::transform([](auto v) { return degree(v); });

            return std::ranges::equal(range, std::array{1,1,1,2,2});
        };
        static_assert(f());
    }
}
#endif
