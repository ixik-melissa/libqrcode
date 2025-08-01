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

#include <vector>
#include <array>
#include <exception>
#include <algorithm>
#include <ranges>

namespace cx::detail
{
    // poor man's constexpr stack-based vector
    template<class T>
    class vector
    {
        static constexpr auto N = 40000;
        using container = std::array<T,N>;

    public:
        [[nodiscard]] static constexpr auto max_size() noexcept { return N; }

        constexpr vector() noexcept
        : current_size{0}, elements{}
        {
        }

        constexpr vector(std::size_t count, T value = T{}) noexcept
        : current_size{count}
        , elements{}
        {
            for (auto i = begin(); i != begin()+count; ++i)
                *i = value;
        }
        
        constexpr vector(std::initializer_list<T> values) noexcept
        : current_size{std::ranges::size(values)}
        , elements{}
        {
            using std::ranges::begin;
            using std::ranges::end;
            using std::random_access_iterator_tag;

            initialize(begin(values), end(values), random_access_iterator_tag{});
        }

        template<class Iterator, class = typename std::iterator_traits<Iterator>::iterator_category>
        constexpr vector(Iterator first, Iterator last) noexcept
        : current_size{}
        , elements{}
        {
            using std::iterator_traits;

            initialize(first, last, typename iterator_traits<Iterator>::iterator_category{});
        }

        using iterator = typename container::iterator;
        using const_iterator = typename container::const_iterator;
        using reverse_iterator = typename container::reverse_iterator;
        using const_reverse_iterator = typename container::const_reverse_iterator;
        using value_type = T;
        
        [[nodiscard]] constexpr auto size() const noexcept { return current_size; }
        [[nodiscard]] constexpr auto empty() const noexcept { return current_size == 0; }

        [[nodiscard]] constexpr auto begin() noexcept { using std::ranges::begin; return begin(elements); }
        [[nodiscard]] constexpr auto end() noexcept { return begin() + current_size; }
        
        [[nodiscard]] constexpr auto begin() const noexcept { using std::ranges::begin; return begin(elements); }
        [[nodiscard]] constexpr auto end() const noexcept { return begin() + current_size; }
            
        [[nodiscard]] constexpr auto rbegin() noexcept { using std::ranges::rbegin; return rbegin(elements)+(N-current_size); }
        [[nodiscard]] constexpr auto rend() noexcept { return rbegin() + current_size; }
        
        [[nodiscard]] constexpr auto rbegin() const noexcept { using std::ranges::rbegin; return rbegin(elements)+(N-current_size); }
        [[nodiscard]] constexpr auto rend() const noexcept { return rbegin() + current_size; }

        constexpr auto& front() const noexcept { return elements[0]; }
        constexpr auto& front() noexcept { return elements[0]; }

        constexpr auto& back() const noexcept { return elements[current_size-1]; }
        constexpr auto& back() noexcept { return elements[current_size-1]; }

        [[nodiscard]] constexpr auto push_back(T v) 
        {
            if (current_size == N)
                std::terminate();

            elements[current_size++] = std::move(v); 
        }

        template<class...Args>
        [[nodiscard]] constexpr auto emplace_back(Args&&... args) 
        {
            if (current_size == N)
                std::terminate();

            elements[current_size++] = T{std::forward<Args>(args)...}; 
        }

        constexpr auto& operator[](int index) const noexcept { return elements[index]; }
        constexpr auto& operator[](int index) noexcept { return elements[index]; }

    private:
        template<class Iterator>
        [[nodiscard]] constexpr auto initialize(Iterator first, Iterator last, std::forward_iterator_tag)
        {
            current_size = std::distance(first, last);
            if (current_size > N)
                std::terminate();

            auto dst = begin();
            for (auto i = first; i != last; ++i, ++dst)
                *dst = *i;
        }

        template<class Iterator>
        [[nodiscard]] constexpr auto initialize(Iterator first, Iterator last, std::input_iterator_tag)
        {
            for (auto i = first; i != last; ++i)
                push_back(*i);
        }

        std::size_t current_size;
        container elements;
    };

    template<class T>
    constexpr auto operator==(vector<T> const& a, vector<T> const& b) noexcept 
    { 
        return std::ranges::equal(a, b);
    }
}

namespace cx
{
    // msvc or the C++ standard library respectively seems to have problems in constexpr sitations, 
    // use old implementation in constexpr instead
#ifdef _MSC_VER 
    template<class T>
    using vector = std::conditional_t<std::is_constant_evaluated(), cx::detail::vector<T>, std::vector<T>>;

    using std::ranges::begin;
    using std::ranges::end;
    using std::ranges::rbegin;
    using std::ranges::rend;
    using std::ranges::size;
    using std::ranges::empty;
#else
    template<class T>
    using vector = std::vector<T>;
#endif
}

#ifdef QRCODE_TESTS_ENABLED
namespace cx::test
{
    constexpr auto cx_vectors_are_empty_by_default()
    {
        constexpr auto v = cx::vector<int>{};
        static_assert(begin(v) == end(v));
    }

    constexpr auto cx_vectors_allow_adding_new_elements()
    {
        auto f = []
        {
            auto v = cx::vector<int>{};

            v.push_back(2);
            v.push_back(5);
            v.push_back(1);

            return *(begin(v)+0) == 2
                && *(begin(v)+1) == 5
                && *(begin(v)+2) == 1
                ;
        };
        static_assert(f());
    }

    constexpr auto cx_vectors_allow_emplacing_new_elements()
    {
        auto f = []
        {
            auto v = cx::vector<std::pair<int, float>>{};

            v.emplace_back(2,1.f);
            v.emplace_back(5,7.f);
            v.emplace_back(6,3.f);

            return *(begin(v)+0) == std::pair<int, float>{2,1.f}
                && *(begin(v)+1) == std::pair<int, float>{5,7.f}
                && *(begin(v)+2) == std::pair<int, float>{6,3.f}
                ;
        };
        static_assert(f());
    }

    constexpr auto cx_vectors_can_be_initialized_with_preexisting_elements()
    {
        static_assert([]()
        {
            auto const v = cx::vector<int>{1,2,3,4,5};
            return std::ranges::equal(v, std::array{1,2,3,4,5});
        }());
    }

    constexpr auto cx_vectors_have_a_size()
    {
        static_assert(size(cx::vector<int>{}) == 0);
        static_assert(size(cx::vector<int>{3}) == 1);
        static_assert(size(cx::vector<int>{3,1}) == 2);
        static_assert(size(cx::vector<int>{3,1,4}) == 3);
        static_assert(size(cx::vector<int>{3,1,4,7}) == 4);
    }

    constexpr auto cx_vectors_can_be_asked_whether_they_are_empty()
    {
        static_assert(empty(cx::vector<int>{}));
        static_assert(!empty(cx::vector<int>{3}));
    }

    constexpr auto cx_vectors_can_be_initialized_with_preexisting_elements_less_than_cx_vectors_capacity()
    {
        static_assert([]()
        {
            auto const v = cx::vector<int>{1,2,3};
            return std::ranges::equal(v, std::array{1,2,3})
                && size(v) == 3;
        }());
    }

    constexpr auto cx_vectors_can_be_initialized_with_a_specified_number_of_elements_equal_or_less_than_cx_vectors_capacity()
    {
        static_assert([]()
        {
            auto const v = cx::vector<int>(4);
            return std::ranges::equal(v, std::array{int{}, int{}, int{}, int{}}) 
                && size(v) == 4;
        }());
    }

    constexpr auto cx_vectors_can_be_initialized_with_a_specified_number_of_elements_and_a_fill_element_equal_or_less_than_cx_vectors_capacity()
    {
        static_assert([]()
        {
            auto const v = cx::vector<int>(4,-2);
            return std::ranges::equal(v, std::array{-2,-2,-2,-2})
                && size(v) == 4;
        }());
    }

    constexpr auto cx_vectors_can_be_initialized_from_iterator_range()
    {
        static_assert([]()
        {
            auto const some_elements = std::array{1,2,3,4,5};
            auto const v = cx::vector<int>{begin(some_elements), end(some_elements)};
            return std::ranges::equal(v, std::array{1,2,3,4,5});
        }());
        
    }

    constexpr auto cx_vectors_allow_accessing_their_elements_by_reverse_iterators()
    {
        static_assert([]
        {
            auto const v = cx::vector<int>{1,2,3,4,5};
            auto const& cv = v;

            auto r = rbegin(v);
            auto cr = rbegin(cv);

            for (auto i = end(v)-1; i != begin(v); --i, ++r, ++cr)
            {
                if (&*r != &*i) return false;
                if (&*cr != &*i) return false;
            }
            
            if (&*(rend(v)-1) != &*begin(v)) return false;
            if (&*(rend(cv)-1) != &*begin(v)) return false;
            return true;
        }());
    }

    constexpr auto cx_vectors_allow_explicit_read_access_to_their_first_element_if_vector_has_any_data()
    {
        static_assert(cx::vector<int>{3,1,4,1,5}.front() == 3);
        static_assert(cx::vector<int>{2,7,0,3,8}.front() ==2);
    }

    constexpr auto cx_vectors_allow_explicit_write_access_to_their_first_element_if_vector_has_any_data()
    {
        static_assert([]
        {
            auto v = cx::vector<int>{3,1,4,1,5};
            auto const& cv = v;
            auto& element = v.front();
            auto const& const_element = cv.front();

            v.front() = 7;

            return element == 7 && const_element == 7;
        }());
    }

    constexpr auto cx_vectors_allow_explicit_read_access_to_theirlast_element_if_vector_has_any_data()
    {
        static_assert(cx::vector<int>{3,1,4,1,5}.back() == 5);
        static_assert(cx::vector<int>{2,7,0,3,8}.back() == 8);
    }

    constexpr auto cx_vectors_allow_explicit_write_access_to_their_last_element_if_vector_has_any_data()
    {
        static_assert([]
        {
            auto v = cx::vector<int>{3,1,4,1,5};
            auto const& cv = v;
            auto& element = v.back();
            auto const& const_element = cv.back();

            v.back() = 7;

            return element == 7 && const_element == 7;
        }());
    }

    constexpr auto cx_vectors_support_read_only_index_access()
    {
        static_assert([]()
        {
            auto v = cx::vector<int>{1,2,3,4,5};

            auto const by_value = v[3];
            auto const& by_reference = v[3];
            *(begin(v)+3) = 7;

            return by_value == 4 && by_reference == 7;
        }());
    }

    constexpr auto cx_vectors_support_write_index_access()
    {
        static_assert([]()
        {
            auto v = cx::vector<int>{1,2,3,4,5};

            auto const by_value = v[3];
            auto& by_reference = v[3];
            by_reference = 7;

            return by_value == 4 && by_reference == 7;
        }());
    }
}
#endif
