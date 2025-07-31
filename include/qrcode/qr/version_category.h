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

#include <qrcode/qr/symbol_version.h>

namespace qrcode::qr
{
    enum class version_category { small, medium, large };

    [[nodiscard]] constexpr auto number(version_category category) noexcept
    {
        return static_cast<int>(category);
    }

    [[nodiscard]] constexpr auto version_categories() noexcept
    {
        return std::array{
            version_category::small, version_category::medium, version_category::large};
    }

    [[nodiscard]] constexpr auto make_version_category(symbol_version version) noexcept
    {
        return version.number < 10 
            ? version_category::small 
            : (version.number < 27 ? version_category::medium : version_category::large);
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::qr::test
{
    constexpr auto number_converts_given_version_category_into_index()
    {
        static_assert(number(version_category::small) == 0);
        static_assert(number(version_category::medium) == 1);
        static_assert(number(version_category::large) == 2);
    }

    constexpr auto version_category_can_be_created_from_given_symbol_version()
    {
        static_assert(make_version_category(symbol_version{1}) == version_category::small);
        static_assert(make_version_category(symbol_version{2}) == version_category::small);
        static_assert(make_version_category(symbol_version{3}) == version_category::small);
        static_assert(make_version_category(symbol_version{4}) == version_category::small);
        static_assert(make_version_category(symbol_version{5}) == version_category::small);
        static_assert(make_version_category(symbol_version{6}) == version_category::small);
        static_assert(make_version_category(symbol_version{7}) == version_category::small);
        static_assert(make_version_category(symbol_version{8}) == version_category::small);
        static_assert(make_version_category(symbol_version{9}) == version_category::small);

        static_assert(make_version_category(symbol_version{10}) == version_category::medium);
        static_assert(make_version_category(symbol_version{11}) == version_category::medium);
        static_assert(make_version_category(symbol_version{12}) == version_category::medium);
        static_assert(make_version_category(symbol_version{13}) == version_category::medium);
        static_assert(make_version_category(symbol_version{14}) == version_category::medium);
        static_assert(make_version_category(symbol_version{15}) == version_category::medium);
        static_assert(make_version_category(symbol_version{16}) == version_category::medium);
        static_assert(make_version_category(symbol_version{17}) == version_category::medium);
        static_assert(make_version_category(symbol_version{18}) == version_category::medium);
        static_assert(make_version_category(symbol_version{19}) == version_category::medium);
        static_assert(make_version_category(symbol_version{20}) == version_category::medium);
        static_assert(make_version_category(symbol_version{21}) == version_category::medium);
        static_assert(make_version_category(symbol_version{22}) == version_category::medium);
        static_assert(make_version_category(symbol_version{23}) == version_category::medium);
        static_assert(make_version_category(symbol_version{24}) == version_category::medium);
        static_assert(make_version_category(symbol_version{25}) == version_category::medium);
        static_assert(make_version_category(symbol_version{26}) == version_category::medium);

        static_assert(make_version_category(symbol_version{27}) == version_category::large);
        static_assert(make_version_category(symbol_version{28}) == version_category::large);
        static_assert(make_version_category(symbol_version{29}) == version_category::large);
        static_assert(make_version_category(symbol_version{30}) == version_category::large);
        static_assert(make_version_category(symbol_version{31}) == version_category::large);
        static_assert(make_version_category(symbol_version{32}) == version_category::large);
        static_assert(make_version_category(symbol_version{33}) == version_category::large);
        static_assert(make_version_category(symbol_version{34}) == version_category::large);
        static_assert(make_version_category(symbol_version{35}) == version_category::large);
        static_assert(make_version_category(symbol_version{36}) == version_category::large);
        static_assert(make_version_category(symbol_version{37}) == version_category::large);
        static_assert(make_version_category(symbol_version{38}) == version_category::large);
        static_assert(make_version_category(symbol_version{39}) == version_category::large);
        static_assert(make_version_category(symbol_version{40}) == version_category::large);
    }
}
#endif
