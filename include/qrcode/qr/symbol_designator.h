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

#include <array>
#include <string_view>
#include <qrcode/qr/error_correction.h>
#include <qrcode/qr/symbol_version.h>

namespace qrcode::qr
{
    struct symbol_designator
    {
        symbol_version version{1};
        error_correction error_level{error_correction::level_L};

        [[nodiscard]] constexpr auto operator==(symbol_designator const&) const noexcept -> bool = default;
        [[nodiscard]] constexpr auto operator!=(symbol_designator const&) const noexcept -> bool = default;
    };

    [[nodiscard]] constexpr auto version(symbol_designator const& designator) noexcept
    {
        return designator.version;
    }

    [[nodiscard]] constexpr auto error_level(symbol_designator const& designator) noexcept
    {
        return designator.error_level;
    }

    [[nodiscard]] constexpr auto str(symbol_designator const& designator) noexcept
    {
        auto to_char = [](auto digit) { return '\x30' + digit; };

        constexpr auto level_strings = std::array{'M', 'L', 'H', 'Q'};
        
        auto result = std::array<char,5>{};

        auto next = begin(result);
        if (version(designator).number > 9)
            *(next++) = to_char(version(designator).number/10);
        *(next++) = to_char(version(designator).number%10);
        *(next++) = '-';
        *(next++) = level_strings[number(error_level(designator))];
        *next = '\0';
        return result;
    }

    template<class Stream>
    [[nodiscard]] constexpr auto& operator<<(
        Stream& stream, symbol_designator const& designator) noexcept
    {
        using std::data;
        auto const string = str(designator);
        for (auto i : std::string_view{data(string)})
            stream << i;
        return stream;
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::qr::test
{
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

        static_assert(strcmp(str(symbol_designator{symbol_version{1}, error_correction::level_L}), "1-L"));
        static_assert(strcmp(str(symbol_designator{symbol_version{2}, error_correction::level_L}), "2-L"));
        static_assert(strcmp(str(symbol_designator{symbol_version{32}, error_correction::level_L}), "32-L"));
        
        static_assert(strcmp(str(symbol_designator{symbol_version{1}, error_correction::level_M}), "1-M"));
        static_assert(strcmp(str(symbol_designator{symbol_version{2}, error_correction::level_M}), "2-M"));
        static_assert(strcmp(str(symbol_designator{symbol_version{32}, error_correction::level_M}), "32-M"));

        static_assert(strcmp(str(symbol_designator{symbol_version{1}, error_correction::level_Q}), "1-Q"));
        static_assert(strcmp(str(symbol_designator{symbol_version{2}, error_correction::level_Q}), "2-Q"));
        static_assert(strcmp(str(symbol_designator{symbol_version{32}, error_correction::level_Q}), "32-Q"));

        static_assert(strcmp(str(symbol_designator{symbol_version{1}, error_correction::level_H}), "1-H"));
        static_assert(strcmp(str(symbol_designator{symbol_version{2}, error_correction::level_H}), "2-H"));
        static_assert(strcmp(str(symbol_designator{symbol_version{32}, error_correction::level_H}), "32-H"));
    }
}
#endif