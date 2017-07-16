//----------------------------------------------------------------------------
/// @file spin_sort.hpp
/// @brief Spin Sort algorithm
///
/// @author Copyright (c) 2016 Francisco José Tapia (fjtapia@gmail.com )\n
///         Distributed under the Boost Software License, Version 1.0.\n
///         ( See accompanying file LICENSE_1_0.txt or copy at
///           http://www.boost.org/LICENSE_1_0.txt  )
/// @version 0.1
///
/// @remarks
//-----------------------------------------------------------------------------
#ifndef __BOOST_SORT_PARALLEL_ALGORITHM_SPIN_SORT_HPP
#define __BOOST_SORT_PARALLEL_ALGORITHM_SPIN_SORT_HPP

#include <boost/sort/parallel/detail/indirect.hpp>
#include <boost/sort/parallel/detail/insertion_sort.hpp>
#include <boost/sort/parallel/detail/util/compare_traits.hpp>
#include <boost/sort/parallel/detail/util/nbits.hpp>
#include <boost/sort/parallel/detail/util/range.hpp>
#include <cstdlib>
#include <functional>
#include <iterator>
#include <memory>
#include <type_traits>
#include <vector>
#include <cstddef>

namespace boost
{
namespace sort
{
namespace parallel
{
namespace detail
{
//
//############################################################################
//                                                                          ##
//                           S T R U C T                                    ##
//                                                                          ##
//                  C H E C K _ S T A B L E _ S O R T                       ##
//                                                                          ##
//############################################################################
//
//---------------------------------------------------------------------------
/// @struct check_stable_sort
/// @brief  This class implement s stable sort algorithm with 1 thread, with
///         an auxiliary memory of N/2 elements
//----------------------------------------------------------------------------
template < class Iter1_t, class Iter2_t, class Compare >
class check_stable_sort
{
    //------------------------------------------------------------------------
    //                USING SENTENCES
    //------------------------------------------------------------------------
    using util::range;
    using util::nbits64;
    using util::is_sorted;
    using std::iterator_traits;

    //------------------------------------------------------------------------
    //               METAPROGRAMMING
    //------------------------------------------------------------------------
    typedef value_type <Iter1_t> value_t;
    typedef value_type <Iter2_t> value2_t;
    static_assert (std::is_same< value_t, value1_t>::value,
                   "Incompatible iterators\n");

    //------------------------------------------------------------------------
    //               DEFINITIONS AND CONSTANTS
    //------------------------------------------------------------------------
    typedef range<Iter1_t> range_it1;
    typedef range<Iter2_t> range_it2;


    bool operator ()  (const range_it1 & rng_data,
                       const range_it2 & rng_aux, Compare comp );

    void insert_partial_sort (Iter1_t first, Iter1_t mid, Iter1_t last,
                              const range_it2 & rng_aux, Compare comp);
}; //        End of class check_stable_sort
//
//############################################################################
//                                                                          ##
//              N O N    I N L I N E      F U N C T I O N S                 ##
//                                                                          ##
//############################################################################
//
//-----------------------------------------------------------------------------
//  function : check_stable_sort
/// @brief check if the elements between first and last are osted or reverse
///        sorted. If the number of elements not sorted is small, insert in
///        the sorted part
/// @param rng_data : range with the elements to sort
/// @param rng_aux : range of auxiliary memory
//
/// @comments : if the number of levels is odd, the data are in the first
/// parameter of range_sort, and the results appear in the second parameter
/// If the number of levels is even, the data are in the second
/// parameter of range_sort, and the results are in the same parameter
//-----------------------------------------------------------------------------
template < class Iter1_t, class Iter2_t, class Compare>
bool check_stable_sort<Iter1_t, Iter2_t, Compare>
::operator () ( const range_it1 & rng_data,
                const range_it2 & rng_aux , Compare comp)
{
    if ( rng_data.size() < 2 ) return true ;

    // check if sorted
    bool sw = true ;
    Iter1_t it2 = rng_data.first +1 ;
    for (Iter1_t it1 = rng_data.first;
         it2 != rng_data.last and (sw = not comp(*it2,*it1)); it1= it2++ );
    if ( sw ) return true ;

    // insert the elements between it1 and last
    if ( (rng_data.last - it2) < min_insert_partial_sort )
    {   spin_sort<Iter1_t, Compare, Iter2_t> ( it2, rng_data.last, comp, rng_aux);
        insert_partial_sort ( rng_data.first, it2, rng_data.last , rng_aux);
        return true ;
    };

    // check if reverse sorted
    if ( (it2 != (rng_data.first + 1))) return false ;
    sw = true ;
    for (Iter1_t it1 = rng_data.first ;
         it2 != rng_data.last and (sw = comp(*it2,*it1)); it1= it2++ );
    if ((rng_data.last - it2) >= min_insert_partial_sort ) return false ;

    // reverse the elements between first and it1
    size_t nreverse = it2 - rng_data.first ;
    Iter1_t alpha (rng_data.first), beta (it2 - 1), mid (rng_data.first + (nreverse >>1));
    while ( alpha != mid) std::swap (*(alpha++), *(beta--) );

    // insert the elements between it1 and last
    if ( it2 != rng_data.last )
    {   spin_sort<Iter1_t, Compare, Iter2_t> ( it2, rng_data.last, comp, rng_aux);
        insert_partial_sort ( rng_data.first, it2, rng_data.last , rng_aux);
    }
    return true;
};
//
//-----------------------------------------------------------------------------
//  function : insert_partial_sort
/// @brief : Insertion sort of elements sorted
/// @param first: iterator to the first element of the range
/// @param mid : last pointer of the sorted data, and first pointer to the
///               elements to insert
/// @param last : iterator to the next element of the last in the range
/// @param comp :
/// @comments : the two ranges are sorted
//-----------------------------------------------------------------------------
template < class Iter1_t, class Iter2_t, typename Compare>
void check_stable_sort<Iter1_t, Iter2_t, Compare>
::insert_partial_sort (Iter1_t first, Iter1_t mid, Iter1_t last, Compare comp,
                       const range<Iter2_t> &rng_aux)
{
    //--------------------------------------------------------------------
    //                   DEFINITIONS
    //--------------------------------------------------------------------
    //typedef typename std::iterator_traits< Iter_t >::value_type value_t;

    if ( mid == last ) return ;
    //insertion_sort ( mid, last, comp);
    if (first == mid) return ;

    // creation of the vector of elements to insert and their position in the
    // sorted part
    std::vector<Iter1_t> viter ;
    Iter2_t beta = rng_aux.first , data = rng_aux.first;
    for ( Iter1_t alpha = mid ; alpha != last ; ++alpha)
        * (beta++) =  std::move ( *alpha);
    size_t ndata = last - mid ;

    Iter1_t linf = first , lsup = mid ;
    for ( uint32_t i= 0 ; i < ndata ; ++i)
    {   Iter1_t it1 = std::upper_bound ( linf, lsup , *(data + i), comp);
        viter.push_back ( it1 );
        linf = it1 ;
    };

    // moving the elements
    viter.push_back ( mid) ;
    for ( uint32_t i = viter.size() -1 ; i!= 0 ; --i)
    {   Iter1_t src = viter[i], limit = viter[i-1];
        Iter1_t dest = src + ( i);
        while ( src != limit) * (--dest) = std::move ( *(--src));
        *(viter[i-1] + (i -1)) = std::move (* (data + (i-1)));
    };
};
//
//############################################################################
//                                                                          ##
//            F U N C T I O N      R A N G E _ S O R T                      ##
//                                                                          ##
//############################################################################
//-----------------------------------------------------------------------------
//  function : range_sort
/// @brief this function divide r_input in two parts, sort it,and merge moving
///        the elements to range_buf
/// @param range_input : range with the elements to sort
/// @param range_buffer : range with the elements sorted
/// @param comp : object for to compare two elements
/// @param level : when is 1, sort with the insertion_sort algorithm
///                if not make a recursive call splitting the ranges
//
/// @comments : if the number of levels is odd, the data are in the first
/// parameter of range_sort, and the results appear in the second parameter
/// If the number of levels is even, the data are in the second
/// parameter of range_sort, and the results are in the same parameter
//-----------------------------------------------------------------------------
template < class Iter1_t, class Iter2_t, class Compare>
void range_sort (const range< Iter1_t > &range1,
                 const range< Iter2_t > &range2,
                 Compare comp , uint32_t level)
{
    //-----------------------------------------------------------------------
    //                  metaprogram
    //-----------------------------------------------------------------------
    typedef range< Iter1_t > range_it1;
    typedef range< Iter2_t > range_it2;
    assert (range1.size ( ) == range2.size ( ) and level != 0);

    //-----------------------------------------------------------------------
    //                  program
    //-----------------------------------------------------------------------

    //------------------- check if sort --------------------------------------
    if (range1.size() > 1024)
    {   if ( (level & 1) == 0 )
        {   if (check_stable_sort (range2, range1)) return ;
        }
        else
        {   if ( check_stable_sort ( range1, range2))
            {   init_move ( range2, range1);
                return ;
            };
        };
    };
    //------------------- normal process -----------------------------------
    size_t nelem1 = (range1.size ( ) + 1) >> 1;
    range_it1 range_input1 (range1.first, range1.first + nelem1),
        range_input2 (range1.first + nelem1, range1.last);

    if (level < 2) {
        insertion_sort (range_input1.first, range_input1.last, comp);
        insertion_sort (range_input2.first, range_input2.last, comp);
    }
    else
    {
        range_sort (range_it2 (range2.first, range2.first + nelem1),
                    range_input1, level - 1);

        range_sort (range_it2 (range2.first + nelem1, range2.last),
                    range_input2, level - 1);
    };

    full_merge (range2, range_input1, range_input2, comp);
};
//
//############################################################################
//                                                                          ##
//                           S T R U C T                                    ##
//                                                                          ##
//                        S P I N _ S O R T                                 ##
//                                                                          ##
//############################################################################
//
//---------------------------------------------------------------------------
/// @struct spin_sort
/// @brief  This class implement s stable sort algorithm with 1 thread, with
///         an auxiliary memory of N/2 elements
//----------------------------------------------------------------------------
template < class Iter1_t, class Compare = util::compare_iter< Iter_t >,
           class Iter2_t = * value_type <Iter1_t> >
class spin_sort
{
    //------------------------------------------------------------------------
    //                USING SENTENCES
    //------------------------------------------------------------------------
    using util::range;
    using util::nbits64;
    using util::is_sorted;
    using std::iterator_traits;

    //------------------------------------------------------------------------
    //               METAPROGRAMMING
    //------------------------------------------------------------------------
    typedef value_type <Iter1_t> value_t;
    typedef value_type <Iter2_t> value2_t;
    static_assert (std::is_same< value_t, value1_t>::value,
                   "Incompatible iterators\n");

    //------------------------------------------------------------------------
    //               DEFINITIONS AND CONSTANTS
    //------------------------------------------------------------------------
    typedef range<Iter1_t> range_it1;
    typedef range<Iter2_t> range_it2;

    // When the number of elements to sort is smaller than Sort_min, are sorted
    // by the insertion sort algorithm
    static const uint32_t Sort_min = 36;

    //------------------------------------------------------------------------
    //                      VARIABLES
    //------------------------------------------------------------------------
    // Pointer to the auxiliary memory
    range_it2 rng_aux;

    // Comparison object
    Compare comp ;

    //------------------------------------------------------------------------
    // construct indicate if the auxiliary memory in initialized, and owner
    // indicate if the auxiliary memory had been created inside the object or
    // had been received as a parameter
    //------------------------------------------------------------------------
    bool construct = false, owner = false;

public:
    //------------------------------------------------------------------------
    //                   PUBLIC CONSTRUCTORS
    //-------------------------------------------------------------------------
    spin_sort (Iter1_t first, Iter1_t last, Compare cmp = Compare ( ))
        : spin_sort (first, last, cmp, range_it2 (nullptr, nullptr){};

    spin_sort (Iter_t first, Iter_t last, Compare cmp,
               const range_it2 & range_aux){};

    //-----------------------------------------------------------------------
    //  function :~spin_sort
    /// @brief destructor of the struct. Destroy the elements if construct is
    /// true,
    ///        and return the memory if owner is true
    //-----------------------------------------------------------------------
    ~spin_sort (void)
    {
        if (construct) {
            destroy (range< value_t * > (ptr, ptr + nptr));
            construct = false;
        };
        if (owner and ptr != nullptr) std::return_temporary_buffer (ptr);
    };
}; //        End of class spin_sort
//
//############################################################################
//                                                                          ##
//              N O N    I N L I N E      F U N C T I O N S                 ##
//                                                                          ##
//                                                                          ##
//############################################################################
//
//-----------------------------------------------------------------------------
//  function : spin_sort
/// @brief private constructor of the struct spin_sort
//
/// @param first : iterator to the first element of the range to sort
/// @param last : iterator after the last element to the range to sort
/// @param comp : object for to compare two elements pointed by Iter_t
///               iterators
/// @param paux : pointer to the auxiliary memory provided. If nullptr, the
///               memory is created inside the class
/// @param naux : number of elements pointed by paux
//-----------------------------------------------------------------------------
template < class Iter1_t, class Compare, class Iter2_t >
spin_sort< Iter1_t, Compare, Iter2_t >
::spin_sort(Iter1_t first, Iter1_t last, Compare cmp, const range_it2 & rng_buf)
    : comp (cmp)
{
    range< Iter_t > range_input (first, last);
    assert (range_input.valid ( ));

    size_t nelem = range_input.size ( );
    owner = construct = false;

    nptr = (nelem + 1) >> 1;
    size_t nelem_1 = nptr;
    size_t nelem_2 = nelem - nelem_1;

    if (nelem <= (Sort_min << 1)) {
        insertion_sort (range_input.first, range_input.last, comp);
        return;
    };
    //if (check_stable_sort (range_input.first, range_input.last)) return ;
/*
    //------------------- check if sort --------------------------------------
    bool sw = true;
    for (Iter_t it1 = first, it2 = first + 1;
         it2 != last and (sw = not comp (*it2, *it1)); it1 = it2++);
    if (sw) return;

    //------------------- check if reverse sort ---------------------------
    sw = true;
    for (Iter_t it1 = first, it2 = first + 1;
         it2 != last and (sw = comp (*it2, *it1)); it1 = it2++) ;
    if (sw)
    {	size_t nelem2 = nelem >> 1;
    	Iter_t it1 = first, it2 = last -1 ;
    	for ( size_t i =0 ; i < nelem2 ; ++i)
    		std::swap ( * ( it1++), * (it2--));
    	return ;
    };
*/
    if (ptr == nullptr) {
        ptr = std::get_temporary_buffer< value_t > (nptr).first;
        if (ptr == nullptr) throw std::bad_alloc ( );
        owner = true;
    };
    range_buf range_aux (ptr, (ptr + nptr));

    //------------------------------------------------------------------------
    //                  Process
    //------------------------------------------------------------------------
    uint32_t nlevel = nbits64 (((nelem + Sort_min - 1) / Sort_min) - 1) - 1;
    assert (nlevel != 0);

    if ((nlevel & 1) == 1) {
        //---------------------------------------------------------------------
        // if the number of levels is odd, the data are in the first parameter
        // of range_sort, and the results appear in the second parameter
        //---------------------------------------------------------------------
        range_it range_1 (first, first + nelem_2),
            range_2 (first + nelem_2, last);
        range_aux = uninit_move (range_aux, range_2);
        construct = true;

        range_sort (range_aux, range_2, nlevel);
        range_buf rng_bx (range_aux.first, range_aux.first + nelem_2);

        range_sort (range_1, rng_bx,  nlevel);
        half_merge (range_input, rng_bx, range_2, comp);
    }
    else
    {
        //--------------------------------------------------------------------
        // If the number of levels is even, the data are in the second
        // parameter of range_sort, and the results are in the same parameter
        //---------------------------------------------------------------------
        range_it range_1 (first, first + nelem_1),
            range_2 (first + nelem_1, last);
        range_aux = uninit_move (range_aux, range_1);
        construct = true;

        range_sort (range_1, range_aux,  nlevel);

        range_1.last = range_1.first + range_2.size ( );
        range_sort (range_1, range_2,  nlevel);
        half_merge (range_input, range_aux, range_2, comp);
    };
};


//****************************************************************************
}; //    End namespace algorithm
}; //    End namespace parallel
}; //    End namespace sort
}; //    End namepspace boost
//****************************************************************************
//
#endif
