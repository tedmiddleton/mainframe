//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_detail_useries_h
#define INCLUDED_mainframe_detail_useries_h

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <list>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "mainframe/detail/base.hpp"
#include "mainframe/missing.hpp"
#include "mainframe/detail/series_vector.hpp"

namespace mf
{

template<typename T>
class series;

// This is an untyped version of series
class useries
{
    using iseries_vector = detail::iseries_vector;
    template<typename T>
    using series_vector = detail::series_vector<T>;

public:
    useries(const useries&)            = default;
    useries& operator=(const useries&) = default;

    template<typename T>
    useries(const series<T>& s)
        : m_name(s.m_name)
        , m_data(std::dynamic_pointer_cast<iseries_vector>(s.m_sharedvec))
    {}

    template<typename T>
    operator series<T>() const
    {
        series<T> s;
        s.m_name      = m_name;
        s.m_sharedvec = std::dynamic_pointer_cast<series_vector<T>>(m_data);
        return s;
    }

    void
    clear()
    {
        m_data->clear();
    }

    const std::string&
    name() const
    {
        return m_name;
    }

    void
    set_name(const std::string& name)
    {
        m_name = name;
    }

    size_t
    size() const
    {
        return m_data->size();
    };


private:
    std::string m_name;
    std::shared_ptr<iseries_vector> m_data;
};

} // namespace mf


#endif // INCLUDED_mainframe_detail_useries_h

