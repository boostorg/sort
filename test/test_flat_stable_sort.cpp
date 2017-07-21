//----------------------------------------------------------------------------
/// @file test_flat_stable_sort.cpp
/// @brief test program of the flat_stable_sort algorithm
///
/// @author Copyright (c) 2017 Francisco José Tapia (fjtapia@gmail.com )\n
///         Distributed under the Boost Software License, Version 1.0.\n
///         ( See accompanying file LICENSE_1_0.txt or copy at
///           http://www.boost.org/LICENSE_1_0.txt  )
/// @version 0.1
///
/// @remarks
//-----------------------------------------------------------------------------
#include <algorithm>
#include <iostream>
#include <random>
#include <vector>
#include <ciso646>
#include <boost/sort/flat_stable_sort/flat_stable_sort.hpp>
#include <boost/test/included/test_exec_monitor.hpp>
#include <boost/test/test_tools.hpp>

using namespace boost::sort;

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

void test2 ( )
{
    uint64_t V1[ 300 ];
    typedef std::less< uint64_t > compare_t;
    compare_t comp;

    for (uint32_t i = 0; i < 200; ++i) V1[ i ] = i;

    indirect_flat_stable_sort (&V1[ 0 ], &V1[ 200 ], comp);
    for (unsigned i = 1; i < 200; i++) {
        BOOST_CHECK (V1[ i - 1 ] <= V1[ i ]);
    };

    for (uint32_t i = 0; i < 200; ++i) V1[ i ] = 199 - i;
    flat_stable_sort (&V1[ 0 ], &V1[ 200 ], comp);
    for (unsigned i = 1; i < 200; i++) {
        BOOST_CHECK (V1[ i - 1 ] <= V1[ i ]);
    };

    for (uint32_t i = 0; i < 300; ++i) V1[ i ] = 299 - i;

    flat_stable_sort (&V1[ 0 ], &V1[ 300 ], comp);
    for (unsigned i = 1; i < 300; i++) {
        BOOST_CHECK (V1[ i - 1 ] <= V1[ i ]);
    };

    for (uint32_t i = 0; i < 300; ++i) V1[ i ] = 88;

    flat_stable_sort (&V1[ 0 ], &V1[ 300 ], comp);
    for (unsigned i = 1; i < 300; i++) {
        BOOST_CHECK (V1[ i - 1 ] <= V1[ i ]);
    };
};

void test3 ( )
{
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
    flat_stable_sort (V1.begin ( ), V1.end ( ), compare_t ( ));
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
    const uint32_t NElem = 500000;
    std::vector< uint64_t > V1;
    std::mt19937_64 my_rand (0);
    compare_t comp;

    for (uint32_t i = 0; i < NElem; ++i) V1.push_back (my_rand ( ) % NElem);

    flat_stable_sort (V1.begin ( ), V1.end ( ), comp);
    for (unsigned i = 1; i < NElem; i++) {
        BOOST_CHECK (V1[ i - 1 ] <= V1[ i ]);
    };

    V1.clear ( );
    for (uint32_t i = 0; i < NElem; ++i) V1.push_back (i);
    flat_stable_sort (V1.begin ( ), V1.end ( ), comp);
    for (unsigned i = 1; i < NElem; i++) {
        BOOST_CHECK (V1[ i - 1 ] <= V1[ i ]);
    };

    V1.clear ( );
    for (uint32_t i = 0; i < NElem; ++i) V1.push_back (NElem - i);
    flat_stable_sort (V1.begin ( ), V1.end ( ), comp);
    for (unsigned i = 1; i < NElem; i++) {
        BOOST_CHECK (V1[ i - 1 ] <= V1[ i ]);
    };

    V1.clear ( );
    for (uint32_t i = 0; i < NElem; ++i) V1.push_back (1000);
    flat_stable_sort (V1.begin ( ), V1.end ( ), comp);
    for (unsigned i = 1; i < NElem; i++) {
        BOOST_CHECK (V1[ i - 1 ] == V1[ i ]);
    };
};

void test5 (void)
{
    typedef std::less< uint64_t > compare;

    const uint32_t KMax = 500000;

    std::vector< uint64_t > K, M;
    std::mt19937_64 my_rand (0);
    compare comp;

    for (uint32_t i = 0; i < KMax; ++i) K.push_back (my_rand ( ));
    M = K;

    flat_stable_sort (K.begin ( ), K.end ( ), comp);
    std::stable_sort (M.begin ( ), M.end ( ), comp);
    for (unsigned i = 0; i < KMax; i++) BOOST_CHECK (M[ i ] == K[ i ]);
};

void test6 (void)
{
    typedef std::less< uint64_t > compare_t;
    std::vector< uint64_t > V;

    for (uint32_t i = 0; i < 2083333; ++i) V.push_back (i);
    flat_stable_sort(V.begin ( ), V.end ( ), compare_t ( ));
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

    flat_stable_sort (A.begin ( ) + 1000,
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

    flat_stable_sort (A.begin ( ) + 1000,
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
    for (uint32_t i = 0; i < NELEM; ++i) A.push_back (NELEM - i);

    flat_stable_sort (A.begin ( ), A.end ( ), comp);
    for (iter_t it = A.begin ( ) + 1; it != A.end ( ); ++it) {
        if ((*(it - 1)) > (*it)) std::cout << "error 1\n";
    };
    BOOST_CHECK (B[ 998 ] == 0 and B[ 999 ] == 0 and B[ 1000 + N1 ] == 0 and
                 B[ 1001 + N1 ] == 0);

    for (uint32_t i = 0; i < B.size ( ); ++i) B[ i ] = 999999999;
    A.clear ( );
    for (uint32_t i = 0; i < NELEM; ++i) A.push_back (NELEM - i);
    flat_stable_sort (A.begin ( ), A.end ( ), comp);

    for (iter_t it = A.begin ( ) + 1; it != A.end ( ); ++it) {
        BOOST_CHECK ((*(it - 1)) <= (*it));
    };
    BOOST_CHECK (B[ 998 ] == 999999999 and B[ 999 ] == 999999999 and
                 B[ 1000 + N1 ] == 999999999 and B[ 1001 + N1 ] == 999999999);
};
void test8 (void)
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
	flat_stable_sort( V.begin() , V.end(), compare_t());
	for ( uint32_t i =0 ; i < ( NELEM * 10); ++i)
	{	BOOST_CHECK ( V[i].num == (i / 10) and V[i].tail == (i %10) );
	};
}
int test_main (int, char *[])
{
    test2 ( );
    test3 ( );
    test4 ( );
    test5 ( );
    test6 ( );
    test7 ( );
    test8 ( );
    return 0;
};
