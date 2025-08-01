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

#include <ranges>
#include <concepts>

#include <qrcode/result.h>
#include <qrcode/symbol.h>

#include <qrcode/qr/raw_code.h>
#include <qrcode/qr/code_bits.h>
#include <qrcode/qr/code_capacity.h>
#include <qrcode/qr/data_encoding.h>
#include <qrcode/qr/data_length.h>
#include <qrcode/qr/error_correction.h>
#include <qrcode/qr/finalize_symbol.h>
#include <qrcode/qr/fit_version.h>
#include <qrcode/qr/symbol_version.h>
#include <qrcode/qr/version_category.h>
#include <qrcode/eci/view.h>

namespace qrcode::qr
{
    using qrcode::structure::module;

    template<class Module = module, std::ranges::forward_range Message>
    requires std::convertible_to<std::ranges::range_value_t<Message>, char>
    [[nodiscard]] constexpr auto make_symbol(
        Message&& message, symbol_version version, error_correction error_level) noexcept
    {
        using symbol_type = symbol<Module, symbol_designator>;

        auto const designator = symbol_designator{version, error_level};
        auto const category = make_version_category(version);
        auto const capacity = make_code_capacity(designator);

        auto const needed_size = data_length(message, category);        
        if (needed_size > total_data_bits(capacity))
            return result<symbol_type>{error_code::data_too_large};

        auto const stream = encode_data(std::forward<Message>(message), category);
        return result{finalize(
            make_raw_code<Module>(version, code_bits(stream.get(), capacity)),
            designator
        )};
    }

    template<class Module = module, std::ranges::forward_range Message>
    requires std::convertible_to<std::ranges::range_value_t<Message>, char>
    [[nodiscard]] constexpr auto make_symbol(
        Message&& message, error_correction error_level) noexcept
    {
        using symbol_type = symbol<Module, symbol_designator>;

        auto const version = fit_version(message, error_level);
        if (!version)
            return result<symbol_type>{error_code::data_too_large};

        auto const designator = symbol_designator{*version, error_level};
        auto const stream = encode_data(message, make_version_category(*version));
        return result{finalize(
            make_raw_code<Module>(
                *version, 
                code_bits(stream.get(), make_code_capacity(designator))),
            designator
        )};    
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>
#include <qrcode/structure/make_matrix.h>

namespace qrcode::qr
{
    constexpr auto qr_symbols_can_be_generated_from_given_version_and_error_level()
    {
        static_assert([]()
        {
            using qrcode::structure::make_matrix;
            using namespace std::literals;

            constexpr auto any_data = "01234567"sv;
            constexpr auto any_version = symbol_version{1};
            constexpr auto any_error_level = error_correction::level_M;
            constexpr auto selected_mask_id = 0;

            auto const s = make_symbol<char>(any_data, any_version, any_error_level).value();

            return s == symbol{
                symbol_designator{any_version, any_error_level},
                selected_mask_id,
                make_matrix<char>({21,21},
                    "*******--,+++-*******"
                    "*-----*-*++,,-*-----*"
                    "*-***-*--++,,-*-***-*"
                    "*-***-*--+,++-*-***-*"
                    "*-***-*-*+,++-*-***-*"
                    "*-----*--,,+,-*-----*"
                    "*******-*-*-*-*******"
                    "---------,,,,--------"
                    "*-*-*-*--,+,+---*--*-"
                    "++,+,,-,+,++,+,+,,,+,"
                    ",,,++,*++,++,+++,+++,"
                    "++,,++-+,+,+++,++,,+,"
                    ",,+,,+*+,+++,+++,,,,+"
                    "--------*,+,,,+,,,,+,"
                    "*******--,,,+,,,+,,,+"
                    "*-----*--,+,,,+,,+,++"
                    "*-***-*-*++,+,+,+++,+"
                    "*-***-*--+,+,+,+,+++,"
                    "*-***-*-*+,+,+++,,+,+"
                    "*-----*--,,+++,+++,,,"
                    "*******-*,,+,+++,,+,+"sv
                )
            };
        }());
    }

    constexpr auto qr_symbols_can_be_generated_from_given_error_level_and_message()
    {
        static_assert([]()
        {
            using qrcode::structure::make_matrix;
            using namespace std::literals;

            constexpr auto any_data = "ABRACADABRA"sv;
            constexpr auto any_version = symbol_version{1};
            constexpr auto any_error_level = error_correction::level_M;
            constexpr auto selected_mask_id = 7;

            auto const s = make_symbol<char>(any_data, any_error_level).value();

            return s == symbol{
                symbol_designator{any_version, any_error_level},
                selected_mask_id,
                make_matrix<char>({21,21},
                    "*******--,,,,-*******"
                    "*-----*--+,++-*-----*"
                    "*-***-*--+++,-*-***-*"
                    "*-***-*--,,++-*-***-*"
                    "*-***-*--+,++-*-***-*"
                    "*-----*-*,+,,-*-----*"
                    "*******-*-*-*-*******"
                    "---------+,,,--------"
                    "*--*-**-*,+++*-*-----"
                    "++,,+,-++,,+,++++++,,"
                    ",,++++*,,+++,++,,++,,"
                    ",,,,,+-+,,,,+++++,+++"
                    ",,++,,*,++++,,+,,+++,"
                    "--------*+,,,+++++,+,"
                    "*******--++++,+,++,,+"
                    "*-----*-*+,,,,+++++,,"
                    "*-***-*--++,+++,+,+,+"
                    "*-***-*-*,,,,++,,,+++"
                    "*-***-*--,++,++,,++,+"
                    "*-----*--,,++,,+,++,+"
                    "*******-*+,,,+,,+,+,,"sv
                )
            };
        }());
    }

    constexpr auto qr_symbol_generation_fails_if_data_is_too_large_for_given_symbol_version()
    {
        static_assert([]()
        {
            using namespace std::literals;
            constexpr auto any_data = "This is way too long. Sorry! This does not work!!!"sv;
            constexpr auto any_version = symbol_version{1};
            constexpr auto any_error_level = error_correction::level_L;

            auto const s = make_symbol<char>(any_data, any_version, any_error_level);

            return !s.has_value();
        }());
    }

    constexpr auto qr_symbols_support_eci_encoding()
    {
        static_assert([]()
        {
            using qrcode::structure::make_matrix;
            using namespace std::literals;

            constexpr auto any_eci_message = eci::view{eci::assignment_number{9}, "\xC1\xC2\xC3\xC4\xC5"sv};
            constexpr auto any_version = symbol_version{1};
            constexpr auto any_error_level = error_correction::level_L;
            constexpr auto selected_mask_id = 5;

            auto const s = make_symbol<char>(any_eci_message, any_version, any_error_level).value();

            return s == symbol{
                symbol_designator{any_version, any_error_level},
                selected_mask_id,
                make_matrix<char>({21,21},
                    "*******--,,,+-*******"
                    "*-----*--,+,+-*-----*"
                    "*-***-*--,+++-*-***-*"
                    "*-***-*-*,+,+-*-***-*"
                    "*-***-*-*+,,+-*-***-*"
                    "*-----*--+,+,-*-----*"
                    "*******-*-*-*-*******"
                    "---------,,,,--------"
                    "**---***-,,+,---**---"
                    ",,,+,,-+,,,+++,,++,++"
                    ",,+++,*,,,,,+,++,,++,"
                    "+,,,,+-,+,,++++,,,+,,"
                    "++,+,+*,+,,++++++,+++"
                    "--------*,,,+,,+,+,,,"
                    "*******-*,++,+,,,,++,"
                    "*-----*-*++,,,+,+,+++"
                    "*-***-*--,,+,+,,+,,,+"
                    "*-***-*--+,++++,,+,,,"
                    "*-***-*--,++++,,+++++"
                    "*-----*-*+,++++,,,+++"
                    "*******-*++,+,,,++,+,"sv
                )
            };
        }());
    }
}
#endif
