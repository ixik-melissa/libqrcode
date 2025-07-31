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

namespace qrcode::structure
{
    class dimension 
    { 
    public:
        explicit constexpr dimension(int size) noexcept
        : width(size), height(size)
        {
        }
        
        constexpr dimension(int the_width, int the_height) noexcept
        : width(the_width), height(the_height)
        {
        }

        [[nodiscard]] friend constexpr auto width(dimension const& size) noexcept { return size.width; }
        [[nodiscard]] friend constexpr auto height(dimension const& size) noexcept { return size.height; }

        [[nodiscard]] constexpr auto operator==(dimension const&) const noexcept -> bool = default;
        [[nodiscard]] constexpr auto operator!=(dimension const&) const noexcept -> bool = default;
            
    private:
        int width;
        int height; 
    };
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::structure::test
{
    constexpr auto dimensions_have_a_width()
    {
        constexpr auto any_width = 5;
        constexpr auto other_width = 9;
        constexpr auto any_height = 7;

        static_assert(width(dimension{any_width, any_height}) == any_width);
        static_assert(width(dimension{other_width, any_height}) == other_width);
    }

    constexpr auto dimensions_have_a_height()
    {
        constexpr auto any_width = 5;
        constexpr auto any_height = 7;
        constexpr auto other_height = 9;

        static_assert(height(dimension{any_width, any_height}) == any_height);
        static_assert(height(dimension{any_width, other_height}) == other_height);
    }

    constexpr auto dimensions_can_be_constructed_by_a_single_value_representing_width_and_height()
    {
        constexpr auto any_length = 5;
        constexpr auto other_length = 9;

        static_assert(width(dimension{any_length}) == any_length);
        static_assert(width(dimension{other_length}) == other_length);
        static_assert(height(dimension{any_length}) == any_length);
        static_assert(height(dimension{other_length}) == other_length);
    }
}
#endif