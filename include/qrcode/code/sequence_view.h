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

#include <algorithm>
#include <ranges>

#include <qrcode/code/code_block.h>
#include <qrcode/code/sequence.h>
#include <qrcode/code/sequence_description.h>

namespace qrcode::code
{
    template<std::ranges::viewable_range Range>
    requires std::is_same_v<
        std::ranges::range_value_t<Range>, 
        code_block<typename std::ranges::range_value_t<Range>::value_type>
    >
    [[nodiscard]] constexpr auto sequence_view(
        Range&& range, sequence_description const& description) noexcept
    {
        using std::ranges::reverse_view;
        using value_type = std::ranges::range_value_t<Range>::value_type;

        auto code_sequence = sequence<value_type>{description};

        auto space_left = total_blocks(description);
        for (auto i = begin(range); i != end(range) && space_left > 0; ++i, --space_left)
        {
            auto const block = *i;
            code_sequence.next(reverse_view{block.data}, reverse_view{block.error});
        }

        auto const sequence = code_sequence.get();
        return std::views::iota(0u, size(sequence)) | std::views::transform([sequence](auto index)
        {
            return sequence[index];
        });
    }
}

namespace qrcode::code::views::detail
{
    struct sequence_view_adaptor
    {
        sequence_description description;
    };

    template<std::ranges::viewable_range Range>
    [[nodiscard]] constexpr auto operator|(Range&& range, sequence_view_adaptor adaptor) noexcept
    {
        return sequence_view(std::forward<Range>(range), adaptor.description);
    }
}

namespace qrcode::code::views
{
    inline constexpr auto sequence = [](sequence_description description)
    {
        using qrcode::code::views::detail::sequence_view_adaptor;
        return sequence_view_adaptor{std::move(description)};
    };
}

namespace qrcode::views
{
    using qrcode::code::views::sequence;
}
