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

#include <algorithm>
#include <concepts>
#include <iterator>
#include <optional>
#include <ranges>
#include <string_view>

namespace qrcode::code::detail
{
    template<std::forward_iterator Iterator, class Sentinel, class Padding> 
    requires std::invocable<Padding&, int>
    class padding_view_iterator
    {
    public:
        using difference_type = std::iterator_traits<Iterator>::difference_type;
        using value_type = std::iterator_traits<Iterator>::value_type;
        using iterator_category = std::forward_iterator_tag;

        constexpr padding_view_iterator() = default;
        
        constexpr padding_view_iterator(Iterator begin, Sentinel end, Padding* range_padding) 
        : iterator{std::move(begin)}
        , sentinel{std::move(end)}
        , index{0}
        , padding{range_padding}
        , current_padding{}
        {
            if (iterator == sentinel)
                current_padding = (*padding)(index);
        }

        constexpr auto& operator++() noexcept
        {
            ++index;
            
            if (iterator != sentinel)
                ++iterator;

            if (iterator == sentinel)
                current_padding = (*padding)(index);

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
            return iterator == sentinel ? *current_padding : *iterator;
        }

        [[nodiscard]] friend constexpr auto operator==(
            padding_view_iterator const& a, padding_view_iterator const& b) noexcept -> bool
        {
            return a.iterator == b.iterator
                && a.padding == b.padding
                && a.current_padding == b.current_padding;
        }

        [[nodiscard]] friend constexpr auto operator==(
            padding_view_iterator const& iterator, std::default_sentinel_t const&) noexcept -> bool
        {
            return iterator.iterator == iterator.sentinel 
                && !iterator.current_padding.has_value();
        }

    private:
        Iterator iterator{};
        Sentinel sentinel{};
        int index = 0;
        Padding* padding = nullptr;
        std::optional<value_type> current_padding{};
    };

    template<std::forward_iterator Iterator, class Sentinel, class Padding> 
    [[nodiscard]] constexpr auto operator!=(
        padding_view_iterator<Iterator, Sentinel, Padding> const& a,
        padding_view_iterator<Iterator, Sentinel, Padding> const& b) noexcept -> bool
    {
        return !(*a == b);
    }

    template<std::forward_iterator Iterator, class Sentinel, class Padding> 
    [[nodiscard]] constexpr auto operator!=(
        padding_view_iterator<Iterator, Sentinel, Padding> const& iterator,
        std::default_sentinel_t const& sentinel) noexcept -> bool
    {
        return !(*iterator == sentinel);
    }

    template<class F>
    requires std::copy_constructible<F> && std::regular_invocable<F&>
    class ignore_arguments
    {
    public:
        constexpr ignore_arguments(F wrapped) noexcept : f{std::move(wrapped)} {}

        [[nodiscard]] constexpr auto operator()(auto&&...) noexcept { return f(); }

    private:
        F f;
    };
}

namespace qrcode::code
{
    template<std::ranges::viewable_range Range, class Padding>
    requires std::copy_constructible<Padding> && std::regular_invocable<Padding&, int>
    class padding_view : public std::ranges::view_interface<padding_view<Range, Padding>>
    {
    public:
        constexpr padding_view(Range the_range, Padding range_padding) noexcept
        : range{std::move(the_range)}, padding{std::move(range_padding)}
        {
        }

        constexpr padding_view() noexcept = default;

        constexpr padding_view(padding_view const& rhs) noexcept
        : range{rhs.range}
        , padding{rhs.padding}
        {
        }

        constexpr padding_view(padding_view&& rhs) noexcept
        : range{std::move(rhs.range)}
        , padding{std::move(rhs.padding)}
        {
        }

        constexpr padding_view& operator=(padding_view const& rhs) noexcept
        requires std::is_copy_assignable_v<Range>
        {
            range = rhs.range;
            if constexpr (std::is_copy_assignable_v<Padding>)
                padding = rhs.padding;
            return *this;
        }

        constexpr padding_view& operator=(padding_view&& rhs) noexcept
        requires std::is_move_assignable_v<Range>
        {
            range = std::move(rhs.range);
            if constexpr (std::is_move_assignable_v<Padding>)
                padding = rhs.padding;
            return *this;
        }

        [[nodiscard]] constexpr auto begin() noexcept
        {
            using std::ranges::begin;
            using std::ranges::end;
            using qrcode::code::detail::padding_view_iterator;
            return padding_view_iterator{begin(range), end(range), &padding.value()}; 
        }

        [[nodiscard]] constexpr auto end() noexcept
        { 
            return std::default_sentinel;
        }

        [[nodiscard]] constexpr auto begin() const noexcept
        requires std::ranges::range<Range const>
            && std::regular_invocable<Padding const&>
        {
            using std::ranges::begin;
            using std::ranges::end;
            using qrcode::code::detail::padding_view_iterator;
            return padding_view_iterator{begin(range), end(range), &padding.value()}; 
        }

        [[nodiscard]] constexpr auto end() const noexcept
        requires std::ranges::range<Range const>
            && std::regular_invocable<Padding const&>
        { 
            return std::default_sentinel;
        }

    private:
        Range range{};
        std::optional<Padding> padding{};
    };

    template<class Range, class Padding>
    requires std::regular_invocable<Padding&>
    padding_view(Range&&, Padding) 
    -> padding_view<std::ranges::views::all_t<Range>, qrcode::code::detail::ignore_arguments<Padding>>;

    template<class Range, class Padding>
    requires std::regular_invocable<Padding&, int>
    padding_view(Range&&, Padding) -> padding_view<std::ranges::views::all_t<Range>, Padding>;
}

namespace qrcode::code::views::detail
{
    template<class Padding>
    struct padding_view_adaptor 
    {
        Padding padding;
    };

    template<std::ranges::viewable_range Range, class Padding>
    [[nodiscard]] constexpr auto operator|(
        Range&& range, padding_view_adaptor<Padding> adaptor) noexcept
    {
        return padding_view{std::forward<Range>(range), std::move(adaptor.padding)};
    }
}

namespace qrcode::code::views
{
    inline constexpr auto padding = [](auto padding)
    {
        using qrcode::code::views::detail::padding_view_adaptor;
        return padding_view_adaptor{std::move(padding)};
    };
}

namespace qrcode::views
{
    using qrcode::code::views::padding;
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::code::detail::test
{
    constexpr auto padding_view_iterators_return_given_range_when_being_dereferenced()
    {
        auto f = []() constexpr
        {
            auto const some_ints = std::array{1,10,7};
            auto any_padding = [](auto&&...){ return 0; };

            auto iterator = padding_view_iterator{begin(some_ints), end(some_ints), &any_padding};

            return *(iterator++) == 1
                && *(iterator++) == 10
                && *(iterator++) == 7;
        };
        static_assert(f());
    }

    constexpr auto padding_view_iterators_return_padding_when_given_range_is_depleted()
    {
        auto f = []
        {
            auto const some_ints = std::array{1,3,7};
            auto any_padding = [count_down=10](auto&&...) mutable { return count_down--; };

            auto iterator = padding_view_iterator{begin(some_ints), end(some_ints), &any_padding};

            return *(iterator++) == 1
                && *(iterator++) == 3
                && *(iterator++) == 7
                && *(iterator++) == 10
                && *(iterator++) == 9
                && *(iterator++) == 8
                && *(iterator++) == 7;
        };
        static_assert(f());
    }

    constexpr auto padding_view_iterators_can_be_post_incremented()
    {
        auto f = []() constexpr
        {
            auto const some_ints = std::array{5,3};
            auto any_padding = [](auto&&...){ return 0; };

            auto iterator = padding_view_iterator{begin(some_ints), end(some_ints), &any_padding};


            auto const old = iterator++;
            return *iterator == 3 && *old == 5;
        };
        static_assert(f());
    }

    constexpr auto padding_view_iterators_can_be_pre_incremented()
    {
        auto f = []() constexpr
        {
            auto const some_ints = std::array{3,6,9};
            auto any_padding = [](auto&&...){ return 0; };

            auto iterator = padding_view_iterator{begin(some_ints), end(some_ints), &any_padding};

            auto const same_object = std::addressof(++iterator);
            return same_object == std::addressof(iterator) && *iterator == 6;
        };
        static_assert(f());
    }

    constexpr auto padding_view_iterators_can_be_compared_to_each_other()
    {
        auto f = []
        {
            auto any_padding = [](auto&&...){ return 0; };
            auto const some_ints = std::array{3,6,9};
            auto const copy = some_ints;

            auto const any_iterator = padding_view_iterator{begin(some_ints), end(some_ints), &any_padding};
            auto const same_iterator = padding_view_iterator{begin(some_ints), end(some_ints), &any_padding};
            auto const other_iterator = padding_view_iterator{begin(some_ints)+1, end(some_ints), &any_padding};
            auto const iterator_from_copy = padding_view_iterator{begin(copy), end(copy), &any_padding};

            return (any_iterator == any_iterator)
                && !(any_iterator != any_iterator)
                && (any_iterator == same_iterator)
                && !(any_iterator != same_iterator)
                && any_iterator != other_iterator
                && !(any_iterator == other_iterator)
                && any_iterator != iterator_from_copy
                && any_iterator != iterator_from_copy
            ;
        };
        static_assert(f());
    }

    constexpr auto padding_view_iterators_pass_current_index_to_padding()
    {
        auto f = []
        {
            auto any_padding = [](auto index){ return index; };
            auto const some_ints = std::array{-1};

            auto any_iterator = padding_view_iterator{begin(some_ints), end(some_ints), &any_padding};

            return *(any_iterator++) == -1
                && *(any_iterator++) == 1
                && *(any_iterator++) == 2
                && *(any_iterator++) == 3
                && *(any_iterator++) == 4;
        };
        static_assert(f());
    }
}

namespace qrcode::code::test
{
    constexpr auto padding_views_return_their_given_range()
    {
        auto f = []() constexpr
        {
            auto const some_ints = std::array{3,1,4,1,5};
            auto any_padding = [] { return 1; };

            auto range = padding_view{some_ints, any_padding};
            
            auto i = begin(range);
            return *(i++) == 3
                && *(i++) == 1
                && *(i++) == 4
                && *(i++) == 1
                && *(i++) == 5;
        };
        static_assert(f());
    }

    constexpr auto padding_views_return_padding_when_given_range_is_depleted()
    {
        auto f = []() constexpr
        {
            auto const some_ints = std::array{3};
            auto any_padding = [count=1]() mutable { return count++; };

            auto range = padding_view{some_ints, any_padding};
            
            auto i = begin(range);
            return *(i++) == 3
                && *(i++) == 1
                && *(i++) == 2
                && *(i++) == 3
                && *(i++) == 4;
        };
        static_assert(f());
    }

    constexpr auto padding_views_is_depleted_if_range_is_depleted_and_padding_returns_an_empty_optional()
    {
        auto f = []() constexpr
        {
            auto const some_ints = std::array{-1};
            auto any_padding = [count=3]() mutable 
            { 
                --count; 
                return count > 0 ? std::optional{5} : std::optional<int>{}; 
            };

            auto range = padding_view{some_ints, any_padding};
            
            return std::ranges::equal(range, std::array{-1,5,5});
        };
        static_assert(f());
    }

    constexpr auto padding_views_pass_current_index_to_padding()
    {
        auto f = []() constexpr
        {
            auto const some_ints = std::array{3};
            auto any_padding = [](auto index) mutable { return index * index; };

            auto range = padding_view{some_ints, any_padding};
            
            auto i = begin(range);
            return *(i++) == 3
                && *(i++) == 1
                && *(i++) == 4
                && *(i++) == 9
                && *(i++) == 16;
        };
        static_assert(f());
    }

    constexpr auto padding_views_can_be_used_with_adaptors()
    {
        auto f = []() constexpr
        {
            auto const some_ints = std::array{1,2,3};
            auto any_padding = [count=0]() mutable { return count--; };

            auto range = some_ints | views::padding(any_padding);
            
            auto i = begin(range);
            return *(i++) == 1
                && *(i++) == 2
                && *(i++) == 3
                && *(i++) == 0
                && *(i++) == -1
                && *(i++) == -2
                && *(i++) == -3;
        };
        static_assert(f());
    }

    constexpr auto padding_views_can_be_used_in_adaptor_chains()
    {
        auto f = []() constexpr
        {
            auto const some_ints = std::array{1,2,3};
            auto any_padding = [count=0]() mutable { return count--; };

            auto range = some_ints 
                | views::padding(any_padding) 
                | std::views::transform([](auto v) { return 2*v; });

            auto i = begin(range);
            return *(i++) == 2
                && *(i++) == 4
                && *(i++) == 6
                && *(i++) == 0
                && *(i++) == -2
                && *(i++) == -4
                && *(i++) == -6;
        };
        static_assert(f());
    }
}
#endif
