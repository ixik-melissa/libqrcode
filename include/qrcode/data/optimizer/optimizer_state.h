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

#include <algorithm>
#include <optional>

#include <qrcode/data/optimizer/mode.h>

namespace qrcode::data::optimizer
{
    template<std::forward_iterator Iterator>
    class optimizer_state
    {
    public:
        constexpr optimizer_state(mode encoder_type, Iterator start) noexcept
        : type{encoder_type}, begin{start}, end{start}, last{}
        {
        }

        [[nodiscard]] constexpr auto advance(
            std::optional<mode> const& encoder_type = std::nullopt) noexcept
        {
            auto copy = *this;
            copy.type = encoder_type.value_or(type);
            copy.last = std::optional{*end};
            ++copy.end;
            return copy;
        }

        [[nodiscard]] constexpr auto last_character() const noexcept
        {
            return last;
        }

        [[nodiscard]] constexpr auto encoder_type() const noexcept
        {
            return type;
        }

        [[nodiscard]] constexpr auto character_count() const noexcept
        {
            return std::distance(begin, end);
        }

        template<class Publish>
        [[nodiscard]] constexpr auto finalize(Publish publish, mode new_type) const
        {
            publish(std::ranges::subrange{begin, end}, type);
            return optimizer_state{new_type, end}.advance();
        }

        template<class Publish>
        [[nodiscard]] constexpr auto finalize(Publish publish) const
        {
            publish(std::ranges::subrange{begin, end}, type);
        }

    private:
        mode type;
        Iterator begin;
        Iterator end;
        std::optional<typename std::iterator_traits<Iterator>::value_type> last;
    };
}

#ifdef QRCODE_TESTS_ENABLED
#include <string_view>

namespace qrcode::data::optimizer::test
{
    constexpr auto optimizer_state_is_initialized_correctly()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_data = "foobar"sv;
            auto const any_mode = mode::byte;

            auto const state = optimizer_state{any_mode, begin(any_data)};

            return state.last_character() == std::nullopt
                && state.encoder_type() == any_mode
                && state.character_count() == 0;
        };
        static_assert(f());
    }

    constexpr auto optimizer_state_can_be_advanced()
    {
        auto f = []() constexpr
        {
            using namespace std::literals;
            auto const any_data = "foobar"sv;
            auto const any_mode = mode::byte;

            auto const state = optimizer_state{any_mode, begin(any_data)}.advance().advance();

            return state.last_character() == 'o'
                && state.encoder_type() == any_mode
                && state.character_count() == 2;
        };
        static_assert(f());
    }

    constexpr auto optimizer_state_can_override_state_when_being_advanced()
    {
        auto f = []
        {
            using namespace std::literals;
            auto const any_data = "foobar"sv;
            auto const any_mode = mode::byte;
            auto const other_mode = mode::kanji;

            auto const state = optimizer_state{any_mode, begin(any_data)}.advance(other_mode);

            return state.last_character() == 'f'
                && state.encoder_type() == other_mode
                && state.character_count() == 1;
        };
        static_assert(f());
    }

    constexpr auto optimizer_state_can_be_finalized()
    {
        auto f = []() constexpr
        {
            using namespace std::literals;
            using std::ranges::begin;
            using std::ranges::end;
            auto const any_data = "foobar"sv;
            auto const any_mode = mode::byte;
            auto finalized_range = std::ranges::subrange{begin(any_data), begin(any_data)};
            auto finalized_mode = mode::numeric;

            auto state = optimizer_state{any_mode, begin(any_data)}.advance();
            state.finalize([&](auto range, auto mode)
            { 
                finalized_range = range;
                finalized_mode = mode;
            });

            return
                finalized_mode == any_mode
                && begin(finalized_range) == begin(any_data)
                && end(finalized_range) == (begin(any_data)+1);
        };
        static_assert(f());
    }

    constexpr auto optimizer_state_creates_a_new_state_when_next_mode_is_given_during_finalized()
    {
        auto f = []() constexpr
        {
            using namespace std::literals;
            using std::ranges::begin;
            using std::ranges::end;
            auto const any_data = "foobar"sv;
            auto const any_mode = mode::byte;
            auto const next_mode = mode::alphanumeric;
            auto finalized_range = std::ranges::subrange{begin(any_data), begin(any_data)};
            auto finalized_mode = mode::numeric;

            auto state = optimizer_state{any_mode, begin(any_data)}.advance();
            auto const next_state = state.finalize([&](auto range, auto mode)
            { 
                finalized_range = range;
                finalized_mode = mode;
            }, next_mode);

            return next_state.encoder_type() == next_mode
                && next_state.last_character() == 'o'
                && next_state.character_count() == 1
                && finalized_mode == any_mode
                && begin(finalized_range) == begin(any_data)
                && end(finalized_range) == (begin(any_data)+1);
        };
        static_assert(f());
    }
}
#endif
