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

#include <cassert>
#include <optional>

namespace qrcode
{
    enum class error_code
    {
        data_too_large,
        version_does_not_support_given_data,
        version_and_error_level_not_supported
    };

    template<class Symbol>
    class result
    {
    public:
        constexpr result(Symbol success) noexcept 
        : symbol{std::move(success)}
        {
        }

        constexpr result(error_code error) noexcept 
        : fail{error}
        {
        }

        constexpr auto has_value() const noexcept
        {
            return symbol.has_value();
        }

        constexpr explicit operator bool() const noexcept
        {
            return has_value();
        }

        constexpr auto& value() const & noexcept 
        {
            assert(has_value());
            return *symbol;
        }

        constexpr auto&& value() const && noexcept 
        {
            assert(has_value());
            return std::move(*symbol);
        }

        constexpr auto error() const noexcept 
        {
            assert(!has_value());
            return fail;
        }

    private:
        std::optional<Symbol> symbol{};
        error_code fail{error_code::data_too_large};
    };
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::test
{
    constexpr auto result_has_a_value_if_constructed_with_symbol()
    {
        struct symbol_stub {};

        static_assert(result{symbol_stub{}}.has_value());
        static_assert(result{symbol_stub{}});
    }

    constexpr auto result_does_not_have_a_value_if_constructed_with_error_code()
    {
        struct symbol_stub {};
        constexpr auto any_error = error_code::data_too_large;

        static_assert(!result<symbol_stub>{any_error}.has_value());
        static_assert(!result<symbol_stub>{any_error});
    }

    constexpr auto result_can_return_error()
    {
        struct symbol_stub {};
        constexpr auto any_error = error_code::data_too_large;
        constexpr auto some_result = result<symbol_stub>{any_error};

        constexpr auto error = some_result.error();

        static_assert(error == any_error);
    }

    constexpr auto result_can_return_value_by_l_value_reference()
    {
        auto f = []
        {
            struct symbol_stub { constexpr auto operator==(symbol_stub const&) const -> bool = default; };
            auto const any_symbol = symbol_stub{};
            auto const some_result = result{any_symbol};

            auto&& value = some_result.value();

            return value == any_symbol 
                && std::is_lvalue_reference_v<decltype(value)>
                && std::is_same_v<std::decay_t<decltype(value)>, symbol_stub>;
        };
        static_assert(f());
    }

    constexpr auto result_can_return_value_by_r_value_reference()
    {
        auto f = []() constexpr
        {
            struct symbol_stub { constexpr auto operator==(symbol_stub const&) const -> bool = default; };
            auto const any_symbol = symbol_stub{};

            constexpr auto is_rvalue = std::is_rvalue_reference_v<decltype(result{any_symbol}.value())>;
            
            return result{any_symbol}.value() == any_symbol 
                && is_rvalue
                && std::is_same_v<std::decay_t<decltype(result{any_symbol}.value())>, symbol_stub>;
        };
        static_assert(f());
    }
}
#endif
