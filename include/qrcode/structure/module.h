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

#include <cstdint>
#include <cassert>

namespace qrcode::structure
{
    class module
    {
    public:
        constexpr module() : value{0} {}
        constexpr auto operator!() noexcept -> module&
        {
            value ^= 0b1u;
            return *this;
        }

        constexpr operator bool() const noexcept { return value & 1; }

        friend constexpr auto is_free(module const& module) noexcept;
        friend constexpr auto is_function(module const& module) noexcept;
        friend constexpr auto is_data(module const& module) noexcept;

        friend constexpr auto make_function(bool value) noexcept;
        friend constexpr auto make_data(bool value) noexcept;

        [[nodiscard]] constexpr auto operator==(module const&) const noexcept -> bool = default;
        [[nodiscard]] constexpr auto operator!=(module const&) const noexcept -> bool = default;

        constexpr auto operator^=(bool rhs)
        {
            value ^= static_cast<value_type>(rhs);
            return *this;
        }

    private:
        constexpr module(unsigned int internal_value) 
        : value{static_cast<value_type>(internal_value)} 
        {}

        using value_type = std::uint8_t;
        value_type value;
    };

    [[nodiscard]] constexpr auto is_free(module const& module) noexcept 
    { 
        return (module.value >> 1) == 0; 
    }

    [[nodiscard]] constexpr auto is_function(module const& module) noexcept 
    { 
        return (module.value >> 1) == 1; 
    }

    [[nodiscard]] constexpr auto is_data(module const& module) noexcept 
    { 
        return (module.value >> 2) == 1; 
    }

    [[nodiscard]] constexpr auto make_free() noexcept 
    { 
        return module{}; 
    }

    [[nodiscard]] constexpr auto make_function(bool value) noexcept 
    { 
        return module{value ? 0b011u : 0b010u}; 
    }

    [[nodiscard]] constexpr auto make_data(bool value) noexcept 
    { 
        return module{value ? 0b101u : 0b100u}; 
    }

    [[nodiscard]] constexpr auto operator^(module a, bool b)
    {
        if (!is_free(a))
            a ^= b;
        return a;
    }

    [[nodiscard]] constexpr auto operator==(module const& a, bool b)
    {
        return static_cast<bool>(a) == b;
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::structure::test
{
    constexpr auto module_is_free_by_default()
    {
        static_assert(is_free(module{}));
    }

    constexpr auto free_module_are_free()
    {
        static_assert(is_free(make_free()));
    }

    constexpr auto data_modules_are_data_modules()
    {
        static_assert(is_data(make_data(false)));
        static_assert(is_data(make_data(true)));
    }

    constexpr auto data_modules_are_no_function_modules()
    {
        static_assert(!is_function(make_data(false)));
        static_assert(!is_function(make_data(true)));
    }

    constexpr auto data_modules_are_not_free()
    {
        static_assert(!is_free(make_data(false)));
        static_assert(!is_free(make_data(true)));
    }

    constexpr auto function_modules_are_no_data_modules()
    {
        static_assert(!is_data(make_function(false)));
        static_assert(!is_data(make_function(true)));
    }

    constexpr auto function_modules_are_function_modules()
    {
        static_assert(is_function(make_function(false)));
        static_assert(is_function(make_function(true)));
    }

    constexpr auto function_modules_are_not_free()
    {
        static_assert(!is_free(make_function(false)));
        static_assert(!is_free(make_function(true)));
    }

    constexpr auto modules_can_be_compared_to_each_other()
    {
        static_assert(make_free() == make_free());

        static_assert(make_function(false) == make_function(false));
        static_assert(make_function(true) == make_function(true));
        static_assert(!(make_function(false) == make_function(true)));
        static_assert(!(make_function(true) == make_function(false)));

        static_assert(make_data(true) == make_data(true));
        static_assert(make_data(true) == make_data(true));
        static_assert(!(make_data(false) == make_data(true)));
        static_assert(!(make_data(true) == make_data(false)));

        static_assert(!(make_free() == make_function(false)));
        static_assert(!(make_free() == make_function(true)));
        static_assert(!(make_free() == make_data(false)));
        static_assert(!(make_free() == make_data(true)));

        static_assert(!(make_data(false) == make_function(false)));
        static_assert(!(make_data(false) == make_function(true)));
        static_assert(!(make_data(true) == make_function(false)));
        static_assert(!(make_data(true) == make_function(true)));
    }

    constexpr auto modules_can_be_compared_to_each_other_by_inequality()
    {
        static_assert(!(make_free() != make_free()));

        static_assert(!(make_function(false) != make_function(false)));
        static_assert(!(make_function(true) != make_function(true)));
        static_assert(make_function(false) != make_function(true));
        static_assert(make_function(true) != make_function(false));

        static_assert(!(make_data(true) != make_data(true)));
        static_assert(!(make_data(true) != make_data(true)));
        static_assert(make_data(false) != make_data(true));
        static_assert(make_data(true) != make_data(false));

        static_assert(make_free() != make_function(false));
        static_assert(make_free() != make_function(true));
        static_assert(make_free() != make_data(false));
        static_assert(make_free() != make_data(true));

        static_assert(make_data(false) != make_function(false));
        static_assert(make_data(false) != make_function(true));
        static_assert(make_data(true) != make_function(false));
        static_assert(make_data(true) != make_function(true));
    }

    constexpr auto free_modules_always_convert_to_false()
    {
        static_assert(!static_cast<bool>(make_free()));
    }

    constexpr auto data_modules_can_be_converted_to_bool()
    {
        static_assert(!static_cast<bool>(make_data(false)));
        static_assert(static_cast<bool>(make_data(true)));
    }

    constexpr auto modules_can_be_compared_to_bool()
    {
        static_assert(make_free() == false);

        static_assert(make_data(false) == false);
        static_assert(make_data(true) == true);

        static_assert(make_function(false) == false);
        static_assert(make_function(true) == true);
    }

    constexpr auto function_modules_can_be_xored_with_given_bool()
    {
        auto f = [](auto a, auto b) 
        { 
            return (make_function(a) ^ b) == make_function(a ^ b); 
        };
        
        static_assert(f(false, false));
        static_assert(f(false, true));
        static_assert(f(true, false));
        static_assert(f(true, true));
    }

    constexpr auto data_modules_can_be_xored_with_given_bool()
    {
        auto f = [](auto a, auto b) 
        { 
            return (make_data(a) ^ b) == make_data(a ^ b); 
        };
        
        static_assert(f(false, false));
        static_assert(f(false, true));
        static_assert(f(true, false));
        static_assert(f(true, true));
    }

    constexpr auto free_modules_remain_free_after_xoring_them_with_any_bool()
    {
        static_assert(is_free(make_free() ^ false));
        static_assert(is_free(make_free() ^ true));

        static_assert(!is_function(make_free() ^ false));
        static_assert(!is_function(make_free() ^ true));
        
        static_assert(!is_data(make_free() ^ false));
        static_assert(!is_data(make_free() ^ true));
    }
}
#endif
