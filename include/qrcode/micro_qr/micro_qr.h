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
#include <concepts>
#include <optional>
#include <ranges>

#include <qrcode/result.h>
#include <qrcode/symbol.h>

#include <qrcode/micro_qr/best_version.h>
#include <qrcode/micro_qr/raw_code.h>
#include <qrcode/micro_qr/code_bits.h>
#include <qrcode/micro_qr/code_capacity.h>
#include <qrcode/micro_qr/error_correction.h>
#include <qrcode/micro_qr/finalize_symbol.h>
#include <qrcode/micro_qr/encoders.h>
#include <qrcode/micro_qr/symbol_version.h>

#include <qrcode/data/best_fit/data_encoding.h>
#include <qrcode/data/best_fit/data_length.h>

namespace qrcode::micro_qr
{
    using qrcode::structure::module;

    template<class Module = module, std::ranges::forward_range Message>
    requires std::convertible_to<std::ranges::range_value_t<Message>, char>
    [[nodiscard]] constexpr auto make_symbol(
        Message&& message, 
        symbol_version version, 
        std::optional<error_correction> error_level = std::nullopt
    ) noexcept
    {
        using symbol_type = symbol<Module, symbol_designator>;
        using qrcode::data::best_fit::encode_data;
        using qrcode::data::best_fit::data_length;
        using qrcode::data::bit_stream;

        auto const symbol_designator = make_designator(version, error_level);
        if (!symbol_designator)
            return result<symbol_type>{error_code::version_and_error_level_not_supported}; 
        
        auto const encoders = available_encoders(version);
        auto const needed_size = data_length(encoders, message);
        auto const capacity = make_code_capacity(*symbol_designator);
        
        if (needed_size > total_data_bits(capacity))
            return result<symbol_type>{error_code::data_too_large};

        auto stream = bit_stream{};
        [[maybe_unused]] auto const success = encode_data(stream, encoders, message);
        assert(success);
        
        return result<symbol_type>{finalize(
            make_raw_code<Module>(version, code_bits(stream.get(), capacity)), 
            *symbol_designator
        )};
    }

    template<class Module = module, std::ranges::forward_range Message>
    requires std::convertible_to<std::ranges::range_value_t<Message>, char>
    [[nodiscard]] constexpr auto make_symbol(
        Message&& message, std::optional<error_correction> error_level = std::nullopt) noexcept
    {
        using symbol_type = symbol<Module, symbol_designator>;
        using qrcode::data::best_fit::encode_data;
        using qrcode::data::best_fit::data_length;
        using qrcode::data::bit_stream;

        constexpr auto all_versions = versions();
        auto filtered = all_versions 
            | std::views::filter([=](auto v){ return v != symbol_version::M1 || error_level; })
            | std::views::filter([=](auto v){ return v == symbol_version::M4 || error_level != error_correction::level_Q; });
        
        for (auto i : filtered)
        {
            auto const encoders = available_encoders(i);
            auto const needed_size = data_length(encoders, message);
            if (!needed_size)
                continue;

            auto const selected_version = best_version(error_level, *needed_size);
            if (!selected_version || *selected_version != i)
                continue;

            auto const designator = make_designator(*selected_version, error_level);
            assert(designator != std::nullopt);

            auto stream = bit_stream{};
            [[maybe_unused]] auto const success = encode_data(stream, encoders, message);
            assert(success);
            
            return result{finalize(
                make_raw_code<Module>(
                    *selected_version, 
                    code_bits(stream.get(), make_code_capacity(*designator))),
                *designator
            )};    
        }
        return result<symbol_type>{error_code::data_too_large};
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>
#include <qrcode/structure/make_matrix.h>

namespace qrcode::micro_qr
{
    constexpr auto micro_qr_symbols_can_be_generated_from_given_version_and_error_level()
    {
        using qrcode::structure::make_matrix;
        using namespace std::literals;
        constexpr auto any_data = "01234567"sv;
        constexpr auto any_version = symbol_version::M2;
        constexpr auto any_error_level = error_correction::level_L;
        constexpr auto selected_mask_id = 1;

        constexpr auto s = make_symbol<char>(any_data, any_version, any_error_level).value();

        static_assert(s == symbol{
            *make_designator(any_version, any_error_level),
            selected_mask_id,
            make_matrix<char>({13,13},
                "*******-*-*-*"
                "*-----*-*++,+"
                "*-***-*--++,+"
                "*-***-*--++++"
                "*-***-*-*++,,"
                "*-----*-*,,,+"
                "*******--++++"
                "---------++,,"
                "**-*----*,,,+"
                "-++,+,+,+,+,+"
                "*++,,+++++++,"
                "-,,+,+,,,,++,"
                "*++,+,,++,+++"sv
            )
        });
    }

    constexpr auto micro_qr_symbols_can_be_generated_from_given_version_and_error_level2()
    {
        using qrcode::structure::make_matrix;
        using namespace std::literals;
        constexpr auto any_data = "Wikipedia"sv;
        constexpr auto any_error_level = error_correction::level_L;
        constexpr auto selected_version = symbol_version::M3;
        constexpr auto selected_mask_id = 2;

        constexpr auto s = make_symbol<char>(any_data, any_error_level).value();

        static_assert(s == symbol{
            *make_designator(selected_version, any_error_level),
            selected_mask_id,
            make_matrix<char>({15,15},
                "*******-*-*-*-*"
                "*-----*--,,+,+,"
                "*-***-*-*++++++"
                "*-***-*-*,+,,++"
                "*-***-*--,,+,++"
                "*-----*-*,+,+,+"
                "*******--+,,++,"
                "---------+,+,,+"
                "******---,+,+,+"
                "-,,+,,++,,,,+++"
                "*++,++,++++,+++"
                "-+,,,,+,,+++,+,"
                "*,+++++,+++,+,+"
                "-+,,,,+,,+,+++,"
                "*++,++,+,+,++++"sv
            )
        });
    }
}
#endif
