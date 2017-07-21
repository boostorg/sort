//----------------------------------------------------------------------------
/// @file test_spinsort.cpp
/// @brief test program of the spinsort algorithm
///
/// @author Copyright (c) 2016 Francisco José Tapia (fjtapia@gmail.com )\n
///         Distributed under the Boost Software License, Version 1.0.\n
///         ( See accompanying file LICENSE_1_0.txt or copy at
///           http://www.boost.org/LICENSE_1_0.txt  )
/// @version 0.1
///
/// @remarks
//-----------------------------------------------------------------------------
#include <ciso646>
#include <algorithm>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <random>
#include <boost/sort/spinsort/spinsort.hpp>
#include <boost/test/included/test_exec_monitor.hpp>
#include <boost/test/test_tools.hpp>


using namespace boost::sort;
using spin_detail::check_stable_sort;
using spin_detail::range_sort;
using common::range;

void test2 ( );
void test3 ( );
void test4 ( );
void test5 ( );
void test6 ( );
void test7 ( );

struct xk
{
    unsigned tail : 4;
    unsigned num : 28;
    xk ( uint32_t n =0 , uint32_t t =0): tail (t), num(n){};
    bool operator< (xk A) const { return (num < A.num); };
};

void test1 ( )
{

    std::mt19937_64 my_rand (0);
    typedef typename std::vector< uint64_t >::iterator iter_t;
    typedef std::less< uint64_t > compare;
    typedef range< iter_t > range_it;

    std::vector< uint64_t > V1, V2, V3;
    uint32_t NELEM = 0;

    //----------------------------------------------------------------------
    // Range of 40 , randomly filled level 1
    //----------------------------------------------------------------------
    NELEM = 40;
    for (uint32_t i = 0; i < NELEM; ++i) {
        V1.push_back (my_rand ( ) % 10000);
    };
    V2 = V1;
    range_it R2 (V2.begin ( ), V2.end ( ));

    V3.resize (NELEM, 0);
    range_it RAux (V3.begin ( ), V3.end ( ));
    range_sort (R2, RAux, compare ( ), 1);

    std::sort (V1.begin ( ), V1.end ( ));

    for (uint32_t i = 0; i < NELEM; ++i) {
        BOOST_CHECK (V1[ i ] == V3[ i ]);
    };

    //----------------------------------------------------------------------
    // Range of 75, randomly filled , level 2
    //---------------------------------------------------------------------
    V1.clear ( );
    V2.clear ( );
    V3.clear ( );
    NELEM = 75;

    for (uint32_t i = 0; i < NELEM; ++i) {
        V1.push_back (my_rand ( ) % 10000);
    };
    V2 = V1;
    R2 = range_it (V2.begin ( ), V2.end ( ));

    V3.resize (NELEM, 0);
    RAux = range_it (V3.begin ( ), V3.end ( ));
    range_sort (RAux, R2, compare ( ), 2);
    std::sort (V1.begin ( ), V1.end ( ));

    for (uint32_t i = 0; i < NELEM; ++i) {
        BOOST_CHECK (V1[ i ] == V2[ i ]);
    };

    //----------------------------------------------------------------------
    // Range of 200, randomly filled , level 3
    //---------------------------------------------------------------------
    V1.clear ( );
    V2.clear ( );
    V3.clear ( );
    NELEM = 200;

    for (uint32_t i = 0; i < NELEM; ++i) {
        V1.push_back (my_rand ( ) % 10000);
    };
    V2 = V1;
    R2 = range_it (V2.begin ( ), V2.end ( ));

    V3.resize (NELEM, 0);
    RAux = range_it (V3.begin ( ), V3.end ( ));

    range_sort (R2, RAux, compare ( ), 3);

    std::sort (V1.begin ( ), V1.end ( ));

    for (uint32_t i = 0; i < NELEM; ++i) {
        BOOST_CHECK (V1[ i ] == V3[ i ]);
    };

    //----------------------------------------------------------------------
    // Range of 400, randomly filled , level 4
    //---------------------------------------------------------------------
    V1.clear ( );
    V2.clear ( );
    V3.clear ( );
    NELEM = 400;

    for (uint32_t i = 0; i < NELEM; ++i) {
        V1.push_back (my_rand ( ) % 10000);
    };
    V2 = V1;
    R2 = range_it (V2.begin ( ), V2.end ( ));

    V3.resize (NELEM, 0);
    RAux = range_it (V3.begin ( ), V3.end ( ));

    range_sort (RAux, R2, compare ( ), 4);
    std::sort (V1.begin ( ), V1.end ( ));

    for (uint32_t i = 0; i < NELEM; ++i) {
        BOOST_CHECK (V1[ i ] == V2[ i ]);
    };

};

void test2 ( )
{
    uint64_t V1[ 300 ];
    typedef std::less< uint64_t > compare_t;
    compare_t comp;

    for (uint32_t i = 0; i < 200; ++i) V1[ i ] = i;
    indirect_spinsort (&V1[ 0 ], &V1[ 200 ], comp);
    //spinsort< uint64_t *, compare_t > (&V1[ 0 ], &V1[ 200 ], comp);
    for (unsigned i = 1; i < 200; i++) {
        BOOST_CHECK (V1[ i - 1 ] <= V1[ i ]);
    };

    for (uint32_t i = 0; i < 200; ++i) V1[ i ] = 199 - i;
    spinsort< uint64_t *, compare_t > (&V1[ 0 ], &V1[ 200 ], comp);
    for (unsigned i = 1; i < 200; i++) {
        BOOST_CHECK (V1[ i - 1 ] <= V1[ i ]);
    };

    for (uint32_t i = 0; i < 300; ++i) V1[ i ] = 299 - i;

    spinsort< uint64_t *, compare_t > (&V1[ 0 ], &V1[ 300 ], comp);
    for (unsigned i = 1; i < 300; i++) {
        BOOST_CHECK (V1[ i - 1 ] <= V1[ i ]);
    };

    for (uint32_t i = 0; i < 300; ++i) V1[ i ] = 88;

    spinsort< uint64_t *, compare_t > (&V1[ 0 ], &V1[ 300 ], comp);
    for (unsigned i = 1; i < 300; i++) {
        BOOST_CHECK (V1[ i - 1 ] <= V1[ i ]);
    };
};

void test3 ( )
{
    typedef typename std::vector< xk >::iterator iter_t;
    typedef std::less< xk > compare_t;
    std::mt19937_64 my_rand (0);

    const uint32_t NMAX = 500000;


    std::vector< xk > V1, V2, V3;
    V1.reserve (NMAX);
    for (uint32_t i = 0; i < 8; ++i) {
        for (uint32_t k = 0; k < NMAX; ++k) {
            uint32_t NM = my_rand ( );
            xk G;
            G.num = NM >> 3;
            G.tail = i;
            V1.push_back (G);
        };
    };
    V3 = V2 = V1;
    spinsort< iter_t, compare_t > (V1.begin ( ), V1.end ( ), compare_t ( ));
    std::stable_sort (V2.begin ( ), V2.end ( ));

    BOOST_CHECK (V1.size ( ) == V2.size ( ));
    for (uint32_t i = 0; i < V1.size ( ); ++i) {
        BOOST_CHECK (V1[ i ].num == V2[ i ].num and
                     V1[ i ].tail == V2[ i ].tail);
    };
};

void test4 (void)
{
    typedef std::less< uint64_t > compare_t;
    typedef typename std::vector< uint64_t >::iterator iter_t;

    const uint32_t NElem = 500000;
    std::vector< uint64_t > V1;
    std::mt19937_64 my_rand (0);
    compare_t comp;

    for (uint32_t i = 0; i < NElem; ++i) V1.push_back (my_rand ( ) % NElem);

    spinsort< iter_t, compare_t > (V1.begin ( ), V1.end ( ), comp);
    for (unsigned i = 1; i < NElem; i++) {
        BOOST_CHECK (V1[ i - 1 ] <= V1[ i ]);
    };

    V1.clear ( );
    for (uint32_t i = 0; i < NElem; ++i) V1.push_back (i);
    spinsort< iter_t, compare_t > (V1.begin ( ), V1.end ( ), comp);
    for (unsigned i = 1; i < NElem; i++) {
        BOOST_CHECK (V1[ i - 1 ] <= V1[ i ]);
    };

    V1.clear ( );
    for (uint32_t i = 0; i < NElem; ++i) V1.push_back (NElem - i);
    spinsort< iter_t, compare_t > (V1.begin ( ), V1.end ( ), comp);
    for (unsigned i = 1; i < NElem; i++) {
        BOOST_CHECK (V1[ i - 1 ] <= V1[ i ]);
    };

    V1.clear ( );
    for (uint32_t i = 0; i < NElem; ++i) V1.push_back (1000);
    spinsort< iter_t, compare_t > (V1.begin ( ), V1.end ( ), comp);
    for (unsigned i = 1; i < NElem; i++) {
        BOOST_CHECK (V1[ i - 1 ] == V1[ i ]);
    };
};

void test5 (void)
{
    typedef std::vector< uint64_t >::iterator iter_t;
    typedef std::less< uint64_t > compare;

    const uint32_t KMax = 500000;

    std::vector< uint64_t > K, M;
    std::mt19937_64 my_rand (0);
    compare comp;

    for (uint32_t i = 0; i < KMax; ++i) K.push_back (my_rand ( ));
    M = K;

    uint64_t *Ptr = std::get_temporary_buffer< uint64_t > (KMax >> 1).first;
    if (Ptr == nullptr) throw std::bad_alloc ( );
    range< uint64_t * > Rbuf (Ptr, Ptr + (KMax >> 1));

    spin_detail::spinsort< iter_t, compare > (K.begin ( ), K.end ( ), comp, Rbuf);

    std::return_temporary_buffer (Ptr);

    std::stable_sort (M.begin ( ), M.end ( ), comp);
    for (unsigned i = 0; i < KMax; i++) BOOST_CHECK (M[ i ] == K[ i ]);
};

void test6 (void)
{
    typedef std::vector< uint64_t >::iterator iter_t;
    typedef std::less< uint64_t > compare_t;
    std::vector< uint64_t > V;

    for (uint32_t i = 0; i < 2083333; ++i) V.push_back (i);
    spinsort< iter_t, compare_t > (V.begin ( ), V.end ( ), compare_t ( ));
    for (uint32_t i = 0; i < V.size ( ); ++i) {
        BOOST_CHECK (V[ i ] == i);
    };
};

void test7 (void)
{
    typedef typename std::vector< uint64_t >::iterator iter_t;
    typedef std::less< uint64_t > compare_t;

    compare_t comp;
    const uint32_t NELEM = 416667;
    const uint32_t N1 = (NELEM + 1) / 2;
    std::vector< uint64_t > A;

    for (uint32_t i = 0; i < 1000; ++i) A.push_back (0);
    for (uint32_t i = 0; i < NELEM; ++i) A.push_back (NELEM - i);
    for (uint32_t i = 0; i < 1000; ++i) A.push_back (0);

    spinsort< iter_t, compare_t > (A.begin ( ) + 1000,
                                    A.begin ( ) + (1000 + NELEM), comp);

    for (iter_t it = A.begin ( ) + 1000; it != A.begin ( ) + (1000 + NELEM);
         ++it)
    {
        BOOST_CHECK ((*(it - 1)) <= (*it));
    };
    BOOST_CHECK (A[ 998 ] == 0 and A[ 999 ] == 0 and A[ 1000 + NELEM ] == 0 and
                 A[ 1001 + NELEM ] == 0);


    A.clear ( );
    for (uint32_t i = 0; i < 1000; ++i) A.push_back (999999999);
    for (uint32_t i = 0; i < NELEM; ++i) A.push_back (NELEM - i);
    for (uint32_t i = 0; i < 1000; ++i) A.push_back (999999999);

    spinsort< iter_t, compare_t > (A.begin ( ) + 1000,
                                    A.begin ( ) + (1000 + NELEM), comp);

    for (iter_t it = A.begin ( ) + 1001; it != A.begin ( ) + (1000 + NELEM);
         ++it)
    {
        BOOST_CHECK ((*(it - 1)) <= (*it));
    };
    BOOST_CHECK (A[ 998 ] == 999999999 and A[ 999 ] == 999999999 and
                 A[ 1000 + NELEM ] == 999999999 and
                 A[ 1001 + NELEM ] == 999999999);

    std::vector< uint64_t > B (N1 + 2000, 0);

    A.clear ( );
    range< uint64_t * > Rbuf (&B[ 1000 ], (&B[ 1000 ]) + N1);
    for (uint32_t i = 0; i < NELEM; ++i) A.push_back (NELEM - i);

    spin_detail::spinsort< iter_t, compare_t > (A.begin ( ), A.end ( ),
    		                                    comp, Rbuf);
    for (iter_t it = A.begin ( ) + 1; it != A.end ( ); ++it) {
        if ((*(it - 1)) > (*it)) std::cout << "error 1\n";
    };
    BOOST_CHECK (B[ 998 ] == 0 and B[ 999 ] == 0 and B[ 1000 + N1 ] == 0 and
                 B[ 1001 + N1 ] == 0);

    for (uint32_t i = 0; i < B.size ( ); ++i) B[ i ] = 999999999;
    A.clear ( );
    for (uint32_t i = 0; i < NELEM; ++i) A.push_back (NELEM - i);
    spin_detail::spinsort< iter_t, compare_t > (A.begin ( ), A.end ( ),
    		                                    comp, Rbuf);

    for (iter_t it = A.begin ( ) + 1; it != A.end ( ); ++it) {
        BOOST_CHECK ((*(it - 1)) <= (*it));
    };
    BOOST_CHECK (B[ 998 ] == 999999999 and B[ 999 ] == 999999999 and
                 B[ 1000 + N1 ] == 999999999 and B[ 1001 + N1 ] == 999999999);
};
void test8 (void)
{
    typedef typename std::vector<uint64_t>::iterator    iter_t ;
    typedef range<iter_t>                               range_it ;
    std::less<uint64_t> comp ;

    std::vector<uint64_t> V = { 1,  3,  5,  7,  9, 11, 13, 15, 17, 19,
                               21, 23, 25, 27, 29, 31, 33, 35, 37, 39,
                               41, 43, 45, 47, 49, 51, 53, 55, 57, 59,
                               14,  2,  4,  6,  8, 10, 12, 16, 18, 20};
    range_it rdata (V.begin() , V.end());
    std::vector<uint64_t> aux (40,0 );
    range_it raux ( aux.begin() , aux.end());

    check_stable_sort ( rdata, raux, comp );
    for ( uint32_t i =0 ; i < V.size() ; ++i)
        std::cout<<V[i]<<", ";
    std::cout<<std::endl;

    V = {59, 57, 55, 53, 51, 49, 47, 45, 43, 41,
         39, 37, 35, 33, 31, 29, 27, 25, 23, 21,
         19, 17, 15, 13, 11,  9,  7,  5,  3,  1,
         14,  2,  6, 16, 18, 20, 12,  4,  8, 10};
    rdata = range_it (V.begin() , V.end());
    aux.assign (40,0);
    raux = range_it ( aux.begin() , aux.end());
    check_stable_sort ( rdata, raux, comp );
    for ( uint32_t i =0 ; i < V.size() ; ++i)
        std::cout<<V[i]<<", ";
    std::cout<<std::endl;

}
void test9 (void)
{
	typedef typename std::vector<xk>::iterator 	iter_t;
	typedef std::less<xk>       					compare_t;
	std::mt19937 my_rand (0);
	std::vector<xk> V ;
	const uint32_t NELEM = 1000000;
	V.reserve(NELEM * 10);


	for (uint32_t k =0 ; k < 10 ; ++k)
	{	for ( uint32_t i =0 ; i < NELEM ; ++i)
		{	V.emplace_back(i , k);
		};
		iter_t first = V.begin() + (k * NELEM);
		iter_t last = first + NELEM ;
		std::shuffle( first, last, my_rand);
	};
	spinsort( V.begin() , V.end(), compare_t());
	for ( uint32_t i =0 ; i < ( NELEM * 10); ++i)
	{	BOOST_CHECK ( V[i].num == (i / 10) and V[i].tail == (i %10) );
	};
}
int test_main (int, char *[])
{
    test1 ( );
    test2 ( );
    test3 ( );
    test4 ( );
    test5 ( );
    test6 ( );
    test7 ( );
    test8 ( );
    test9 ( );
    return 0;
};
