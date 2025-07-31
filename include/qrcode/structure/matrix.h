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
#include <cassert>
#include <cx/vector.h>
#include <qrcode/structure/position.h>
#include <qrcode/structure/dimension.h>

namespace qrcode::structure::detail
{
    [[nodiscard]] constexpr auto relative_address(dimension extend, position index) noexcept
    {
        return width(extend) * index.y + index.x;
    }
}

namespace qrcode::structure
{
    template<class T>
    class matrix
    {
        using container = cx::vector<T>;

    public:
        template<class U>
        requires std::convertible_to<U, T>
        explicit constexpr matrix(dimension size, U fill) noexcept
        : extend{size}
        , elements(width(size) * height(size), T{fill}) 
        {
        }

        explicit constexpr matrix(dimension size) noexcept
        : matrix{size, T{}}
        {
        }

        using value_type = T;
        
        [[nodiscard]] constexpr auto operator==(matrix const&) const noexcept -> bool = default;
        [[nodiscard]] constexpr auto operator!=(matrix const&) const noexcept -> bool = default;

        [[nodiscard]] friend constexpr auto size(matrix const& matrix) noexcept { return matrix.extend; }

        [[nodiscard]] friend constexpr auto element_at(matrix& matrix, position index) noexcept -> value_type& 
        { 
            using std::ranges::begin;
            assert(index.x >= 0 && index.x < width(matrix));
            assert(index.y >= 0 && index.y < height(matrix));
            return *(begin(matrix.elements) + detail::relative_address(size(matrix), index));
        }

        [[nodiscard]] friend constexpr auto element_at(matrix const& matrix, position index) noexcept -> value_type const&
        { 
            using std::ranges::begin;
            assert(index.x >= 0 && index.x < width(matrix));
            assert(index.y >= 0 && index.y < height(matrix));
            return *(begin(matrix.elements) + detail::relative_address(size(matrix), index));
        }

        [[nodiscard]] friend constexpr auto contains(matrix const& matrix, position index) noexcept
        { 
            return index.x >= 0 && index.x < width(matrix)
                && index.y >= 0 && index.y < height(matrix);
        }

    private:
        dimension extend;
        container elements;
    };

    template<class T> 
    [[nodiscard]] constexpr auto width(matrix<T> const& matrix) noexcept 
    { 
        return width(size(matrix)); 
    }

    template<class T> 
    [[nodiscard]] constexpr auto height(matrix<T> const& matrix) noexcept 
    { 
        return height(size(matrix)); 
    }
}

#ifdef QRCODE_TESTS_ENABLED
namespace qrcode::structure::detail::test
{
    constexpr auto relative_address_returns_the_index_in_a_linearized_two_dimensional_array_of_given_position()
    {
        constexpr auto any_matrix_size = dimension{3,9};
        constexpr auto other_matrix_size = dimension{5,3};

        static_assert(relative_address(any_matrix_size, {0,0}) == 0);
        static_assert(relative_address(any_matrix_size, {1,0}) == 1);
        static_assert(relative_address(any_matrix_size, {2,0}) == 2);
        static_assert(relative_address(any_matrix_size, {0,1}) == 3);
        static_assert(relative_address(any_matrix_size, {0,2}) == 6);
        static_assert(relative_address(any_matrix_size, {1,2}) == 7);
        static_assert(relative_address(any_matrix_size, {2,5}) == 17);
        static_assert(relative_address(other_matrix_size, {2,0}) == 2);
        static_assert(relative_address(other_matrix_size, {1,2}) == 11);
        static_assert(relative_address(other_matrix_size, {4,1}) == 9);
    }
}

namespace qrcode::structure::test
{
    constexpr auto matrix_elements_are_all_default_initialized()
    {
        constexpr auto any_size = dimension{3, 2};
        
        constexpr auto any_matrix = matrix<int>{any_size};

        static_assert(element_at(any_matrix, {0,0}) == 0);
        static_assert(element_at(any_matrix, {1,0}) == 0);
        static_assert(element_at(any_matrix, {2,0}) == 0);
        static_assert(element_at(any_matrix, {0,1}) == 0);
        static_assert(element_at(any_matrix, {1,1}) == 0);
        static_assert(element_at(any_matrix, {2,1}) == 0);
    }

    constexpr auto matrix_elements_can_be_initialized_by_fill_value()
    {
        constexpr auto any_size = dimension{3, 2};
        
        constexpr auto any_matrix = matrix<int>{any_size, -1};

        static_assert(element_at(any_matrix, {0,0}) == -1);
        static_assert(element_at(any_matrix, {1,0}) == -1);
        static_assert(element_at(any_matrix, {2,0}) == -1);
        static_assert(element_at(any_matrix, {0,1}) == -1);
        static_assert(element_at(any_matrix, {1,1}) == -1);
        static_assert(element_at(any_matrix, {2,1}) == -1);
    }

    constexpr auto matrices_define_their_element_type_as_value_type()
    {
        static_assert(std::is_same_v<matrix<int>::value_type, int>);
        static_assert(std::is_same_v<matrix<float>::value_type, float>);
    }

    constexpr auto matrices_have_a_size()
    {
        static_assert(size(matrix<int>{dimension{1,2}}) == dimension{1,2});
        static_assert(size(matrix<int>{dimension{5,6}}) == dimension{5,6});
    }

    constexpr auto matrices_have_a_width()
    {
        static_assert(width(matrix<int>{dimension{1,2}}) == 1);
        static_assert(width(matrix<int>{dimension{5,6}}) == 5);
    }

    constexpr auto matrices_have_a_height()
    {
        static_assert(height(matrix<int>{dimension{1,2}}) == 2);
        static_assert(height(matrix<int>{dimension{5,6}}) == 6);
    }

    constexpr auto matrices_support_random_access_to_their_elements_when_given_matrix_is_non_const()
    {
        auto f = []
        {
            auto any_matrix = matrix<int>{dimension{5,9}};
            constexpr auto any_position = position{4,6};
            constexpr auto other_position = position{1,2};
            constexpr auto any_value = 12;
            constexpr auto other_value = 4;

            element_at(any_matrix, any_position) = any_value;
            element_at(any_matrix, other_position) = other_value;

            return element_at(any_matrix, any_position) == any_value
                && element_at(any_matrix, other_position) == other_value;
        };
        static_assert(f());
    }

    constexpr auto matrices_support_readonly_access_to_their_elements_when_given_matrix_is_const()
    {
        auto f = []
        {
            auto any_matrix = matrix<int>{dimension{5,9}};
            constexpr auto any_position = position{4,6};
            constexpr auto other_position = position{1,2};
            constexpr auto any_value = 12;
            constexpr auto other_value = 4;

            element_at(any_matrix, any_position) = any_value;
            element_at(any_matrix, other_position) = other_value;
            auto const& const_matrix = any_matrix;

            return element_at(const_matrix, any_position) == any_value
                && element_at(const_matrix, other_position) == other_value;
        };
        static_assert(f());
    }

    constexpr auto matrix_can_be_asked_whether_a_point_lies_inside()
    {
        constexpr auto any_size = dimension{3, 2};
        
        constexpr auto any_matrix = matrix<int>{any_size};

        static_assert(contains(any_matrix, {0,0}));
        static_assert(!contains(any_matrix, {-1,0}));
        static_assert(contains(any_matrix, {1,0}));
        static_assert(!contains(any_matrix, {0,-1}));
        static_assert(contains(any_matrix, {2,1}));
        static_assert(!contains(any_matrix, {5,1}));
        static_assert(contains(any_matrix, {0,1}));
        static_assert(!contains(any_matrix, {0,10}));
    }    
}
#endif
