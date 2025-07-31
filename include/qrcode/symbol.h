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

#include <qrcode/structure/matrix.h>
#include <qrcode/structure/horizontal_view.h>
#include <qrcode/structure/vertical_view.h>
#include <qrcode/structure/element_view.h>

namespace qrcode
{
    using qrcode::structure::matrix;
    using qrcode::structure::dimension;

    template<class T>
    concept Symbol_Designator = requires(T t)
    {
        version(t); 
        error_level(t);
        str(t);
    };

    template<class Module, Symbol_Designator Designator>
    class symbol
    {
    public:
        constexpr symbol(
            Designator symbol_designator, int mask_pattern, matrix<Module> matrix
        ) noexcept
        : designator{std::move(symbol_designator)}
        , mask_id{mask_pattern}
        , modules{std::move(matrix)}
        {
        }

        [[nodiscard]] friend constexpr auto designator(symbol const& symbol) noexcept 
        { 
            return symbol.designator;
        }

        [[nodiscard]] friend constexpr auto mask_pattern(symbol const& symbol) noexcept 
        { 
            return symbol.mask_id; 
        }

        [[nodiscard]] friend constexpr auto modules(symbol const& symbol) noexcept 
        { 
            return symbol.modules; 
        }

        [[nodiscard]] constexpr auto operator==(symbol const&) const noexcept -> bool = default;
        [[nodiscard]] constexpr auto operator!=(symbol const&) const noexcept -> bool = default;

    private:
        Designator designator{};
        int mask_id;
        matrix<Module> modules{{0,0}};
    };

    template<class Module, Symbol_Designator Designator>
    symbol(matrix<Module>, Designator) -> symbol<Module, Designator>;

    template<class Module, Symbol_Designator Designator>
    [[nodiscard]] constexpr auto version(symbol<Module, Designator> const& symbol) noexcept 
    { 
        return version(designator(symbol)); 
    }

    template<class Module, Symbol_Designator Designator>
    [[nodiscard]] constexpr auto error_level(symbol<Module, Designator> const& symbol) noexcept 
    { 
        return error_level(designator(symbol)); 
    }

    template<class Module, Symbol_Designator Designator>
    [[nodiscard]] constexpr auto size(symbol<Module, Designator> const& symbol) noexcept 
    { 
        return size(modules(symbol)); 
    }

    template<class Module, Symbol_Designator Designator>
    [[nodiscard]] constexpr auto width(symbol<Module, Designator> const& symbol) noexcept 
    { 
        return width(size(symbol)); 
    }

    template<class Module, Symbol_Designator Designator>
    [[nodiscard]] constexpr auto height(symbol<Module, Designator> const& symbol) noexcept 
    { 
        return height(size(symbol)); 
    }

    template<class Module, Symbol_Designator Designator>
    [[nodiscard]] constexpr auto horizontal_view(symbol<Module, Designator> const& symbol)
    {
        return views::horizontal({0,0}, size(symbol)) | views::element(modules(symbol));
    }

    template<class Module, Symbol_Designator Designator>
    [[nodiscard]] constexpr auto vertical_view(symbol<Module, Designator> const& symbol)
    {
        return views::vertical({0,0}, size(symbol)) | views::element(modules(symbol));
    }
}

namespace qrcode::views
{
    template<class Module, Symbol_Designator Designator>
    [[nodiscard]] constexpr auto horizontal(symbol<Module, Designator> const& symbol) noexcept
    {
        return horizontal_view(symbol);
    }

    template<class Module, Symbol_Designator Designator>
    [[nodiscard]] constexpr auto vertical(symbol<Module, Designator> const& symbol) noexcept
    {
        return vertical_view(symbol);
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::test
{
    struct designator_stub
    {
        int version; int error_level;

        [[nodiscard]] constexpr auto operator==(designator_stub const&) const noexcept -> bool = default;
        [[nodiscard]] constexpr auto operator!=(designator_stub const&) const noexcept -> bool = default;
    };

    constexpr auto version(designator_stub const& stub) noexcept { return stub.version; }
    constexpr auto error_level(designator_stub const& stub) noexcept { return stub.error_level; }
    constexpr auto str(designator_stub const&) noexcept { return "stub"; }

    constexpr auto symbols_can_be_constructed_with_given_module_matrix()
    {
        constexpr auto any_matrix = matrix<char>{{4,6}};
        constexpr auto any_designator = designator_stub{1, 2};
        constexpr auto any_mask_pattern = 7;
        
        static_assert(modules(symbol{any_designator, any_mask_pattern, any_matrix}) == any_matrix);
    }

    constexpr auto symbols_have_a_designator()
    {
        constexpr auto any_mask_pattern = 7;

        static_assert(designator(symbol{designator_stub{.version=1, .error_level=100}, any_mask_pattern, matrix<char>{{2,3}}}) == designator_stub{.version=1, .error_level=100});
        static_assert(designator(symbol{designator_stub{.version=2, .error_level=100}, any_mask_pattern, matrix<char>{{2,3}}}) == designator_stub{.version=2, .error_level=100});
        static_assert(designator(symbol{designator_stub{.version=2, .error_level=600}, any_mask_pattern, matrix<char>{{2,3}}}) == designator_stub{.version=2, .error_level=600});
    }

    constexpr auto symbols_have_a_version()
    {
        constexpr auto any_mask_pattern = 7;

        static_assert(version(symbol{designator_stub{.version=1, .error_level=100}, any_mask_pattern, matrix<char>{{2,3}}}) == 1);
        static_assert(version(symbol{designator_stub{.version=2, .error_level=100}, any_mask_pattern, matrix<char>{{2,3}}}) == 2);
        static_assert(version(symbol{designator_stub{.version=3, .error_level=100}, any_mask_pattern, matrix<char>{{2,3}}}) == 3);
    }

    constexpr auto symbols_have_an_error_level()
    {
        constexpr auto any_mask_pattern = 7;

        static_assert(error_level(symbol{designator_stub{.version=100, .error_level=1}, any_mask_pattern, matrix<char>{{2,3}}}) == 1);
        static_assert(error_level(symbol{designator_stub{.version=100, .error_level=2}, any_mask_pattern, matrix<char>{{2,3}}}) == 2);
        static_assert(error_level(symbol{designator_stub{.version=100, .error_level=3}, any_mask_pattern, matrix<char>{{2,3}}}) == 3);
    }

    constexpr auto symbols_have_a_size()
    {
        constexpr auto any_designator = designator_stub{1, 2};
        constexpr auto any_mask_pattern = 7;

        static_assert(size(symbol{any_designator, any_mask_pattern, matrix<int>{{2,3}}}) == dimension{2,3});
        static_assert(size(symbol{any_designator, any_mask_pattern, matrix<int>{{2,5}}}) == dimension{2,5});
        static_assert(size(symbol{any_designator, any_mask_pattern, matrix<int>{{7,5}}}) == dimension{7,5});
    }

    constexpr auto symbols_have_a_width()
    {
        constexpr auto any_designator = designator_stub{1, 2};
        constexpr auto any_mask_pattern = 7;

        static_assert(width(symbol{any_designator, any_mask_pattern, matrix<char>{{2,3}}}) == 2);
        static_assert(width(symbol{any_designator, any_mask_pattern, matrix<char>{{3,5}}}) == 3);
        static_assert(width(symbol{any_designator, any_mask_pattern, matrix<char>{{7,5}}}) == 7);
    }

    constexpr auto symbols_have_a_height()
    {
        constexpr auto any_designator = designator_stub{1, 2};
        constexpr auto any_mask_pattern = 7;

        static_assert(height(symbol{any_designator, any_mask_pattern, matrix<char>{{2,3}}}) == 3);
        static_assert(height(symbol{any_designator, any_mask_pattern, matrix<char>{{2,5}}}) == 5);
        static_assert(height(symbol{any_designator, any_mask_pattern, matrix<char>{{2,7}}}) == 7);
    }
}
#endif
