
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_base_h
#define INCLUDED_mainframe_base_h

#include <ostream>
#include <vector>
#include <variant>

namespace mf
{

const char * get_horzrule( size_t num );
const char * get_emptyspace( size_t num );
size_t get_max_string_length( const std::vector<std::string>& );
std::vector<size_t> get_max_string_lengths( const std::vector<std::vector<std::string>>& );

template< typename T > class mi;

template<typename T>
auto stringify( std::ostream & o, const T & t, bool ) -> decltype( o << t, o )
{
    o << t;
    return o;
}

std::ostream& stringify( std::ostream & o, const char & t, bool );

std::ostream& stringify( std::ostream & o, const unsigned char & t, bool );

template<typename T>
std::ostream& stringify( std::ostream & o, const mi<T> & t, bool )
{
    if ( !t.has_value() ) {
        o << "missing";
        return o;
    }
    else {
        return stringify( o, *t, true );
    }
}

template<typename T>
std::ostream& stringify( std::ostream & o, const T &, int )
{
    o << "opaque";
    return o;
}

template< typename T, typename Vt >
struct prepend_variant;

template< typename T, typename ... Ts >
struct prepend_variant<T, std::variant<Ts...>>
{
    using type = std::variant<T, Ts...>;
};

template< typename T, typename Vt >
struct variant_contains;

template< typename T, typename V, typename ... Vs >
struct variant_contains<T, std::variant<V, Vs...>>
{
    static const bool value = 
        std::is_same<T, V>::value ||
        variant_contains<T, std::variant<Vs...>>::value;
};

template< typename T, typename V >
struct variant_contains<T, std::variant<V>>
{
    static const bool value = std::is_same<T, V>::value;
};


template< typename ... Ts >
struct variant_unique;

template< typename T, typename ... Ts >
struct variant_unique<T, Ts...>
{
    using uniquecdr = typename variant_unique<Ts...>::type;
    using type = typename std::conditional<
        variant_contains<T, uniquecdr>::value,
        uniquecdr,
        typename prepend_variant<T, uniquecdr>::type
        >::type;
};

template< typename T >
struct variant_unique<T>
{
    using type = std::variant<T>;
};

template<size_t Ind, size_t Curr, typename ... Ts>
struct pack_element_impl;

template<size_t Matched, typename T, typename ... Ts>
struct pack_element_impl<Matched, Matched, T, Ts...>
{
    using type = T;
};

template<size_t Ind, size_t Curr, typename T, typename ... Ts>
struct pack_element_impl<Ind, Curr, T, Ts...>
{
    using type = typename pack_element_impl<Ind, Curr+1, Ts...>::type;
};

template<size_t Ind, typename ... Ts>
struct pack_element : pack_element_impl<Ind, 0, Ts...> {};

template < size_t Ind, size_t ... List >
struct contains : std::true_type {};

template < size_t Ind, size_t IndHead, size_t... IndRest >
struct contains<Ind, IndHead, IndRest...> : 
    std::conditional< 
        Ind == IndHead,
        std::true_type,
        contains<Ind, IndRest ... >
    >::type {};

template < size_t Ind >
struct contains<Ind> : std::false_type {};

template< typename T >
struct is_missing : std::false_type {};

template< typename T >
struct is_missing<mi<T>> : std::true_type {};

template< typename T >
struct ensure_missing
{
    using type = mi<T>;
};

template< typename T >
struct ensure_missing< mi<T> >
{
    using type = mi<T>;
};

template< typename T >
struct unwrap_missing
{
    using type = T;
    static T unwrap( const T& t ) 
    {
        return t;
    }
};

template< typename T >
struct unwrap_missing< mi<T> >
{
    using type = T;
    static T unwrap( const mi<T>& t ) 
    {
        if ( t.has_value() ) {
            return *t;
        }
        else {
            // default construction - our last resort
            return T{};
        }
    }
};

} // namespace mf


#endif // INCLUDED_mainframe_base_h

