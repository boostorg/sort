/*
  Copyright (c) Alexander Zaitsev <zamazan4ik@gmail.com>, 2016
  Distributed under the Boost Software License, Version 1.0. (See
  accompanying file LICENSE_1_0.txt or copy at
  http://www.boost.org/LICENSE_1_0.txt)
  See http://www.boost.org/ for latest version.
*/

#include <boost/sort/timsort.hpp>
// Include unit test framework
#include <boost/test/included/test_exec_monitor.hpp>
#include <boost/test/test_tools.hpp>
#include <vector>
#include <cstdlib>
#include <algorithm>

using namespace std;
using namespace boost::sort;

struct A
{
    int a;
    int b;

    A(int val_a, int val_b) : a(val_a), b(val_b) {}

    bool operator<(const A& val) const
    {
        return a < val.a;
    }

    bool operator==(const A& val) const
    {
        return a == val.a && b == val.b;
    }
};


boost::int32_t
rand_32(bool sign = true) {
    boost::int32_t result = rand() | (rand()<< 16);
    if (rand() % 2)
        result |= 1 << 15;
    //Adding the sign bit
    if (sign && (rand() % 2))
        result *= -1;
    return result;
}

void stable_test()
{
    vector<A> base_vec;
    for(size_t i = 0; i < 1000; ++i)
    {
      base_vec.push_back(A(rand() % 6, rand() % 11));
    }

    vector<A> test_vec = base_vec;
    vector<A> sorted_vec = base_vec;
    stable_sort(sorted_vec.begin(), sorted_vec.end());
    timsort(test_vec);
    BOOST_CHECK(test_vec == sorted_vec);
}

void random_test()
{
    // Prepare inputs
    vector<int> base_vec;
    size_t count = 100000;
    //Generating semirandom numbers
    for (size_t u = 0; u < count; ++u)
    {
        base_vec.push_back(rand_32());
    }
    vector<int> sorted_vec = base_vec;
    vector<int> test_vec = base_vec;
    std::sort(sorted_vec.begin(), sorted_vec.end());
    //Testing basic call
    timsort(test_vec.begin(), test_vec.end());
    BOOST_CHECK(test_vec == sorted_vec);
    //Boost.Range variant
    test_vec = base_vec;
    timsort(test_vec);
    BOOST_CHECK(test_vec == sorted_vec);
    //Functor
    test_vec = base_vec;
    timsort(test_vec.begin(), test_vec.end(), less<int>());
    BOOST_CHECK(test_vec == sorted_vec);
    //reverse order
    test_vec = base_vec;
    std::sort(test_vec.begin(), test_vec.end(), greater<int>());
    timsort(test_vec.begin(), test_vec.end());
    BOOST_CHECK(test_vec == base_vec);
}


template<typename T>
void corner_test(T val)
{
    vector<T> test_vec;
    boost::sort::timsort(test_vec.begin(), test_vec.end());
    BOOST_CHECK(test_vec.size() == 0);
    test_vec.push_back(val);
    boost::sort::timsort(test_vec.begin(), test_vec.end());
    BOOST_CHECK(test_vec.size() == 1);
    BOOST_CHECK(test_vec[0] == val);
}


// test main
int test_main( int, char*[] )
{
    srand(1);

    random_test();
    stable_test();

    //Corner tests
    corner_test<int>(42);
    corner_test<double>(42.0);
    corner_test<size_t>(0);
    corner_test<A>(A(42, 322));
    return 0;
}