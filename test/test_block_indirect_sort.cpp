//----------------------------------------------------------------------------
/// @file test_block_indirect_sort.cpp
/// @brief Test program of the block_indirect_sort algorithm
///
/// @author Copyright (c) 2016 Francisco Jose Tapia (fjtapia@gmail.com )\n
///         Distributed under the Boost Software License, Version 1.0.\n
///         ( See accompanying file LICENSE_1_0.txt or copy at
///           http://www.boost.org/LICENSE_1_0.txt  )
/// @version 0.1
///
/// @remarks
//-----------------------------------------------------------------------------
#include <algorithm>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <ciso646>
#include <boost/test/included/test_exec_monitor.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/sort/sort.hpp>


namespace bsc = boost::sort::common;
namespace bsp = boost::sort;
using boost::sort::block_indirect_sort;
using bsc::range;


void test1 (void)
{
    typedef std::less< uint64_t > compare;

    const uint32_t NElem = 500000;
    std::vector< uint64_t > V1;
    std::mt19937_64 my_rand (0);
    compare comp;

    for (uint32_t i = 0; i < NElem; ++i) V1.push_back (my_rand ( ) % NElem);
    block_indirect_sort  (V1.begin ( ), V1.end ( ), comp, 2);

    for (unsigned i = 1; i < NElem; i++) {
        BOOST_CHECK (V1[ i - 1 ] <= V1[ i ]);
    };

    V1.clear ( );
    for (uint32_t i = 0; i < NElem; ++i) V1.push_back (i);

    block_indirect_sort ( V1.begin ( ), V1.end ( ), comp, 2);
    for (unsigned i = 1; i < NElem; i++) {
        BOOST_CHECK (V1[ i - 1 ] <= V1[ i ]);
    };

    V1.clear ( );
    for (uint32_t i = 0; i < NElem; ++i) V1.push_back (NElem - i);

    block_indirect_sort ( V1.begin ( ), V1.end ( ), comp, 2);
    for (unsigned i = 1; i < NElem; i++) {
        BOOST_CHECK (V1[ i - 1 ] <= V1[ i ]);
    };

    V1.clear ( );
    for (uint32_t i = 0; i < NElem; ++i) V1.push_back (1000);

    block_indirect_sort (V1.begin ( ), V1.end ( ), comp, 2);
    for (unsigned i = 1; i < NElem; i++) {
        BOOST_CHECK (V1[ i - 1 ] == V1[ i ]);
    };
};

void test2 (void)
{
    std::less< uint64_t > comp;
    std::vector< uint64_t > V;

    for (uint32_t i = 0; i < 2083333; ++i) V.push_back (i);

    block_indirect_sort ( V.begin ( ), V.end ( ),comp, 8);
    for (uint32_t i = 0; i < V.size ( ); ++i) {
        BOOST_CHECK (V[ i ] == i);
    };
};

void test3 (void)
{
    typedef typename std::vector< uint64_t >::iterator iter_t;
    typedef range< iter_t > range_it;

    const uint32_t NELEM = 416667;
    std::vector< uint64_t > A;
    std::less< uint64_t > comp;

    for (uint32_t i = 0; i < 1000; ++i) A.push_back (0);
    for (uint32_t i = 0; i < NELEM; ++i) A.push_back (NELEM - i);
    for (uint32_t i = 0; i < 1000; ++i) A.push_back (0);

    range_it R1 (A.begin ( ) + 1000, A.begin ( ) + (1000 + NELEM));
    block_indirect_sort ( A.begin () + 1000,
                          A.begin () + (1000 + NELEM), comp, 8);
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

    R1 = range_it (A.begin ( ) + 1000, A.begin ( ) + (1000 + NELEM));
    block_indirect_sort ( A.begin ( ) + 1000,
                          A.begin ( ) + (1000 + NELEM), comp, 4);

    for (iter_t it = A.begin ( ) + 1001; it != A.begin ( ) + (1000 + NELEM);
         ++it)
    {
        BOOST_CHECK ((*(it - 1)) <= (*it));
    };
    BOOST_CHECK (A[ 998 ] == 999999999 and A[ 999 ] == 999999999 and
                 A[ 1000 + NELEM ] == 999999999 and
                 A[ 1001 + NELEM ] == 999999999);
};

void test4 (void)
{
    typedef std::less< uint32_t > compare;

    const uint32_t NElem = 1000000;
    std::vector< uint32_t > V1, V2, V3;
    V1.reserve ( NElem ) ;
    std::mt19937 my_rand (0);

    for (uint32_t i = 0; i < NElem; ++i) V1.push_back (my_rand ( ));

    V2 = V1;
    V3 = V1;
    std::sort (V2.begin ( ), V2.end ( ));

    V1 = V3;
    block_indirect_sort ( V1.begin ( ), V1.end ( ), compare(), 2);
    for (unsigned i = 0; i < V1.size(); i++)
    {
        BOOST_CHECK (V1[ i ] == V2[ i ]);
    };

    V1 = V3;
    block_indirect_sort (V1.begin ( ), V1.end ( ), compare(), 4);
    for (unsigned i = 0; i < V1.size(); i++)
    {   BOOST_CHECK (V1[ i ] == V2[ i ]);
    };

    V1 = V3;

    block_indirect_sort (V1.begin ( ), V1.end ( ), compare(), 8);

    for (unsigned i = 0; i < V1.size(); i++)
    {   BOOST_CHECK (V1[ i ] == V2[ i ]);
    };

    V1 = V3;
    block_indirect_sort ( V1.begin ( ), V1.end ( ), compare(), 16);
    for (unsigned i = 0; i < V1.size(); i++)
    {   BOOST_CHECK (V1[ i ] == V2[ i ]);
    };

    V1 = V3;
    block_indirect_sort ( V1.begin ( ), V1.end ( ), compare(), 100);
    for (unsigned i = 1; i < V1.size(); i++)
    {   BOOST_CHECK (V1[ i ] == V2[ i ]);
    };
};

template<uint32_t NN>
struct int_array
{
	uint64_t M[NN];

	int_array(uint64_t number = 0)
	{
		for (uint32_t i = 0; i < NN; ++i)
			M[i] = number;
	}

	bool operator<(const int_array<NN> &A) const
	{
		return M[0] < A.M[0];
	}
};

void test5(void)
{
	namespace bspd = boost::sort::blk_detail;
	BOOST_CHECK(bspd::block_size<0>::data == 4096);

	BOOST_CHECK(bspd::block_size<1>::data == 4096);

	BOOST_CHECK(bspd::block_size<2>::data == 4096);

	BOOST_CHECK(bspd::block_size<3>::data == 4096);
	BOOST_CHECK(bspd::block_size<4>::data == 4096);

	BOOST_CHECK(bspd::block_size<5>::data == 4096);
	BOOST_CHECK(bspd::block_size<6>::data == 4096);
	BOOST_CHECK(bspd::block_size<7>::data == 4096);
	BOOST_CHECK(bspd::block_size<8>::data == 4096);

	BOOST_CHECK(bspd::block_size<9>::data == 2048);
	BOOST_CHECK(bspd::block_size<12>::data == 2048);
	BOOST_CHECK(bspd::block_size<15>::data == 2048);
	BOOST_CHECK(bspd::block_size<16>::data == 2048);

	BOOST_CHECK(bspd::block_size<17>::data == 1024);
	BOOST_CHECK(bspd::block_size<24>::data == 1024);
	BOOST_CHECK(bspd::block_size<31>::data == 1024);
	BOOST_CHECK(bspd::block_size<32>::data == 1024);

	BOOST_CHECK(bspd::block_size<33>::data == 768);
	BOOST_CHECK(bspd::block_size<50>::data == 768);
	BOOST_CHECK(bspd::block_size<63>::data == 768);
	BOOST_CHECK(bspd::block_size<64>::data == 768);

	BOOST_CHECK(bspd::block_size<65>::data == 512);
	BOOST_CHECK(bspd::block_size<100>::data == 512);
	BOOST_CHECK(bspd::block_size<127>::data == 512);
	BOOST_CHECK(bspd::block_size<128>::data == 512);

	BOOST_CHECK(bspd::block_size<129>::data == 256);
	BOOST_CHECK(bspd::block_size<200>::data == 256);
	BOOST_CHECK(bspd::block_size<255>::data == 256);
	BOOST_CHECK(bspd::block_size<256>::data == 256);

	BOOST_CHECK(bspd::block_size<257>::data == 128);
	BOOST_CHECK(bspd::block_size<400>::data == 128);
	BOOST_CHECK(bspd::block_size<511>::data == 128);
	BOOST_CHECK(bspd::block_size<512>::data == 128);
	BOOST_CHECK(bspd::block_size<513>::data == 128);
	BOOST_CHECK(bspd::block_size<600>::data == 128);
};

void test6()
{
	std::less<uint64_t> cmp64;
	std::less<uint32_t> cmp32;
	std::less<uint16_t> cmp16;
	std::less<uint8_t> cmp8;

	std::mt19937_64 my_rand(0);

	const uint32_t NELEM = (1 << 20);
	std::vector<uint64_t> V1, V2;
	V1.reserve(NELEM);
	V2.reserve(NELEM);

	for (uint32_t i = 0; i < NELEM; ++i)
		V1.push_back(my_rand());
	V2 = V1;

	uint64_t *p64 = &V1[0];
	uint32_t *p32 = reinterpret_cast<uint32_t *>(p64);
	uint16_t *p16 = reinterpret_cast<uint16_t *>(p64);
	uint8_t *p8 = reinterpret_cast<uint8_t *>(p64);

	V1 = V2;
	bsp::block_indirect_sort(p64, p64 + NELEM, cmp64, 8);
	for (unsigned i = 1; i < NELEM; i++)
	{
		BOOST_CHECK(p64[i - 1] <= p64[i]);
	};

	V1 = V2;
	bsp::block_indirect_sort(p32, p32 + (NELEM << 1), cmp32, 8);
	for (unsigned i = 1; i < (NELEM << 1); i++)
	{
		BOOST_CHECK(p32[i - 1] <= p32[i]);
	};

	V1 = V2;
	bsp::block_indirect_sort(p16, p16 + (NELEM << 2), cmp16, 8);
	for (unsigned i = 1; i < (NELEM << 2); i++)
	{
		BOOST_CHECK(p16[i - 1] <= p16[i]);
	};

	V1 = V2;
	bsp::block_indirect_sort(p8, p8 + (NELEM << 3), cmp8, 8);
	for (unsigned i = 1; i < (NELEM << 3); i++)
	{
		BOOST_CHECK(p8[i - 1] <= p8[i]);
	};
};

template<class IA>
void test_int_array(uint32_t NELEM)
{
	typedef std::less<IA> compare;
	std::mt19937_64 my_rand(0);

	std::vector<IA> V1;
	V1.reserve(NELEM);
	for (uint32_t i = 0; i < NELEM; ++i)
		V1.emplace_back(my_rand());

	bsp::block_indirect_sort(V1.begin(), V1.end(), compare());
	for (unsigned i = 1; i < NELEM; i++)
	{
		BOOST_CHECK(not (V1[i] < V1[i - 1]));
	};
}

void test7()
{
	test_int_array<int_array<1> >(1u << 20);
	test_int_array<int_array<2> >(1u << 19);
	test_int_array<int_array<4> >(1u << 18);
	test_int_array<int_array<8> >(1u << 17);
	test_int_array<int_array<16> >(1u << 17);
	test_int_array<int_array<32> >(1u << 17);
	test_int_array<int_array<64> >(1u << 17);
	test_int_array<int_array<128> >(1u << 17);
}
void test8()
{
	std::mt19937_64 my_rand(0);
	const uint32_t NELEM = 1 << 20;
	const uint32_t NString = 100000;
	std::vector<uint64_t> V1;
	V1.reserve(NELEM);
	for (uint32_t i = 0; i < NELEM; ++i)
		V1.push_back(my_rand());

	uint64_t *p64 = &V1[0];
	char *pchar = reinterpret_cast<char *>(p64);

	std::string sinput(pchar, (NELEM << 3));

	std::istringstream strm_input(sinput);
	std::string inval;
	std::vector<std::string> V;
	V.reserve(NString);
	strm_input.seekg(0, std::ios_base::beg);

	strm_input.seekg(0, std::ios_base::beg);

	for (size_t i = 0; i < NString; ++i)
	{
		if (!strm_input.eof())
		{
			strm_input >> inval;
			V.push_back(inval);
			inval.clear();
		}
		else
		{
			throw std::ios_base::failure("Insuficient lenght of the file\n");
		};
	};

	typedef std::less<std::string> compare;
	bsp::block_indirect_sort(V.begin(), V.end(), compare());
	for (unsigned i = 1; i < NString; i++)
	{
		BOOST_CHECK(not (V[i] < V[i - 1]));
	};

}

int test_main (int, char *[])
{
    test1  ( );
    test2  ( );
    test3  ( );
    test4  ( );
    test5  ( );
    test6  ( );
    test7  ( );
    test8  ( );

    return 0;
};
