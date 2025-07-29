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

#include <optional>

#include <qrcode/micro_qr/error_correction.h>
#include <qrcode/micro_qr/symbol_version.h>

namespace qrcode::micro_qr
{
    class symbol_designator
    {
    public:
        [[nodiscard]] constexpr auto operator==(symbol_designator const&) const noexcept -> bool = default;
        [[nodiscard]] constexpr auto operator!=(symbol_designator const&) const noexcept -> bool = default;

        [[nodiscard]] friend constexpr auto version(symbol_designator const& designator) noexcept
        {
            return designator.version;
        }

        [[nodiscard]] friend constexpr auto error_level(symbol_designator const& designator) noexcept
        {
            return designator.error_level;
        }

        friend constexpr auto make_designator(
            symbol_version version, std::optional<error_correction> error_level) noexcept 
        -> std::optional<symbol_designator>;

    private:
        constexpr symbol_designator(
            symbol_version version_, std::optional<error_correction> error_level_) noexcept
        : version{version_}, error_level{std::move(error_level_)}
        {
        }

        symbol_version version;
        std::optional<error_correction> error_level;
    };

    [[nodiscard]] constexpr auto make_designator(
        symbol_version version, std::optional<error_correction> error_level
    ) noexcept -> std::optional<symbol_designator>
    {
        if (version == symbol_version::M1 && error_level)
            return std::nullopt;
        
        if (error_level == error_correction::level_Q && version != symbol_version::M4)
            return std::nullopt;

        if (version != symbol_version::M1 && !error_level)
            error_level = std::optional<error_correction>{error_correction::level_L};

        return symbol_designator{version, error_level};
    }

    [[nodiscard]] constexpr auto str(symbol_designator const& designator) noexcept
    {
        constexpr auto version_strings = std::array{"M1", "M2", "M3", "M4"};
        constexpr auto level_strings = std::array{'L', 'M', 'Q'};
        
        auto result = std::array<char,5>{};
        auto next = begin(result);

        auto v = version_strings[number(version(designator))];
        *(next++) = *(v++);
        *(next++) = *v;

        if (auto const error = error_level(designator))
        {
            *(next++) = '-';
            *(next++) = level_strings[number(*error)];
        }

        *next = '\0';
        return result;
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>

namespace qrcode::micro_qr::test
{
    constexpr auto designator_can_be_constructed_from_given_symbol_version_and_error_level()
    {
        auto f = [](auto symbol_version, auto error_correction_level)
        {
            auto designator = *make_designator(symbol_version, error_correction_level);

            return version(designator) == symbol_version 
                && error_level(designator) == error_correction_level;
        };
        
        static_assert(f(symbol_version::M1, std::nullopt));
        static_assert(f(symbol_version::M2, error_correction::level_L));
        static_assert(f(symbol_version::M2, error_correction::level_M));
        static_assert(f(symbol_version::M3, error_correction::level_L));
        static_assert(f(symbol_version::M3, error_correction::level_M));
        static_assert(f(symbol_version::M4, error_correction::level_L));
        static_assert(f(symbol_version::M4, error_correction::level_M));
        static_assert(f(symbol_version::M4, error_correction::level_Q));
    }

    constexpr auto designator_returns_nothing_if_symbol_verson_and_error_level_are_not_supported()
    {
        static_assert(!make_designator(symbol_version::M1, error_correction::level_L));
        static_assert(!make_designator(symbol_version::M1, error_correction::level_M));
        static_assert(!make_designator(symbol_version::M1, error_correction::level_Q));

        static_assert(!make_designator(symbol_version::M2, error_correction::level_Q));
        static_assert(!make_designator(symbol_version::M3, error_correction::level_Q));
    }

    constexpr auto designator_uses_error_level_L_if_not_provided_for_symbols_greather_than_M1()
    {
        static_assert(error_level(*make_designator(symbol_version::M2, std::nullopt)) == error_correction::level_L);
        static_assert(error_level(*make_designator(symbol_version::M3, std::nullopt)) == error_correction::level_L);
        static_assert(error_level(*make_designator(symbol_version::M4, std::nullopt)) == error_correction::level_L);
    }

    constexpr auto str_returns_given_designator_in_hyphen_notation()
    {
        auto strcmp = [](auto const& actual, auto const& nominal)
        {
            auto j = begin(actual);
            for (auto i : nominal)
                if (*(j++) != i)
                    return false;
            return true;
        };

        static_assert(strcmp(str(*make_designator(symbol_version::M1, std::nullopt)), "M1"));

        static_assert(strcmp(str(*make_designator(symbol_version::M2, error_correction::level_L)), "M2-L"));
        static_assert(strcmp(str(*make_designator(symbol_version::M2, error_correction::level_M)), "M2-M"));

        static_assert(strcmp(str(*make_designator(symbol_version::M3, error_correction::level_L)), "M3-L"));
        static_assert(strcmp(str(*make_designator(symbol_version::M3, error_correction::level_M)), "M3-M"));

        static_assert(strcmp(str(*make_designator(symbol_version::M4, error_correction::level_L)), "M4-L"));
        static_assert(strcmp(str(*make_designator(symbol_version::M4, error_correction::level_M)), "M4-M"));
        static_assert(strcmp(str(*make_designator(symbol_version::M4, error_correction::level_Q)), "M4-Q"));
    }
}
#endif