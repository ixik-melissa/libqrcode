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

#include <cstddef>
#include <utility>
#include <array>
#include <algorithm>
#include <ranges>
#include <concepts>
#include <iterator>
#include <optional>
#include <limits>

namespace qrcode::code::detail
{
    constexpr auto bits_per_byte()
    {
        return std::numeric_limits<std::uint8_t>::digits;
    }

    template<std::forward_iterator Bit_Iterator, class Bit_Sentinel>
    constexpr auto read_byte(Bit_Iterator iterator, Bit_Sentinel sentinel) 
    -> std::pair<std::byte, Bit_Iterator>
    {
        auto next = std::byte{0x00};
        constexpr auto digits = bits_per_byte();
        for (auto i = digits-1; i >= 0 && iterator != sentinel; --i)
        {
            auto const bit = static_cast<bool>(*(iterator++));
            next = next | static_cast<std::byte>(bit << i);
        }
        return std::make_pair(next, iterator);
    }
    
    template<std::forward_iterator Bit_Iterator, class Bit_Sentinel>
    class byte_view_iterator
    {
    public:
        using difference_type = std::iterator_traits<Bit_Iterator>::difference_type;
        using value_type = std::byte;
        using iterator_category = std::forward_iterator_tag;

        constexpr byte_view_iterator() = default;
        
        constexpr byte_view_iterator(Bit_Iterator iterator, Bit_Sentinel sentinel) 
        : bit_iterator{std::move(iterator)}, bit_sentinel{std::move(sentinel)} 
        {
            ++(*this);
        }

        constexpr auto& operator++() noexcept
        {
            current_value = std::optional<value_type>{};
            if (bit_iterator != bit_sentinel)
                std::tie(current_value, bit_iterator) = read_byte(bit_iterator, bit_sentinel);
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
            byte_view_iterator const& iterator, std::default_sentinel_t const&) noexcept -> bool
        {
            return !iterator.current_value.has_value();
        }

        [[nodiscard]] friend constexpr auto operator==(
            byte_view_iterator const& a, byte_view_iterator const& b) noexcept -> bool
        {
            return a.bit_iterator == b.bit_iterator 
                && a.current_value == b.current_value;
        }

    private:
        Bit_Iterator bit_iterator{};
        Bit_Sentinel bit_sentinel{};
        std::optional<value_type> current_value;
    };

    template<std::forward_iterator Bit_Iterator, class Bit_Sentinel>
    [[nodiscard]] constexpr auto operator!=(
        byte_view_iterator<Bit_Iterator, Bit_Sentinel> const& a, 
        byte_view_iterator<Bit_Iterator, Bit_Sentinel> const& b) noexcept
    {
        return !(a == b);
    }

    template<std::forward_iterator Bit_Iterator, class Bit_Sentinel>
    [[nodiscard]] constexpr auto operator!=(
        byte_view_iterator<Bit_Iterator, Bit_Sentinel> const& iterator, 
        std::default_sentinel_t const& sentinel) noexcept
    {
        return !(iterator == sentinel);
    }
}

namespace qrcode::code
{
    template<std::ranges::viewable_range Bit_Sequence>
    class byte_view : public std::ranges::view_interface<byte_view<Bit_Sequence>>
    {
    public:
        constexpr explicit byte_view(Bit_Sequence bits)
        : sequence{std::move(bits)}
        {
        }

        constexpr byte_view() noexcept = default;

        [[nodiscard]] constexpr auto begin() noexcept
        {
            using qrcode::code::detail::byte_view_iterator;
            using std::ranges::begin;
            using std::ranges::end;
            return byte_view_iterator{begin(sequence), end(sequence)}; 
        }

        [[nodiscard]] constexpr auto end() noexcept 
        { 
            return std::default_sentinel; 
        }

        [[nodiscard]] constexpr auto begin() const noexcept 
        requires std::ranges::range<Bit_Sequence const>
        {
            using qrcode::code::detail::byte_view_iterator;
            using std::ranges::begin;
            using std::ranges::end;
            return byte_view_iterator{begin(sequence), end(sequence)}; 
        }

        [[nodiscard]] constexpr auto end() const noexcept 
        requires std::ranges::range<Bit_Sequence const>
        { 
            return std::default_sentinel; 
        }

    private:
        Bit_Sequence sequence{};
    };

    template<std::ranges::viewable_range Range>
    byte_view(Range&&) -> byte_view<std::ranges::views::all_t<Range>>;
}

namespace qrcode::code::views::detail
{
    struct byte_view_adaptor {};

    template<std::ranges::viewable_range Range>
    [[nodiscard]] constexpr auto operator|(Range&& range, byte_view_adaptor) noexcept
    {
        return byte_view{std::forward<Range>(range)};
    }
}

namespace qrcode::code::views
{
    inline constexpr auto byte = qrcode::code::views::detail::byte_view_adaptor{};
}

namespace qrcode::views
{
    using qrcode::code::views::byte;
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::code::detail::test
{
    constexpr auto read_byte_returns_byte_msb_first_from_given_bit_range()
    {
        auto f = []
        {
            auto const some_bits = std::array<bool, 16>{1,1,0,0, 0,1,1,1, 0,1,0,1, 1,1,0,1};

            return read_byte(begin(some_bits)+0, end(some_bits)) == std::make_pair(std::byte{0b11000111}, begin(some_bits)+8)
                && read_byte(begin(some_bits)+1, end(some_bits)) == std::make_pair(std::byte{0b10001110}, begin(some_bits)+9)
                && read_byte(begin(some_bits)+2, end(some_bits)) == std::make_pair(std::byte{0b00011101}, begin(some_bits)+10)
                && read_byte(begin(some_bits)+3, end(some_bits)) == std::make_pair(std::byte{0b00111010}, begin(some_bits)+11)
                ;
        };
        static_assert(f());
    }

    constexpr auto read_byte_returns_stops_reading_when_range_is_depleted()
    {
        auto f = []
        {
            auto const some_bits = std::array<bool, 4>{0,1,1,1};

            return read_byte(begin(some_bits), end(some_bits)) 
                == std::make_pair(std::byte{0b0111'0000}, end(some_bits));
        };
        static_assert(f());
    }

    constexpr auto byte_view_iterators_return_bytes_when_being_dereferenced()
    {
        auto f = []
        {
            auto const some_bits = std::array<bool, 16>{1,1,0,0,0,1,1,1, 0,1,0,1,1,0,0,1};

            auto const iterator = byte_view_iterator{begin(some_bits), end(some_bits)};

            return *iterator == std::byte{0b11000111};
        };
        static_assert(f());
    }

    constexpr auto byte_view_iterators_can_be_advanced_to_the_next_byte()
    {
        auto f = []
        {
            auto const some_bits = std::array<bool, 16>{1,1,0,0,0,1,1,1, 0,1,0,1,1,0,0,1};
            auto iterator = byte_view_iterator{begin(some_bits), end(some_bits)};
            
            ++iterator;

            return *iterator == std::byte{0b01011001};
        };
        static_assert(f());
    }

    constexpr auto byte_view_iterators_can_be_post_incremented()
    {
        auto f = []
        {
            auto const some_bits = std::array<bool, 16>{1,1,0,0,0,1,1,1, 0,1,0,1,1,0,0,1};

            auto iterator = byte_view_iterator{begin(some_bits), end(some_bits)};

            auto const old = iterator++;
            return *iterator == std::byte{0b0101'1001} && *old == std::byte{0b1100'0111};
        };
        static_assert(f());
    }

    constexpr auto byte_view_iterators_can_be_pre_incremented()
    {
        auto f = []
        {
            auto const some_bits = std::array<bool, 16>{1,1,0,0,0,1,1,1, 0,1,0,1,1,0,0,1};

            auto iterator = byte_view_iterator{begin(some_bits), end(some_bits)};

            auto const same_object = std::addressof(++iterator);
            return same_object == std::addressof(iterator) && *iterator == std::byte{0b0101'1001};
        };
        static_assert(f());
    }

    constexpr auto byte_view_iterators_can_be_compared_to_their_sentinel()
    {
        auto f = []
        {
            auto const some_bits = std::array<bool, 16>{1,1,0,0,0,1,1,1, 0,1,0,1,1,0,0,1};

            auto const any_iterator = byte_view_iterator{begin(some_bits), end(some_bits)};
            auto const end_iterator = byte_view_iterator{end(some_bits), end(some_bits)};

            return !(any_iterator == std::default_sentinel)
                && any_iterator != std::default_sentinel
                && end_iterator == std::default_sentinel
                && !(end_iterator != std::default_sentinel)
            ;
        };
        static_assert(f());
    }

    constexpr auto byte_view_iterators_can_be_compared_to_each_other()
    {
        auto f = []
        {
            auto const some_bits = std::array<bool, 16>{1,1,0,0,0,1,1,1, 0,1,0,1,1,0,0,1};

            auto const any_iterator = byte_view_iterator{begin(some_bits), end(some_bits)};
            auto const same_iterator = byte_view_iterator{begin(some_bits), end(some_bits)};
            auto const other_iterator = byte_view_iterator{begin(some_bits)+1, end(some_bits)};

            return (any_iterator == any_iterator)
                && !(any_iterator != any_iterator)
                && (any_iterator == same_iterator)
                && !(any_iterator != same_iterator)
                && any_iterator != other_iterator
                && !(any_iterator == other_iterator);
            ;
        };
        static_assert(f());
    }
}

namespace qrcode::code::test
{
    constexpr auto byte_views_accept_bit_ranges_and_returns_bytes_by_interpreting_the_bits_most_significant_first()
    {
        auto f = []() constexpr
        {
            auto const some_bits = std::array<bool, 16>{0,1,0,1,1,0,1,0, 1,0,1,0,1,1,0,0};

            auto const range = byte_view{some_bits};

            return std::ranges::equal(
                range, 
                std::array{std::byte{0b0101'1010}, std::byte{0b1010'1100}}
            );
        };
        static_assert(f());
    }

    constexpr auto byte_views_pad_zeros_if_given_bit_stream_is_not_a_multiple_of_bytes()
    {
        auto f = []() constexpr
        {
            auto const some_bits = std::array<bool, 42>{
                0, 0, 0, 1, 0, 0, 0, 0,
                0, 0, 0, 1, 1, 1, 1, 1, 
                0, 1, 1, 0, 0, 0, 1, 1, 
                1, 0, 0, 0, 0, 1, 0, 0, 
                1, 0, 1, 0, 0, 1, 0, 0, 
                0, 0
            };

            auto const range = byte_view{some_bits};

            return std::ranges::equal(
                range, 
                std::array{
                    std::byte{0x10}, std::byte{0x1f}, std::byte{0x63}, 
                    std::byte{0x84}, std::byte{0xa4}, std::byte{0x00}
                }
            );
        };
        static_assert(f());
    }

    constexpr auto byte_view_can_be_used_with_adaptors()
    {
        auto f = []() constexpr
        {
            auto const some_bits = std::array<bool, 16>{1,1,0,0,0,1,1,1, 0,1,0,1,1,0,0,1};

            auto const range = some_bits | views::byte;

            return std::ranges::equal(
                range, std::array{std::byte{0b1100'0111}, std::byte{0b0101'1001}}
            );
        };
        static_assert(f());
    }

    constexpr auto byte_view_can_be_used_in_adaptor_chains()
    {
        auto f = []() constexpr
        {
            auto const some_bits = std::array<bool, 16>{1,1,0,0,0,1,1,1, 0,1,0,1,1,0,0,1};
            
            auto const range = some_bits 
                | views::byte 
                | std::views::transform([](auto v) { return ~v; });

            return std::ranges::equal(
                range, std::array{std::byte{0b0011'1000}, std::byte{0b1010'0110}}
            );
        };
        static_assert(f());
    }
}
#endif