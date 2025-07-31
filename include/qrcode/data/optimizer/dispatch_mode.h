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
#include <qrcode/data/optimizer/determine_mode.h>
#include <qrcode/data/optimizer/optimizer_state.h>

namespace qrcode::data::optimizer
{
    [[nodiscard]] constexpr auto consecutive_run() noexcept
    {
        return 10;
    }

    template<class Publish>
    [[nodiscard]] constexpr auto on_numeric(Publish publish) noexcept
    {
        return [=](auto state, auto character)
        {
            auto const current_mode = determine_mode(character);

            if (current_mode == mode::numeric)
                return state.advance();

            if (state.character_count() < consecutive_run())
            {
                return state.advance(
                    current_mode == mode::alphanumeric ? mode::alphanumeric : mode::byte);
            }
                
            return state.finalize(publish, current_mode);
        };
    }

    template<class Publish>
    [[nodiscard]] constexpr auto on_alphanumeric(Publish publish) noexcept
    {
        return [=](auto state, auto character)
        {
            auto const current_mode = determine_mode(character);

            if (current_mode == mode::alphanumeric || current_mode == mode::numeric)
                return state.advance();
            
            if (state.character_count() < consecutive_run())
                return state.advance(mode::byte);
            
            return state.finalize(publish, current_mode);
        };
    }

    template<class Publish>
    [[nodiscard]] constexpr auto on_byte(Publish publish) noexcept
    {
        return [=](auto state, auto character)
        {
            auto const current_mode = determine_mode(character);
            if (state.character_count() < consecutive_run() || current_mode == mode::byte)
                return state.advance();

            return state.finalize(publish, current_mode);
        };
    }

    template<class Publish>
    [[nodiscard]] constexpr auto on_kanji(Publish publish) noexcept
    {
        return [=](auto state, auto character)
        {
            using qrcode::data::is_kanji;
            using qrcode::data::is_potential_kanji;

            if (state.character_count() % 2)
            {
                if (auto const last = state.last_character(); last && is_kanji(*last, character))
                    return state.advance();
                return state.advance(mode::byte);
            }

            if (is_potential_kanji(character))
                return state.advance(mode::kanji);

            if (state.character_count() < consecutive_run())
                return state.advance(mode::byte);

            return state.finalize(publish, determine_mode(character));
        };
    }

    [[nodiscard]] constexpr auto on_undefined() noexcept
    {
        return [](auto state, auto character)
        {
            return state.advance(determine_mode(character));
        };
    }

    template<class Publish>
    [[nodiscard]] constexpr auto dispatch_mode(Publish publish)
    {
        return [
            undefined = on_undefined(),
            alphanumeric = on_alphanumeric(publish),
            numeric = on_numeric(publish),
            byte = on_byte(publish),
            kanji = on_kanji(publish)
        ](auto state, auto character)
        {
            switch(state.encoder_type())
            {
                case mode::undefined: return undefined(std::move(state), character);
                case mode::numeric: return numeric(std::move(state), character);
                case mode::alphanumeric: return alphanumeric(std::move(state), character);
                case mode::byte: return byte(std::move(state), character);
                case mode::kanji: return kanji(std::move(state), character);
            }
            return state;
        };
    }
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>

namespace qrcode::data::optimizer::test
{
    constexpr auto on_numeric_stays_in_numeric_mode_as_long_as_numerics_are_passed()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_data = "0123456789"sv;
            auto no_publish = [](auto&&...){};
            auto numeric = on_numeric(no_publish);

            auto state = optimizer_state{mode::numeric, begin(any_data)};
            auto count = 1;
            for (auto i : any_data)
            {
                state = numeric(state, i);
                if (state.character_count() != (count++) ||
                    state.encoder_type() != mode::numeric ||
                    state.last_character() != i)
                {
                    return false;
                }
            }
            return true;
        };
        static_assert(f());
    }

    constexpr auto on_numeric_switchs_to_alphanumeric_mode_if_alphanumeric_is_passed_and_minimum_number_of_numerics_has_not_been_reached_yet()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_data = "0A"sv;
            auto const initial_state = optimizer_state{mode::numeric, begin(any_data)};
            auto no_publish = [](auto&&...){};
            auto numeric = on_numeric(no_publish);

            auto const next_state = 
                numeric(numeric(initial_state, *begin(any_data)), *(begin(any_data)+1));

            return next_state.character_count() == 2 
                && next_state.encoder_type() == mode::alphanumeric
                && next_state.last_character() == 'A';
        };
        static_assert(f());
    }

    constexpr auto on_numeric_switchs_to_byte_mode_if_minimum_number_of_numerics_has_not_been_reached_and_given_character_is_not_an_alphanumeric()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_data = "0\x10"sv;
            auto const initial_state = optimizer_state{mode::numeric, begin(any_data)};
            auto no_publish = [](auto&&...){};
            auto numeric = on_numeric(no_publish);

            auto const next_state = 
                numeric(numeric(initial_state, *begin(any_data)), *(begin(any_data)+1));

            return next_state.character_count() == 2 
                && next_state.encoder_type() == mode::byte
                && next_state.last_character() == '\x10';
        };
        static_assert(f());
    }

    constexpr auto on_numeric_switchs_to_byte_mode_if_minimum_number_of_numerics_has_not_been_reached_and_given_character_is_a_kanji_byte()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_data = "4\x93"sv;
            auto const initial_state = optimizer_state{mode::numeric, begin(any_data)};
            auto no_publish = [](auto&&...){};
            auto numeric = on_numeric(no_publish);

            auto const next_state = 
                numeric(numeric(initial_state, *begin(any_data)), *(begin(any_data)+1));

            return next_state.character_count() == 2 
                && next_state.encoder_type() == mode::byte
                && next_state.last_character() == '\x93';
        };
        static_assert(f());
    }

    constexpr auto on_numeric_publishes_numeric_range_if_minimum_number_of_numerics_has_been_reached()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_data = "0123456789\x10"sv;
            auto ok = false;
            auto publish = [&](auto const& range, auto encoder)
            {
                using std::ranges::begin;
                using std::ranges::end;
                ok = begin(range) == begin(any_data) 
                    && end(range) == (begin(any_data)+10)
                    && encoder == mode::numeric;
            };
            auto numeric = on_numeric(publish);

            auto state = optimizer_state{mode::numeric, begin(any_data)};
            for (auto i : any_data)
                state = numeric(state, i);

            return ok;
        };
        static_assert(f());
    }

    constexpr auto on_numeric_returns_new_state_having_mode_that_fits_last_character_best()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_data = "0123456789\x10"sv;
            auto no_publish = [](auto&&...){};
            auto numeric = on_numeric(no_publish);

            auto state = optimizer_state{mode::numeric, begin(any_data)};
            for (auto i : any_data)
                state = numeric(state, i);

            return state.character_count() == 1 
                && state.encoder_type() == mode::byte
                && state.last_character() == '\x10';
        };
        static_assert(f());
    }

    constexpr auto on_alphanumeric_stays_in_alphanumeric_mode_as_long_as_alphanumerics_are_passed()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_data = "A1B3C5D7E9"sv;
            auto no_publish = [](auto&&...){};
            auto alphanumeric = on_alphanumeric(no_publish);

            auto state = optimizer_state{mode::alphanumeric, begin(any_data)};
            auto count = 1;
            for (auto i : any_data)
            {
                state = alphanumeric(state, i);
                if (state.character_count() != (count++) ||
                    state.encoder_type() != mode::alphanumeric ||
                    state.last_character() != i)
                {
                    return false;
                }
            }
            return true;
        };
        static_assert(f());
    }

    constexpr auto on_alphanumeric_switchs_to_byte_mode_if_neither_numeric_nor_alphanumeric_is_passed_and_minimum_number_of_alphanumerics_has_not_been_reached_yet()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_data = "0A\x70"sv;
            auto const initial_state = optimizer_state{mode::alphanumeric, begin(any_data)};
            auto no_publish = [](auto&&...){};
            auto alphanumeric = on_alphanumeric(no_publish);
            auto b = begin(any_data);

            auto const next_state = 
                alphanumeric(alphanumeric(alphanumeric(initial_state, *b), *(b+1)), *(b+2));

            return next_state.character_count() == 3 
                && next_state.encoder_type() == mode::byte
                && next_state.last_character() == '\x70';
        };
        static_assert(f());
    }

    constexpr auto on_alphanumeric_publishes_alphanumeric_range_if_minimum_number_of_alphanumerics_has_been_reached()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_data = "HELLO WORLD\x10"sv;
            auto ok = false;
            auto publish = [&](auto const& range, auto encoder)
            {
                using std::ranges::begin;
                using std::ranges::end;
                ok = begin(range) == begin(any_data) 
                    && end(range) == (begin(any_data)+11)
                    && encoder == mode::alphanumeric;
            };
            auto alphanumeric = on_alphanumeric(publish);

            auto state = optimizer_state{mode::alphanumeric, begin(any_data)};
            for (auto i : any_data)
                state = alphanumeric(state, i);

            return ok;
        };
        static_assert(f());
    }

    constexpr auto on_alphanumeric_returns_new_state_having_mode_that_fits_last_character_best()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_data = "HELLO56789\x10"sv;
            auto no_publish = [](auto&&...){};
            auto alphanumeric = on_alphanumeric(no_publish);

            auto state = optimizer_state{mode::numeric, begin(any_data)};
            for (auto i : any_data)
                state = alphanumeric(state, i);

            return state.character_count() == 1 
                && state.encoder_type() == mode::byte
                && state.last_character() == '\x10';
        };
        static_assert(f());
    }

    constexpr auto on_byte_stays_in_byte_mode_as_long_as_bytes_are_passed()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_data = "\x01\x02\x03\x04\x05"sv;
            auto no_publish = [](auto&&...){};
            auto byte = on_byte(no_publish);

            auto state = optimizer_state{mode::byte, begin(any_data)};
            auto count = 1;
            for (auto i : any_data)
            {
                state = byte(state, i);
                if (state.character_count() != (count++) ||
                    state.encoder_type() != mode::byte ||
                    state.last_character() != i)
                {
                    return false;
                }
            }
            return true;
        };
        static_assert(f());
    }

    constexpr auto on_byte_stays_publishes_byte_range_if_minimum_number_of_bytes_has_been_reached()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_data = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x41"sv;
            auto ok = false;
            auto publish = [&](auto const& range, auto encoder)
            {
                using std::ranges::begin;
                using std::ranges::end;
                ok = begin(range) == begin(any_data) 
                    && end(range) == (begin(any_data)+10)
                    && encoder == mode::byte;
            };
            auto byte = on_byte(publish);

            auto state = optimizer_state{mode::byte, begin(any_data)};
            for (auto i : any_data)
                state = byte(state, i);

            return ok;
        };
        static_assert(f());
    }

    constexpr auto on_byte_returns_new_state_having_mode_that_fits_last_character_best()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_data = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x41"sv;
            auto no_publish = [](auto&&...){};
            auto byte = on_byte(no_publish);

            auto state = optimizer_state{mode::byte, begin(any_data)};
            for (auto i : any_data)
                state = byte(state, i);

            return state.character_count() == 1 
                && state.encoder_type() == mode::alphanumeric
                && state.last_character() == 'A';
        };
        static_assert(f());
    }

    constexpr auto on_kanji_stays_in_kanji_mode_as_long_as_kanjis_are_passed()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_data = "\x93\x5F\xE4\xAA"sv;
            auto no_publish = [](auto&&...){};
            auto kanji = on_kanji(no_publish);

            auto state = optimizer_state{mode::kanji, begin(any_data)};
            auto count = 1;
            for (auto i : any_data)
            {
                state = kanji(state, i);
                if (state.character_count() != (count++) ||
                    state.encoder_type() != mode::kanji ||
                    state.last_character() != i)
                {
                    return false;
                }
            }
            return true;
        };
        static_assert(f());
    }

    constexpr auto on_kanji_switchs_to_byte_mode_if_no_kanji_is_passed_and_minimum_number_of_kanjis_has_not_been_reached_yet()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_data = "\x93\x01"sv;
            auto const initial_state = optimizer_state{mode::kanji, begin(any_data)};
            auto no_publish = [](auto&&...){};
            auto kanji = on_kanji(no_publish);

            auto const next_state = 
                kanji(kanji(initial_state, *begin(any_data)), *(begin(any_data)+1));

            return next_state.character_count() == 2 
                && next_state.encoder_type() == mode::byte
                && next_state.last_character() == '\x01'
                ;
        };
        static_assert(f());
    }

    constexpr auto on_kanji_stays_publishes_kanji_range_if_minimum_number_of_kanjis_has_been_reached()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_data = "\x93\x5F\xE4\xAA\x93\x5F\xE4\xAA\x93\x5F\xE4\xAA\x01"sv;
            auto ok = false;
            auto publish = [&](auto const& range, auto encoder)
            {
                using std::ranges::begin;
                using std::ranges::end;
                ok = begin(range) == begin(any_data) 
                    && end(range) == (begin(any_data)+12)
                    && encoder == mode::kanji;
            };
            auto kanji = on_kanji(publish);

            auto state = optimizer_state{mode::kanji, begin(any_data)};
            for (auto i : any_data)
                state = kanji(state, i);

            return ok;
        };
        static_assert(f());
    }

    constexpr auto on_kanji_returns_new_state_having_mode_that_fits_last_character_best()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_data = "\x93\x5F\xE4\xAA\x93\x5F\xE4\xAA\x93\x5F\xE4\xAA\x01"sv;
            auto no_publish = [](auto&&...){};
            auto kanji = on_kanji(no_publish);

            auto state = optimizer_state{mode::kanji, begin(any_data)};
            for (auto i : any_data)
                state = kanji(state, i);

            return state.character_count() == 1 
                && state.encoder_type() == mode::byte
                && state.last_character() == '\x01';
        };
        static_assert(f());
    }
}

#endif
