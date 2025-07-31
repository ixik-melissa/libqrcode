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

#include <qrcode/qr/best_version.h>
#include <qrcode/qr/data_length.h>
#include <qrcode/qr/error_correction.h>
#include <qrcode/qr/version_category.h>

namespace qrcode::qr
{
    template<std::ranges::forward_range Message>
    requires std::convertible_to<std::ranges::range_value_t<Message>, char>
    [[nodiscard]] constexpr auto fit_version(
        Message&& message, error_correction error_level) noexcept
    {
        for (auto i : version_categories())
        {
            auto const needed_size = data_length(message, i);        
            auto const version = best_version(error_level, needed_size);
            if (version && make_version_category(*version) == i)
                return std::optional<symbol_version>{version};
        }
        return std::optional<symbol_version>{};
    }
}

