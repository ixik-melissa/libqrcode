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

#include <optional>

#include <qrcode/data/best_fit/best_encoder.h>
#include <qrcode/data/bit_stream.h>

namespace qrcode::data::best_fit
{
    template<class Message>
    [[nodiscard]] constexpr auto encode_data(
        bit_stream& stream, nullable_encoders const& encoders, Message&& message) noexcept
    {
        auto const encoder = best_encoder(message, encoders);
        if (!encoder)
           return false;

        std::visit([&](auto&& encoder){ encoder.write_segment(stream, message); }, *encoder);
        return true;
    }

    template<class Message>
    [[nodiscard]] constexpr auto encode_data(
        bit_stream& stream, nonull_encoders const& encoders, Message&& message) noexcept
    {
        auto const encoder = best_encoder(message, encoders);
        std::visit([&](auto&& encoder){ encoder.write_segment(stream, message); }, encoder);
    }
}

