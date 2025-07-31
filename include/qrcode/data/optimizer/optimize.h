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

#include <qrcode/data/optimizer/mode.h>
#include <qrcode/data/optimizer/dispatch_mode.h>
#include <qrcode/data/optimizer/optimizer_state.h>
#include <qrcode/data/bit_stream.h>
#include <qrcode/data/encoders.h>

namespace qrcode::data::optimizer
{
    template<class Iterator, class Sentinel>
    struct segment
    {
        mode encoder;
        std::ranges::subrange<Iterator, Sentinel> range;
    };

    using qrcode::data::nonull_encoders;

    template<class Publish>
    [[nodiscard]] constexpr auto publish_encoder(
        nonull_encoders const& encoders, Publish publish) noexcept
    {
        return [=](auto const& to_be_published)
        {
            switch(to_be_published.encoder)
            {
                case mode::undefined: break;
                case mode::numeric: publish(to_be_published.range, encoders.numeric); break;
                case mode::alphanumeric: publish(to_be_published.range,encoders.alphanumeric);break;
                case mode::byte: publish(to_be_published.range, encoders.byte); break;
                case mode::kanji: publish(to_be_published.range, encoders.kanji); break;
            }
        };
    }

    template<class Iterator, class Sentinel, class Publish>
    constexpr auto publish_merged(
        segment<Iterator, Sentinel>* last_published, Publish publish) noexcept
    {
        return [=](auto const& data_range, auto selected_mode)
        {
            using std::ranges::begin;
            using std::ranges::end;
            using std::ranges::empty;

            auto const last_empty = empty(last_published->range);
            if (!last_empty && last_published->encoder == selected_mode)
            {
                last_published->range = 
                    std::ranges::subrange{begin(last_published->range), end(data_range)};
                return;
            }

            if (!last_empty)
                publish(*last_published);

            *last_published = {.encoder=selected_mode, .range=data_range};
        };
    }

    template<class Message, class Publish>
    requires std::convertible_to<std::ranges::range_value_t<Message>, char>
    [[nodiscard]] constexpr auto optimize(
        Message&& message, nonull_encoders const& encoders, Publish publish) noexcept
    {
        using iterator = std::ranges::iterator_t<Message>;
        using sentinel = std::ranges::sentinel_t<Message>;
        using std::ranges::begin;
        using std::ranges::end;
        using std::ranges::empty;

        auto publish_with_encoder = publish_encoder(encoders, std::move(publish));

        auto last_published = segment<iterator, sentinel>{
            .encoder=mode::undefined, 
            .range=std::ranges::subrange{begin(message), end(message)}
        };

        auto merged_publish = publish_merged(&last_published, publish_with_encoder);
        auto dispatch = dispatch_mode(merged_publish);
        auto state = optimizer_state<iterator>{mode::undefined, begin(message)};

        for (auto const& i : message)
            state = dispatch(state, i);

        if (state.character_count() != 0)
            state.finalize(merged_publish);

        if (!empty(last_published.range))
            publish_with_encoder(last_published);
    }
}
