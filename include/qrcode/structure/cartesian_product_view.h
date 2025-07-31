/*
 * The MIT License
 *
 * Copyright (c) 2021 Sebastian Bauer
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

#include <algorithm>
#include <ranges>
#include <concepts>
#include <iterator>

#include <qrcode/structure/position.h>

namespace qrcode::structure::detail
{
    template<std::ranges::range Range0, std::ranges::range Range1>
    requires std::integral<std::ranges::range_value_t<Range0>>
        && std::integral<std::ranges::range_value_t<Range1>>
    class cartesian_iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = position;
        using reference = value_type;

        constexpr cartesian_iterator() = default;
        
        constexpr cartesian_iterator(Range0* r0, Range1* r1) 
        : range0{r0}, range1{r1}
        , i0{std::ranges::begin(*range0)}, i1{std::ranges::begin(*range1)}
        {
        }

        [[nodiscard]] constexpr auto operator==(
            cartesian_iterator const&) const noexcept -> bool = default;
            
        [[nodiscard]] constexpr auto operator!=(
            cartesian_iterator const&) const noexcept -> bool = default;

        constexpr auto& operator++() noexcept
        {
            using std::ranges::begin;
            using std::ranges::end;
            
            ++i0;

            if (i0 == end(*range0))
            {
                i0 = begin(*range0);
                ++i1;
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
            return position{*i0, *i1};
        }

        [[nodiscard]] friend constexpr auto operator==(
            cartesian_iterator const& iterator, std::default_sentinel_t) noexcept
        {
            using std::ranges::end;
            return iterator.i1 == end(*(iterator.range1));
        }

    private:
        Range0* range0{};
        Range1* range1{};

        std::ranges::iterator_t<Range0> i0{};
        std::ranges::iterator_t<Range1> i1{};
    };

    template<std::ranges::viewable_range Range0, std::ranges::viewable_range Range1>
    [[nodiscard]] constexpr auto operator!=(
        cartesian_iterator<Range0, Range1> const& iterator, 
        std::default_sentinel_t const& sentinel) noexcept
    {
        return !(iterator == sentinel);
    }
}

namespace qrcode::structure
{
    template<std::ranges::range Range0, std::ranges::range Range1>
    requires std::integral<std::ranges::range_value_t<Range0>>
        && std::integral<std::ranges::range_value_t<Range1>>
    class cartesian_product_view 
    : public std::ranges::view_interface<cartesian_product_view<Range0, Range1>>
    {
    public:
        constexpr explicit cartesian_product_view(Range0 r0, Range1 r1)
        : range0{std::move(r0)}, range1{std::move(r1)}
        {
        }

        constexpr cartesian_product_view() noexcept = default;

        [[nodiscard]] constexpr auto begin() const noexcept
        {
            using qrcode::structure::detail::cartesian_iterator;
            using std::ranges::begin;
            using std::ranges::end;
            return cartesian_iterator{&range0, &range1}; 
        }

        [[nodiscard]] constexpr auto end() const noexcept 
        { 
            return std::default_sentinel; 
        }

    private:
        Range0 range0{};
        Range1 range1{};
    };

    template<std::ranges::viewable_range Range0, std::ranges::viewable_range Range1>
    cartesian_product_view(Range0&&, Range1&&) 
    -> cartesian_product_view<std::ranges::views::all_t<Range0>, std::ranges::views::all_t<Range1>>;
}

namespace qrcode::structure::views
{
    inline constexpr auto cartesian_product = [](auto&& r0, auto&& r1)
    {
        using range0 = std::decay_t<decltype(r0)>;
        using range1 = std::decay_t<decltype(r1)>;
        return cartesian_product_view{std::forward<range0>(r0), std::forward<range1>(r1)};
    };
}

namespace qrcode::views
{
    using qrcode::structure::views::cartesian_product;
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::structure::test
{
    constexpr auto cartesian_product_can_be_composed_of_integer_sequencess_constructing_positions()
    {
        auto f = []()
        {
            auto range = 
                cartesian_product_view{std::array{1,2,3,4}, std::array{6,7,8}};

            return std::ranges::equal(range, std::array{
                position{1,6}, position{2,6}, position{3,6}, position{4,6},
                position{1,7}, position{2,7}, position{3,7}, position{4,7},
                position{1,8}, position{2,8}, position{3,8}, position{4,8}
            });
        };
        static_assert(f());
    }

    constexpr auto cartesian_products_can_be_composed_of_integer_sequencess_constructing_positions()
    {
        auto f = []()
        {
            auto const range = 
                cartesian_product_view{std::array{1,2,3,4}, std::array{6,7,8}};

            return std::ranges::equal(
                range, 
                std::array{
                    position{1,6}, position{2,6}, position{3,6}, position{4,6}, 
                    position{1,7}, position{2,7}, position{3,7}, position{4,7}, 
                    position{1,8}, position{2,8}, position{3,8}, position{4,8}
                }
             );
        };
        static_assert(f());
    }

    constexpr auto cartesian_products_can_be_used_with_adaptors()
    {
        auto f = []() constexpr
        {
            auto const range = 
                views::cartesian_product(std::array{1,2}, std::array{6,7,8});

            return std::ranges::equal(
                range, 
                std::array{
                    position{1,6}, position{2,6}, position{1,7}, 
                    position{2,7},position{1,8},position{2,8}
                }
            );
        };
        static_assert(f());
    }

    constexpr auto cartesian_products_can_be_used_with_adaptor_chains()
    {
        auto f = []() constexpr
        {
            auto range = 
                views::cartesian_product(std::array{1,2}, std::array{6,7,8})
                | std::views::filter([](auto point) { return point.y > 6; });

            return std::ranges::equal(
                range, 
                std::array{position{1,7},position{2,7},position{1,8},position{2,8}}
            );
        };
        static_assert(f());
    }
}
#endif