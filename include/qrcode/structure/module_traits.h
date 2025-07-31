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

#include <qrcode/structure/module.h>

namespace qrcode::structure
{
    template<class T>
    struct module_traits
    {
    };

    template<>
    struct module_traits<char>
    {
        static constexpr auto is_free = [](auto value) { return value == '.'; };
        static constexpr auto is_set = [](auto value) { return value == '*' || value == '+'; };
        static constexpr auto is_data = [](auto value) { return value == '+' || value == ','; };
        static constexpr auto is_function = [](auto value) { return value == '*' || value == '-'; };
        static constexpr auto make_function = [](auto bit) { return bit ? '*' : '-'; };
        static constexpr auto make_data = [](auto bit) { return bit ? '+' : ','; };
        static constexpr auto make_free = []() { return '.'; };
        static constexpr auto apply_mask = [](auto value, auto mask) { return mask ? make_data(!is_set(value)) : value; };
    };

    template<>
    struct module_traits<int>
    {
        static constexpr auto is_free = [](auto value) { return value == 0; };
        static constexpr auto is_set = [](auto value) { return value != 0; };
        static constexpr auto is_data = [](auto value) { return value == -1; };
        static constexpr auto is_function = [](auto value) { return value == 1; };
        static constexpr auto make_function = [](auto bit) { return bit ? -1 : 0; };
        static constexpr auto make_data = [](auto bit) { return bit ? 1 : 0; };
        static constexpr auto make_free = []() { return 0; };
        static constexpr auto apply_mask = [](auto value, auto mask) { return mask ? make_data(!is_set(value)) : value; };
    };

    template<>
    struct module_traits<module>
    {
        static constexpr auto is_free = [](auto value) { return qrcode::structure::is_free(value); };
        static constexpr auto is_set = [](auto value) { return static_cast<bool>(value); };
        static constexpr auto is_data = [](auto value) { return qrcode::structure::is_data(value); };
        static constexpr auto is_function = [](auto value) { return qrcode::structure::is_function(value); };
        static constexpr auto make_function = [](auto bit) { return qrcode::structure::make_function(bit); };
        static constexpr auto make_data = [](auto bit) { return qrcode::structure::make_data(bit); };
        static constexpr auto make_free = []() { return qrcode::structure::make_free(); };
        static constexpr auto apply_mask = [](auto value, auto mask) { return value ^ mask; };
    };
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::structure::test
{
    constexpr auto module_traits_define_a_set_of_functions_to_handle_the_modules()
    {
        static_assert(module_traits<module>::is_free(make_free()));
        static_assert(!module_traits<module>::is_free(make_function(false)));
        static_assert(!module_traits<module>::is_free(make_function(true)));
        static_assert(!module_traits<module>::is_free(make_data(false)));
        static_assert(!module_traits<module>::is_free(make_data(true)));

        static_assert(!module_traits<module>::is_set(make_free()));
        static_assert(!module_traits<module>::is_set(make_function(false)));
        static_assert(module_traits<module>::is_set(make_function(true)));
        static_assert(!module_traits<module>::is_set(make_data(false)));
        static_assert(module_traits<module>::is_set(make_data(true)));

        static_assert(!module_traits<module>::is_data(make_free()));
        static_assert(!module_traits<module>::is_data(make_function(false)));
        static_assert(!module_traits<module>::is_data(make_function(true)));
        static_assert(module_traits<module>::is_data(make_data(false)));
        static_assert(module_traits<module>::is_data(make_data(true)));

        static_assert(!module_traits<module>::is_function(make_free()));
        static_assert(module_traits<module>::is_function(make_function(false)));
        static_assert(module_traits<module>::is_function(make_function(true)));
        static_assert(!module_traits<module>::is_function(make_data(false)));
        static_assert(!module_traits<module>::is_function(make_data(true)));

        static_assert(module_traits<module>::make_free() == make_free());
        static_assert(module_traits<module>::make_function(false) == make_function(false));
        static_assert(module_traits<module>::make_function(true) == make_function(true));
        static_assert(module_traits<module>::make_data(false) == make_data(false));
        static_assert(module_traits<module>::make_data(true) == make_data(true));

        static_assert(module_traits<module>::apply_mask(make_function(false), false) == make_function(false));
        static_assert(module_traits<module>::apply_mask(make_function(false), true) == make_function(true));
        static_assert(module_traits<module>::apply_mask(make_function(true), true) == make_function(false));
        static_assert(module_traits<module>::apply_mask(make_function(true), false) == make_function(true));

        static_assert(module_traits<module>::apply_mask(make_data(false), false) == make_data(false));
        static_assert(module_traits<module>::apply_mask(make_data(false), true) == make_data(true));
        static_assert(module_traits<module>::apply_mask(make_data(true), true) == make_data(false));
        static_assert(module_traits<module>::apply_mask(make_data(true), false) == make_data(true));

        static_assert(module_traits<module>::apply_mask(make_free(), false) == make_free());
        static_assert(module_traits<module>::apply_mask(make_free(), true) == make_free());
    }
}
#endif
