
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_frame_iterator_h
#define INCLUDED_mainframe_frame_iterator_h

#include <iterator>
#include "mainframe/series.hpp"

#define TRAP() asm ( "int $3\n" )

namespace mf
{

template< size_t Ind >
struct expr_column;

template< typename T >
struct terminal;

template< size_t Ind >
struct columnindex { static constexpr size_t index = Ind; };

inline constexpr columnindex<0> _c0;
inline constexpr columnindex<1> _c1;
inline constexpr columnindex<2> _c2;
inline constexpr columnindex<3> _c3;
inline constexpr columnindex<4> _c4;
inline constexpr columnindex<5> _c5;

template< typename ... Ts >
class _row_proxy;

template< typename ... Ts >
class frame_row
{
public:
    frame_row( std::tuple<Ts...> args ) 
        : data( args ) 
    {
    }

    frame_row( const frame_row& other )
    {
        init<0>( other );
    }

    frame_row( const _row_proxy< Ts... >& refs )
    {
        init<0>( refs );
    }

    frame_row& operator=( const frame_row& other )
    {
        init<0>( other );
        return *this;
    }

    frame_row& operator=( const _row_proxy< Ts... >& refs )
    {
        init<0>( refs );
        return *this;
    }

    template< size_t Ind >
    typename detail::pack_element<Ind, Ts...>::type&
    at( const columnindex<Ind>& )
    {
        return std::get<Ind>( data );
    }

    template< size_t Ind >
    const typename detail::pack_element<Ind, Ts...>::type&
    at( const columnindex<Ind>& ) const
    {
        return std::get<Ind>( data );
    }

    template< size_t Ind=0 >
    void debug( std::ostream& o ) const
    {
        if constexpr ( Ind == 0 ) {
            o << "frame_row[ ";
        }
        columnindex<Ind> ci;
        detail::stringify( o, at( ci ), true );
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            o << ", ";
            debug<Ind+1>( o );
        }
        else {
            o << " ]";
        }
    }

private:

    template< size_t Ind, template < typename... > typename Row >
    void init( const Row< Ts... >& refs )
    {
        columnindex<Ind> ci;
        at( ci ) = refs.at( ci );
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            init<Ind+1>( refs );
        }
    }

    std::tuple<Ts...> data;
};

template< typename ... Ts >
class _row_proxy
{
    template< typename ... Us >
    friend class base_frit;

    // Only base_frit should be able to create one of these
    _row_proxy( const _row_proxy& other )
        : ptrs( other.ptrs )
    {
    }
    
    void swap_ptrs( _row_proxy& other ) noexcept
    {
        swap( ptrs, other.ptrs );
    }

    ptrdiff_t operator-( const _row_proxy& other ) const
    {
        using T = typename detail::pack_element<0, Ts...>::type;
        const T* ptr = std::get<0>( ptrs );
        const T* otherptr = std::get<0>( other.ptrs );
        return ptr - otherptr;
    }

    template< size_t Ind=0 >
    void operator+=( ptrdiff_t off )
    {
        using T = typename detail::pack_element<Ind, Ts...>::type;
        T* ptr = std::get<Ind>( ptrs );
        std::get<Ind>( ptrs ) = ptr + off;
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            operator+= <Ind+1>( off );
        }
    }

    _row_proxy operator+( ptrdiff_t off ) const
    {
        _row_proxy out( ptrs );
        out += off;
        return out;
    }
public:
    _row_proxy( std::tuple<Ts*...> p ) 
        : ptrs( p ) 
    {
    }

    _row_proxy& operator=( const _row_proxy& row )
    {
        init<0>( row );
        return *this;
    }

    _row_proxy& operator=( const frame_row<Ts...>& vals )
    {
        init<0>( vals );
        return *this;
    }

    // by-value swap
    template< size_t Ind = 0 >
    void swap_values( _row_proxy& other ) noexcept
    {
        using std::swap;
        columnindex<Ind> ci;
        swap( at( ci ), other.at( ci ) );
        if constexpr ( Ind + 1 < sizeof...(Ts) ) {
            swap_values< Ind + 1 >( other );
        }
    }

    template< size_t Ind >
    typename detail::pack_element<Ind, Ts...>::type&
    at( const columnindex<Ind>& )
    {
        return *(std::get<Ind>( ptrs ));
    }

    template< size_t Ind >
    const typename detail::pack_element<Ind, Ts...>::type&
    at( const columnindex<Ind>& ) const
    {
        return *(std::get<Ind>( ptrs ));
    }


    template< size_t Ind=0 >
    void debug( std::ostream& o ) const
    {
        if constexpr ( Ind == 0 ) {
            o << "_row_proxy[ ";
        }
        columnindex<Ind> ci;
        detail::stringify( o, at( ci ), true );
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            o << ", ";
            debug<Ind+1>( o );
        }
        else {
            o << " ]";
        }
    }

private:
    template< size_t Ind, template <typename...> typename Row >
    void init( const Row<Ts...>& vals )
    {
        if constexpr ( Ind == 0 ) {
            std::cout << "_row_proxy::init( const Row<Ts...>& vals )\n";
            debug( std::cout );
            std::cout << " := ";
            vals.debug( std::cout );
            std::cout << "\n";
        }
        columnindex<Ind> ci;
        at( ci ) = vals.at( ci );
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            init<Ind+1>( vals );
        }
    }

    std::tuple<Ts*...> ptrs;
};

template< typename ... Ts >
void swap( mf::_row_proxy< Ts... >& left, mf::_row_proxy< Ts... >& right ) noexcept
{
    left.swap_values( right );
}

template< size_t Ind=0, typename ... Ts >
bool operator==( const _row_proxy< Ts... >& left, const _row_proxy< Ts... >& right )
{
    columnindex<Ind> ci;
    const auto& leftval = left.at( ci );
    const auto& rightval = right.at( ci );
    if ( leftval == rightval ) {
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            return operator==<Ind+1>( left, right );
        }
        else {
            return true;
        }
    }
    else {
        return false;
    }
}

template< typename ... Ts >
bool operator!=( const _row_proxy< Ts... >& left, const _row_proxy< Ts... >& right )
{
    return !(left == right);
}

// Wrong, kind of, but we'll go with it for now
template< size_t Ind=0, typename ... Ts >
bool operator<( const _row_proxy< Ts... >& left, const _row_proxy< Ts... >& right )
{
    columnindex<Ind> ci;
    const auto& leftval = left.at( ci );
    const auto& rightval = right.at( ci );
    if ( leftval < rightval ) {
        return true;
    }
    else if ( leftval == rightval ) {
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            return operator< <Ind+1>( left, right );
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}

// This is wrong - but go with it for now anyways
template< size_t Ind=0, typename ... Ts >
bool operator<( const frame_row<Ts...>& left, const _row_proxy<Ts...>& right )
{
    columnindex<Ind> ci;
    const auto& leftval = left.at( ci );
    const auto& rightval = right.at( ci );
    if ( leftval < rightval ) {
        return true;
    }
    else if ( leftval == rightval ) {
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            return operator< <Ind+1>( left, right );
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}

// This is wrong - but go with it for now anyways
template< size_t Ind=0, typename ... Ts >
bool operator<( const _row_proxy<Ts...>& left, const frame_row<Ts...>& right )
{
    columnindex<Ind> ci;
    const auto& leftval = left.at( ci );
    const auto& rightval = right.at( ci );
    if ( leftval < rightval ) {
        return true;
    }
    else if ( leftval == rightval ) {
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            return operator< <Ind+1>( left, right );
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}

template< typename ... Ts >
class base_frit
{
public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = ptrdiff_t;
    using value_type = frame_row<Ts...>;
    //using value_type = _row_proxy<Ts...>;
    using reference = _row_proxy<Ts...>&;
    using pointer = _row_proxy<Ts...>*;

    base_frit( std::tuple<Ts*...> t )
        : row( t )
    {
        std::cout << "base_frit::base_frit( std::tuple<Ts*...> t )\n";
    }

    base_frit( _row_proxy<Ts...> r )
        : row( r )
    {
        std::cout << "base_frit::base_frit( std::tuple<Ts*...> t )\n";
    }

    base_frit( const base_frit& ) = default;
    base_frit& operator=( const base_frit& ) = default;

    reference operator*() const
    {
        return const_cast<reference>( row );
    }

    pointer operator->() const
    {
        return const_cast<pointer>( &row );
    }

    void swap( base_frit& other ) noexcept
    {
        row.swap_ptrs( other );
    }

    base_frit& operator++() 
    {
        row += 1;
        return *this;
    }

    base_frit& operator++(int) 
    {
        row += 1;
        return *this;
    }

    base_frit& operator--() 
    {
        row += -1;
        return *this;
    }

    base_frit& operator--(int) 
    {
        row += -1;
        return *this;
    }

    bool operator==( const base_frit& other ) const
    {
        return row == other.row;
    }

    bool operator!=( const base_frit& other ) const
    {
        return row != other.row;
    }

    bool operator<( const base_frit& other ) const
    {
        std::cout << "base_frit::operator<( const base_frit& ) const\n";
        return row < other.row;
    }

    difference_type operator-( const base_frit& other ) const
    {
        return this->row - other.row;
    }

    base_frit operator+( ptrdiff_t off ) const
    {
        return base_frit( row + off );
    }

    base_frit operator-( ptrdiff_t off ) const
    {
        return base_frit( row + (-off) );
    }

private:

    template< typename ... Us >
    friend void swap( base_frit< Us... >& left, base_frit< Us... >& right ) noexcept;

    _row_proxy<Ts...> row;
};

template< typename ... Ts >
using frit = base_frit<Ts...>;

template< typename ... Ts >
struct pointerize;

template< typename ... Ts >
struct pointerize<std::tuple< std::vector<Ts>...>>
{
    using argument_type = std::tuple< std::vector<Ts>... >;
    using type = std::tuple< Ts*... >;
    using const_type = std::tuple< Ts*... >;

    static type op( argument_type& arg, ptrdiff_t offset = 0 )
    {
        type out;
        assign_impl<0>( arg, offset, out );
        return out;
    }

    static type const_op( const argument_type& arg, ptrdiff_t offset = 0 )
    {
        const_type out;
        assign_const_impl<0>( arg, offset, out );
        return out;
    }

    template< size_t Ind >
    static void assign_impl( argument_type& arg, 
                             ptrdiff_t offset, 
                             type& out )
    {
        using T = typename detail::pack_element<Ind, Ts...>::type;
        std::vector<T>& argelem = std::get<Ind>( arg );
        std::get<Ind>( out ) = argelem.data() + offset;
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            assign_impl< Ind+1 >( arg, offset, out );
        }
    }

    template< size_t Ind >
    static void assign_const_impl( const argument_type& arg, 
                                   ptrdiff_t offset, 
                                   const_type& out )
    {
        using T = typename detail::pack_element<Ind, Ts...>::type;
        const std::vector<T>& argelem = std::get<Ind>( arg );
        std::get<Ind>( out ) = argelem.data() + offset;
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            assign_const_impl< Ind+1 >( arg, offset, out );
        }
    }
};

template< typename ... Ts >
class fr
{
public:
    using iterator = frit<Ts...>;
    fr() = default;

    frit<Ts...> begin()
    {
        auto ptrtuple = pointerize<decltype(columns)>::op( columns );
        frit<Ts...> out{ ptrtuple };
        return out;
    }

    frit<Ts...> end()
    {
        ptrdiff_t offset = std::get<0>(columns).size();
        frit<Ts...> out{ pointerize<decltype(columns)>::op( columns, offset ) };
        return out;
    }

    template< size_t Ind=0 >
    void push_back( frame_row<Ts...>& row )
    {
        using T = typename detail::pack_element<Ind, Ts...>::type;
        std::vector<T>& vec = std::get<Ind>( columns );
        columnindex<Ind> colind;
        vec.push_back( row.at( colind ) );
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            push_back<Ind+1>( row );
        }
    }

    void debug() const
    {
        std::cout << "fr object with " << sizeof...(Ts) << 
            " columns @" << (void*)this << ":\n";
        debug_impl<0>();
    }

    template< size_t Ind=0 >
    void debug_impl() const
    {
        using T = typename detail::pack_element<Ind, Ts...>::type;
        const std::vector<T>& vec = std::get<Ind>(columns);
        const T* start = vec.data();
        const T* end = start + vec.size();
        std::cout << " col " << Ind << " startptr=@" << (void*)start <<
            " endptr=@" << (void*)end << "\n";
        if constexpr ( Ind+1 < sizeof...(Ts) ) {
            debug_impl<Ind+1>();
        }
    }

    std::vector< std::vector< std::string > > 
    to_string() const
    {
        std::vector< std::vector< std::string > > out;
        to_string<0>( out );
        return out;
    }

    template< size_t Ind=0 >
    void 
    to_string( std::vector< std::vector< std::string > >& strs ) const
    {
        using U = typename detail::pack_element<Ind, Ts...>::type;
        const std::vector<U>& s = std::get< Ind >( columns );
        std::vector< std::string > colstrs;
        for ( const auto& elem : s ) {
            std::stringstream ss;
            detail::stringify( ss, elem, true );
            colstrs.push_back( ss.str() );
        }
        strs.push_back( colstrs );
        if constexpr ( Ind+1 < sizeof...( Ts )  ) {
            to_string< Ind+1 >( strs );
        }
    }

    size_t
    size() const
    {
        return std::get< 0 >( columns ).size();
    }

    std::tuple< std::vector< Ts >... > columns;
};

template< typename ... Ts >
std::ostream & operator<<( std::ostream& o, const fr< Ts... >& f )
{
    std::ios_base::fmtflags fl( o.flags() );

    std::vector<std::vector<std::string>> uf = f.to_string();
    std::vector<std::string> names;
    for ( unsigned i = 0; i < uf.size(); ++i ) {
        std::stringstream ss; ss << "col" << i;
        names.push_back( ss.str() );
    }
    auto widths = detail::get_max_string_lengths( uf );
    constexpr size_t num_columns = sizeof...(Ts);
    const size_t num_rows = f.size();

    auto gutter_width = 
        num_rows > 0 ? static_cast<size_t>(std::ceil( std::log10( num_rows ) ) ) + 1 : 1;

    o << std::boolalpha;
    o << detail::get_emptyspace( gutter_width );
    for ( size_t i = 0; i < num_columns; ++i ) {
        auto& name = names[ i ];
        auto& width = widths[ i ];
        width = std::max( width, name.size() );
        o << "| " << std::setw( width ) << name << " ";
    }
    o << "\n";

    o << detail::get_horzrule( gutter_width );
    for ( size_t i = 0; i < num_columns; ++i ) {
        auto& width = widths[ i ];
        o << "|" << detail::get_horzrule( width + 2 );
    }
    o << "\n";

    for ( size_t rowind = 0; rowind < num_rows; ++rowind ) {
        o << std::setw( gutter_width ) << rowind;
        for ( size_t colind = 0; colind < num_columns; ++colind ) {

            std::string& datum = uf[ colind ][ rowind ];
            auto width = widths[ colind ];

            o << "| " << std::setw( width ) << datum << " ";
        }
        o << "\n";
    }

    o.flags( fl );
    return o;
}

#if 0
template< typename ... Ts >
class frame_row
{
public:
    frame_row()
        : m_pushnext( 0 )
    {}

    frame_row( const std::tuple<Ts...>& ptrs )
        : m_ptrs( ptrs ) 
        , m_holdsdata( false )
        , m_pushnext( 0 )
    {}

    frame_row( Ts ... args )
        : m_data( args... )
        , m_pushnext( 0 )
    {}

    frame_row( const frame_row& other )
    {
    }

    template< size_t Ind >
    typename detail::pack_element< Ind, Ts&... >::type
    at()
    {
        if ( m_holdsdata ) {
        }
        else {
            auto colptr = std::get< Ind >( m_ptrs );
            return *colptr;
        }
    }

    template< size_t Ind >
    typename detail::pack_element< Ind, const Ts& ... >::type
    at() const
    {
        auto colptr = std::get< Ind >( m_ptrs );
        return *colptr;
    }

    template< size_t Ind >
    typename detail::pack_element< Ind, Ts& ... >::type
    at( terminal<expr_column<Ind>> )
    {
        auto colptr = std::get< Ind >( m_ptrs );
        return *colptr;
    }

    template< size_t Ind >
    typename detail::pack_element< Ind, const Ts& ... >::type
    at( terminal<expr_column<Ind>> ) const
    {
        auto colptr = std::get< Ind >( m_ptrs );
        return *colptr;
    }

    template< size_t Ind >
    typename detail::pack_element< Ind, Ts& ... >::type
    operator[]( terminal<expr_column<Ind>> )
    {
        auto colptr = std::get< Ind >( m_ptrs );
        return *colptr;
    }

    template< size_t Ind >
    typename detail::pack_element< Ind, const Ts& ... >::type
    operator[]( terminal<expr_column<Ind>> ) const
    {
        auto colptr = std::get< Ind >( m_ptrs );
        return *colptr;
    }

    bool any_missing() const
    {
        return any_missing_impl<0, Ts...>();
    }

    template< size_t Ind, typename U, typename ... Us >
    bool any_missing_impl() const
    {
        const U& elem = at< Ind >();
        if constexpr ( detail::is_missing<U>::value ) {
            if ( !elem.has_value() ) {
                return true;
            }
        }
        if constexpr ( sizeof...(Us) > 0 ) {
            return any_missing_impl<Ind+1, Us...>();
        }
        return false;
    }

    template< size_t Ind=0 >
    bool operator==( const frame_row< Ts... >& other ) const
    {
        auto* t = std::get< Ind >( m_ptrs );
        auto* other_t = std::get< Ind >( other.m_ptrs );
        if ( *t != *other_t ) {
            return false;
        }
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            return operator==<Ind+1>( other );
        }
        return true;
    }

    bool operator!=( const frame_row< Ts... >& other ) const
    {
        return !(operator==( other ));
    }

    template< size_t Ind=0 >
    bool operator<=( const frame_row< Ts... >& other ) const
    {
        auto* t = std::get< Ind >( m_ptrs );
        auto* other_t = std::get< Ind >( other.m_ptrs );
        if ( *t > *other_t ) {
            return false;
        }
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            return operator<=<Ind+1>( other );
        }
        return true;
    }

    template< size_t Ind=0 >
    bool operator>=( const frame_row< Ts... >& other ) const
    {
        auto* t = std::get< Ind >( m_ptrs );
        auto* other_t = std::get< Ind >( other.m_ptrs );
        if ( *t < *other_t ) {
            return false;
        }
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            return operator>=<Ind+1>( other );
        }
        return true;
    }

    template< size_t Ind=0 >
    bool operator<( const frame_row< Ts... >& other ) const
    {
        auto* t = std::get< Ind >( m_ptrs );
        auto* other_t = std::get< Ind >( other.m_ptrs );
        if ( *t >= *other_t ) {
            return false;
        }
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            return operator< <Ind+1>( other );
        }
        return true;
    }

    template< size_t Ind=0 >
    bool operator>( const frame_row< Ts... >& other ) const
    {
        auto* t = std::get< Ind >( m_ptrs );
        auto* other_t = std::get< Ind >( other.m_ptrs );
        if ( *t <= *other_t ) {
            return false;
        }
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            return operator><Ind+1>( other );
        }
        return true;
    }
private:

    std::tuple< std::optional< Ts >... > m_data;
    size_t m_pushnext;
};

template< typename ... Ts >
class _row_proxy
{
    _row_proxy( const std::tuple<Ts*...>& ptrs )
        : m_ptrs( ptrs ) 
    {}

    _row_proxy( Ts* ... args )
        : m_ptrs( args... )
    {}

    std::tuple< Ts*... > m_ptrs;
};

template< bool IsConst, bool IsReverse, typename ... Ts >
class base_frame_iterator
{
public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = ptrdiff_t;

    base_frame_iterator() = default;

    base_frame_iterator( const std::tuple<Ts*...>& ptrs )
        : m_row( ptrs )
    {
    }

    template< size_t Ind >
    Iter<typename std::tuple_element< Ind, std::tuple< Ts ... > >::type> 
    column_iterator()
    {
        return std::get< Ind >( m_row.m_iters );
    }

    void operator++() { operator+=( 1 ); }
    void operator++(int) { operator+=( 1 ); }
    void operator--() { operator-=( 1 ); }
    void operator--(int) { operator-=( 1 ); }

    void operator+=( ptrdiff_t n )
    {
        m_row.template inc< 0, Ts... >( n );
    }
    void operator-=( ptrdiff_t n )
    {
        m_row.template dec< 0, Ts... >( n );
    }

    frame_row< Ts... >& operator*() { return this->m_row; }
    const frame_row< Ts... >& operator*() const { return this->m_row; }
    frame_row< Ts... >* operator->() { return &this->m_row; }
    const frame_row< Ts... >* operator->() const { return &this->m_row; }

protected:

    template< size_t Ind >
    typename std::tuple_element< Ind, std::tuple< Iter<Ts> ... > >::type
    get_iterator() const   
    {
        auto iter = m_row.template get_iterator<Ind>();
        return iter;
    }

    bool operator==( const base_frame_iterator& other ) const
    {
    }

    bool operator!=( const base_frame_iterator& other ) const
    bool operator<=( const base_frame_iterator& other ) const
    bool operator>=( const base_frame_iterator& other ) const
    bool operator< ( const base_frame_iterator& other ) const
    bool operator> ( const base_frame_iterator& other ) const
    template< 
        template<typename> typename ItLeft, 
        template<typename> typename ItRight, 
        typename ... Us >
    friend ptrdiff_t operator-( 
        const base_frame_iterator<ItLeft, Us...>& left,
        const base_frame_iterator<ItRight, Us...>& right );

    frame_row< Ts... > m_row;
};

template< 
    template<typename> typename ItLeft, 
    template<typename> typename ItRight, 
    typename ... Us >
ptrdiff_t operator-( 
    const base_frame_iterator<ItLeft, Us...>& left,
    const base_frame_iterator<ItRight, Us...>& right )
{
    auto itleft = left.template get_iterator<0>();
    auto itright = right.template get_iterator<0>();
    return itleft - itright;
}

template< typename ... Ts >
class frame_iterator : public base_frame_iterator< series_iterator, Ts... >
{
public:
    frame_iterator( const std::tuple<series_iterator<Ts> ...>& its )
        : base_frame_iterator<series_iterator, Ts...>( its )
    {}

    frame_iterator( std::tuple<series<Ts>...>& cols, int offset )
    {
        std::tuple<Ts*...> ptrs;
        init_impl<0, Ts...>( cols, ptrs, offset );
        this->m_row.set_
    }

    template< size_t Ind, typename U, typename ... Us >
    void init_impl( std::tuple<series<Ts>...>& cols, int offset )
    {
        series<U>& s = std::get< Ind >( cols );
        auto* ptr = s.data();
        this->m_row.template set_pointer< Ind >( begin + offset );
        if constexpr ( sizeof...( Us ) > 0 ) {
            init_impl< Ind+1, Us... >( cols, offset );
        }
    }

    template< typename ... Us >
    frame_iterator< Ts..., Us... > combine( 
        const frame_iterator< Us... >& other )
    {
        auto ptr = tuple_cat( 
            this->m_row.m_iters, other->m_iters );
        frame_iterator< Ts..., Us... > out{ ptr };
        return out;
    }

    frame_iterator<Ts...> operator+( ptrdiff_t n ) const
    {
        frame_iterator<Ts...> out{ *this };
        out += n;
        return out;
    }

    frame_iterator<Ts...> operator-( ptrdiff_t n ) const
    {
        frame_iterator<Ts...> out{ *this };
        out -= n;
        return out;
    }

private:
    template< typename ... Us >
    friend class const_frame_iterator;
};

template< typename ... Ts >
class const_frame_iterator : public base_frame_iterator< const_series_iterator, Ts... >
{
public:
    const_frame_iterator( const std::tuple<const_series_iterator<Ts> ...>& its )
        : base_frame_iterator<const_series_iterator, Ts...>( its )
    {}

    const_frame_iterator( const std::tuple<series<Ts>...>& cols, int offset )
    {
        init_impl<0, Ts...>( cols, offset );
    }

    const_frame_iterator( const frame_iterator<Ts...>& fi )
        : base_frame_iterator<const_series_iterator, Ts...>( fi.m_row.m_iters )
    {
    }

    template< size_t Ind, typename U, typename ... Us >
    void init_impl( const std::tuple<series<Ts>...>& cols, int offset )
    {
        const series<U>& s = std::get< Ind >( cols );
        auto begin = s.cbegin();
        this->m_row.template set_iterator< Ind >( begin + offset );
        if constexpr ( sizeof...( Us ) > 0 ) {
            init_impl< Ind+1, Us... >( cols, offset );
        }
    }

    template< typename ... Us >
    const_frame_iterator< Ts..., Us... > combine( 
        const const_frame_iterator< Us... >& other )
    {
        auto ptr = tuple_cat( 
            this->m_row.m_iters, other->m_iters );
        const_frame_iterator< Ts..., Us... > out{ ptr };
        return out;
    }

    const_frame_iterator<Ts...> operator+( ptrdiff_t n ) const
    {
        const_frame_iterator<Ts...> out{ *this };
        out += n;
        return out;
    }

    const_frame_iterator<Ts...> operator-( ptrdiff_t n ) const
    {
        const_frame_iterator<Ts...> out{ *this };
        out -= n;
        return out;
    }
};

template< typename ... Ts >
class reverse_frame_iterator : public base_frame_iterator< reverse_series_iterator, Ts... >
{
public:
    reverse_frame_iterator( const std::tuple<reverse_series_iterator<Ts> ...>& its )
        : base_frame_iterator<reverse_series_iterator, Ts...>( its )
    {
    }

    reverse_frame_iterator( std::tuple<series<Ts>...>& cols, int offset )
    {
        init_impl<0, Ts...>( cols, offset );
    }

    template< size_t Ind, typename U, typename ... Us >
    void init_impl( std::tuple<series<Ts>...>& cols, int offset )
    {
        series<U>& s = std::get< Ind >( cols );
        auto begin = s.rbegin();
        this->m_row.template set_iterator< Ind >( begin + offset );
        if constexpr ( sizeof...( Us ) > 0 ) {
            init_impl< Ind+1, Us... >( cols, offset );
        }
    }

    template< typename ... Us >
    reverse_frame_iterator< Ts..., Us... > combine( 
        const reverse_frame_iterator< Us... >& other )
    {
        auto ptr = tuple_cat( 
            this->m_row.m_iters, other->m_iters );
        reverse_frame_iterator< Ts..., Us... > out{ ptr };
        return out;
    }

    reverse_frame_iterator<Ts...> operator+( ptrdiff_t n ) const
    {
        reverse_frame_iterator<Ts...> out{ *this };
        out += n;
        return out;
    }

    reverse_frame_iterator<Ts...> operator-( ptrdiff_t n ) const
    {
        reverse_frame_iterator<Ts...> out{ *this };
        out -= n;
        return out;
    }

private:
    template< typename ... Us >
    friend class const_reverse_frame_iterator;
};

template< typename ... Ts >
class const_reverse_frame_iterator : public base_frame_iterator< const_reverse_series_iterator, Ts... >
{
public:
    const_reverse_frame_iterator( const std::tuple<const_reverse_series_iterator<Ts> ...>& its )
        : base_frame_iterator<const_reverse_series_iterator, Ts...>( its )
    {}

    const_reverse_frame_iterator( const std::tuple<series<Ts>...>& cols, int offset )
    {
        init_impl<0, Ts...>( cols, offset );
    }

    const_reverse_frame_iterator( const reverse_frame_iterator<Ts...>& fi )
        : base_frame_iterator<const_reverse_series_iterator, Ts...>( fi.m_row.m_iters )
    {
    }

    template< size_t Ind, typename U, typename ... Us >
    void init_impl( const std::tuple<series<Ts>...>& cols, int offset )
    {
        const series<U>& s = std::get< Ind >( cols );
        auto begin = s.crbegin();
        this->m_row.template set_iterator< Ind >( begin + offset );
        if constexpr ( sizeof...( Us ) > 0 ) {
            init_impl< Ind+1, Us... >( cols, offset );
        }
    }

    template< typename ... Us >
    const_reverse_frame_iterator< Ts..., Us... > combine( 
        const const_reverse_frame_iterator< Us... >& other )
    {
        auto ptr = tuple_cat( 
            this->m_row.m_iters, other->m_iters );
        const_reverse_frame_iterator< Ts..., Us... > out{ ptr };
        return out;
    }

    const_reverse_frame_iterator<Ts...> operator+( ptrdiff_t n ) const
    {
        const_reverse_frame_iterator<Ts...> out{ *this };
        out += n;
        return out;
    }

    const_reverse_frame_iterator<Ts...> operator-( ptrdiff_t n ) const
    {
        const_reverse_frame_iterator<Ts...> out{ *this };
        out -= n;
        return out;
    }
};

template< template<typename> typename It, typename ... Us >
bool operator==( const base_frame_iterator< It, Us... >& l, 
                 const base_frame_iterator< It, Us... >& r )
{
    return l.m_row == r.m_row;
}

template< template<typename> typename It, typename ... Us >                                                          
bool operator!=( const base_frame_iterator< It, Us... >& l, 
                 const base_frame_iterator< It, Us... >& r )
{
    return l.m_row != r.m_row;
}

template< template<typename> typename It, typename ... Us >
bool operator<=( const base_frame_iterator< It, Us... >& l, 
                 const base_frame_iterator< It, Us... >& r )
{
    return l.m_row <= r.m_row;
}

template< template<typename> typename It, typename ... Us >                                                          
bool operator>=( const base_frame_iterator< It, Us... >& l, 
                 const base_frame_iterator< It, Us... >& r )
{
    return l.m_row >= r.m_row;
}

template< template<typename> typename It, typename ... Us >
bool operator<( const base_frame_iterator< It, Us... >& l, 
                 const base_frame_iterator< It, Us... >& r )
{
    return l.m_row < r.m_row;
}

template< template<typename> typename It, typename ... Us >                                                          
bool operator>( const base_frame_iterator< It, Us... >& l, 
                 const base_frame_iterator< It, Us... >& r )
{
    return l.m_row > r.m_row;
}
#endif

} // namespace mf

#endif // INCLUDED_mainframe_frame_iterator_h

