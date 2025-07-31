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
#include <ranges>
#include <qrcode/eci/message_header.h>

namespace qrcode::eci
{
    template<std::ranges::viewable_range Byte_Range>
    requires std::convertible_to<std::ranges::range_value_t<Byte_Range>, std::uint8_t> 
        || std::same_as<std::ranges::range_value_t<Byte_Range>, std::byte>
    class view : public std::ranges::view_interface<view<Byte_Range>>
    {
    public:
        constexpr explicit view(assignment_number eci_assignment_number, Byte_Range bytes)
        : header{std::move(eci_assignment_number)}
        , byte_range{std::move(bytes)}
        {
        }

        constexpr view() noexcept = default;

        [[nodiscard]] constexpr auto begin() noexcept
        {
            using std::ranges::begin;
            return begin(byte_range);
        }

        [[nodiscard]] constexpr auto end() noexcept 
        {
            using std::ranges::end;
            return end(byte_range);
        }

        [[nodiscard]] constexpr auto begin() const noexcept 
        requires std::ranges::range<Byte_Range const>
        {
            using std::ranges::begin;
            return begin(byte_range);
        }

        [[nodiscard]] constexpr auto end() const noexcept 
        requires std::ranges::range<Byte_Range const>
        { 
            using std::ranges::end;
            return end(byte_range);
        }

        [[nodiscard]] friend constexpr auto header(view const& view) noexcept
        {
            return view.header;
        }

    private:
        message_header header;
        Byte_Range byte_range;
    };

    template<std::ranges::viewable_range Range>
    view(assignment_number, Range&&) -> view<std::ranges::views::all_t<Range>>;
}

namespace qrcode
{
    template<class T> struct is_eci_view : std::false_type {};
    template<class T> struct is_eci_view<qrcode::eci::view<T>> : std::true_type {};
    template<class T> inline constexpr auto is_eci_view_v = is_eci_view<T>::value;
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::eci::test
{
    constexpr auto is_eci_view_returns_true_when_given_type_is_an_eci_view_false_otherwise()
    {
        static_assert(!is_eci_view_v<int>);
        static_assert(!is_eci_view_v<std::array<char,3>&>);
        static_assert(is_eci_view_v<view<std::array<char,4>&>>);
        static_assert(is_eci_view_v<view<std::array<std::byte,4>&>>);
    }
}
#endif