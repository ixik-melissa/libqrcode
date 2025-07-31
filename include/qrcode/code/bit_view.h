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
#include <cstddef>
#include <algorithm>

namespace qrcode::code::detail
{
    template<class Sentinel>
    class bit_view_sentinel;

    template<std::forward_iterator Iterator>
    requires std::convertible_to<typename std::iterator_traits<Iterator>::value_type, std::byte>
    class bit_view_iterator
    {
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = bool;
        using iterator_category = std::forward_iterator_tag;

        constexpr bit_view_iterator() noexcept = default;

        constexpr bit_view_iterator(Iterator iterator) noexcept
        : byte_iterator{std::move(iterator)}, mask{0b1000'0000}
        {
        }

        constexpr auto operator*() const noexcept
        {
            return static_cast<bool>(*byte_iterator & mask);
        }

        constexpr auto& operator++() noexcept
        {
            mask >>= 1;

            if (mask == std::byte{0})
            {
                ++byte_iterator;
                mask = std::byte{0b1000'0000};
                return *this;
            }

            return *this;
        }

        constexpr auto operator++(int)
        {
            auto copy = *this;
            ++(*this);
            return copy;
        }

        [[nodiscard]] constexpr auto operator==(bit_view_iterator const&) const noexcept -> bool = default;
        [[nodiscard]] constexpr auto operator!=(bit_view_iterator const&) const noexcept -> bool = default;

    private:
        template<class Sentinel> friend class bit_view_sentinel;

        Iterator byte_iterator{};
        std::byte mask{0};
    };

    template<class Sentinel>
    class bit_view_sentinel
    {
    public:
        constexpr bit_view_sentinel(Sentinel sentinel) noexcept
        : byte_sentinel{sentinel}
        {
        }

        constexpr bit_view_sentinel() noexcept = default;

        template<class Iterator_, class Sentinel_>
        friend constexpr auto operator==(
            bit_view_iterator<Iterator_> const& iterator, 
            bit_view_sentinel<Sentinel_> const& sentinel) noexcept -> bool;

    private:
        template<class Iterator>
        [[nodiscard]] constexpr auto equal(
            bit_view_iterator<Iterator> const& iterator) const noexcept
        {
            return byte_sentinel == iterator.byte_iterator;
        }

        Sentinel byte_sentinel{};
    };

    template<class Iterator, class Sentinel>
    [[nodiscard]] constexpr auto operator==(
        bit_view_iterator<Iterator> const& iterator, 
        bit_view_sentinel<Sentinel> const& sentinel) noexcept -> bool
    {
        return sentinel.equal(iterator);
    }
}

namespace qrcode::code
{
    template<std::ranges::viewable_range Byte_Range>
    requires std::convertible_to<std::ranges::range_value_t<Byte_Range>, std::byte>
    class bit_view : public std::ranges::view_interface<bit_view<Byte_Range>>
    {
    public:
        constexpr explicit bit_view(Byte_Range bytes) noexcept
        : byte_range{std::move(bytes)}
        {
        }

        constexpr bit_view() noexcept = default;

        [[nodiscard]] constexpr auto begin() const noexcept
        {
            using qrcode::code::detail::bit_view_iterator;
            using std::ranges::begin;
            return bit_view_iterator{begin(byte_range)}; 
        }

        [[nodiscard]] constexpr auto end() const noexcept 
        { 
            using qrcode::code::detail::bit_view_sentinel;
            using std::ranges::end;
            return bit_view_sentinel{end(byte_range)}; 
        }

    private:
        Byte_Range byte_range;
    };

    template<std::ranges::viewable_range Range>
    bit_view(Range&&) -> bit_view<std::ranges::views::all_t<Range>>;
}

namespace qrcode::code::views::detail
{
    struct bit_view_adaptor {};

    template<std::ranges::viewable_range Range>
    [[nodiscard]] constexpr auto operator|(Range&& range, bit_view_adaptor) noexcept
    {
        return bit_view{std::forward<Range>(range)};
    }
}

namespace qrcode::code::views
{
    inline constexpr auto bit = qrcode::code::views::detail::bit_view_adaptor{};
}

namespace qrcode::views
{
    using qrcode::code::views::bit;
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::code::detail::test
{
    constexpr auto bit_view_iterators_return_bits_when_being_dereferenced()
    {
        auto f = []
        {
            auto const some_bytes = std::array{std::byte{0b1011'0110}, std::byte{0b0101'1001}};

            auto iterator = bit_view_iterator{begin(some_bytes)};

            return *(iterator++) == 1
                && *(iterator++) == 0
                && *(iterator++) == 1
                && *(iterator++) == 1
                && *(iterator++) == 0
                && *(iterator++) == 1
                && *(iterator++) == 1
                && *(iterator++) == 0;
        };
        static_assert(f());
    }

    constexpr auto bit_view_iterators_can_be_post_incremented()
    {
        auto f = []
        {
            auto const some_bytes = std::array{std::byte{0b1011'0110}, std::byte{0b0101'1001}};

            auto iterator = bit_view_iterator{begin(some_bytes)};

            auto const old = iterator++;
            return *iterator == 0 && *old == 1;
        };
        static_assert(f());
    }

    constexpr auto bit_view_iterators_can_be_pre_incremented()
    {
        auto f = []
        {
            auto const some_bytes = std::array{std::byte{0b1011'0110}, std::byte{0b0101'1001}};

            auto iterator = bit_view_iterator{begin(some_bytes)};

            auto const same_object = std::addressof(++iterator);
            return same_object == std::addressof(iterator) && *iterator == 0;
        };
        static_assert(f());
    }
}

namespace qrcode::code::test
{
    constexpr auto bit_views_accept_byte_ranges_and_returns_bits_starting_with_the_most_significant()
    {
        auto f = []
        {
            auto const some_bytes = std::array{std::byte{0b1100'0111}, std::byte{0b0101'1001}};

            auto const range = bit_view{some_bytes};
            
            return std::ranges::equal(range, std::array<bool,16>{1,1,0,0,0,1,1,1,  0,1,0,1,1,0,0,1});
        };
        static_assert(f());
    }

    constexpr auto bit_view_can_be_used_with_adaptors()
    {
        auto f = []() constexpr
        {
            auto const some_bytes = std::array{std::byte{0b1100'0111}, std::byte{0b0101'1001}};

            auto const range = some_bytes | views::bit;

            return std::ranges::equal(range, std::array<bool,16>{1,1,0,0,0,1,1,1,  0,1,0,1,1,0,0,1});
        };
        static_assert(f());
    }

    constexpr auto bit_view_can_be_used_in_adaptor_chains()
    {
        auto f = []() constexpr
        {
            auto const some_bytes = {std::byte{0b1100'0111}, std::byte{0b0101'1001}};

            auto const range = some_bytes 
                | views::bit 
                | std::views::transform([](auto v) { return !v; });

            return std::ranges::equal(range, std::array<bool,16>{0,0,1,1,1,0,0,0,  1,0,1,0,0,1,1,0});
        };
        static_assert(f());
    }
}
#endif