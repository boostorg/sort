// Copyright Steven J. Ross 2014, Francisco Jose Tapia 2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <random>
#include <vector>

#include <boost/sort/common/merge_four.hpp>
#include <boost/sort/common/range.hpp>
#include <boost/sort/common/util/algorithm.hpp>
#include <boost/sort/sort.hpp>
#include <boost/test/included/test_exec_monitor.hpp>
#include <boost/test/test_tools.hpp>

namespace {

// Type with deleted operator& to ensure Boost.Sort algorithms do not use raw unary &.
struct DeletedAddress {
  int value;

  explicit DeletedAddress(int v = 0) : value(v) {}

  bool operator<(const DeletedAddress& other) const {
    return value < other.value;
  }
  bool operator==(const DeletedAddress& other) const {
    return value == other.value;
  }

  DeletedAddress* operator&() = delete;
  const DeletedAddress* operator&() const = delete;
};

// Functor for integer_sort with types overloading operator&.
struct IntRightShift {
  int operator()(const DeletedAddress& x, unsigned offset) const {
    return x.value >> offset;
  }
};

// Tests low-level memory utilities in common/util/algorithm.hpp
// (initialize and destroy) where raw & was previously used.
void TestCommonUtilAlgorithm() {
  const size_t kCount = 16;
  alignas(DeletedAddress) char raw_memory[sizeof(DeletedAddress) * kCount];
  DeletedAddress* ptr = reinterpret_cast<DeletedAddress*>(raw_memory);

  DeletedAddress init_val(42);
  boost::sort::common::util::initialize(ptr, ptr + kCount, init_val);
  for (size_t i = 0; i < kCount; ++i) {
    BOOST_CHECK_EQUAL(ptr[i].value, 42);
  }
  boost::sort::common::util::destroy(ptr, ptr + kCount);
}

// Tests uninit_full_merge4 in common/merge_four.hpp where construct_object
// previously used raw &(*it_dest++).
void TestUninitFullMerge4() {
  std::vector<DeletedAddress> in1;
  in1.push_back(DeletedAddress(1));
  in1.push_back(DeletedAddress(5));
  std::vector<DeletedAddress> in2;
  in2.push_back(DeletedAddress(2));
  in2.push_back(DeletedAddress(6));
  std::vector<DeletedAddress> in3;
  in3.push_back(DeletedAddress(3));
  in3.push_back(DeletedAddress(7));
  std::vector<DeletedAddress> in4;
  in4.push_back(DeletedAddress(4));
  in4.push_back(DeletedAddress(8));

  boost::sort::common::range<std::vector<DeletedAddress>::iterator> ranges[4] = {
      boost::sort::common::range<std::vector<DeletedAddress>::iterator>(in1.begin(), in1.end()),
      boost::sort::common::range<std::vector<DeletedAddress>::iterator>(in2.begin(), in2.end()),
      boost::sort::common::range<std::vector<DeletedAddress>::iterator>(in3.begin(), in3.end()),
      boost::sort::common::range<std::vector<DeletedAddress>::iterator>(in4.begin(), in4.end()),
  };

  const size_t kTotal = 8;
  alignas(DeletedAddress) char raw_memory[sizeof(DeletedAddress) * kTotal];
  DeletedAddress* out_ptr = reinterpret_cast<DeletedAddress*>(raw_memory);
  boost::sort::common::range<DeletedAddress*> dest(out_ptr, out_ptr + kTotal);

  auto res = boost::sort::common::uninit_full_merge4(
      dest, ranges, 4, std::less<DeletedAddress>());
  BOOST_CHECK_EQUAL(res.size(), kTotal);
  for (size_t i = 0; i < kTotal; ++i) {
    BOOST_CHECK_EQUAL(out_ptr[i].value, static_cast<int>(i + 1));
  }
  boost::sort::common::util::destroy(out_ptr, out_ptr + kTotal);
}

// Tests all sorting algorithms with DeletedAddress.
void TestSortAlgorithms() {
  const size_t kSmallSize = 250;
  std::mt19937 gen(12345);
  std::uniform_int_distribution<int> dist(0, 50000);

  std::vector<DeletedAddress> original(kSmallSize);
  for (size_t i = 0; i < kSmallSize; ++i) {
    original[i] = DeletedAddress(dist(gen));
  }

  // pdqsort
  {
    std::vector<DeletedAddress> v = original;
    boost::sort::pdqsort(v.begin(), v.end());
    BOOST_CHECK(std::is_sorted(v.begin(), v.end()));
  }

  // spinsort
  {
    std::vector<DeletedAddress> v = original;
    boost::sort::spinsort(v.begin(), v.end());
    BOOST_CHECK(std::is_sorted(v.begin(), v.end()));
  }

  // flat_stable_sort
  {
    std::vector<DeletedAddress> v = original;
    boost::sort::flat_stable_sort(v.begin(), v.end());
    BOOST_CHECK(std::is_sorted(v.begin(), v.end()));
  }

  // insert_sort
  {
    std::vector<DeletedAddress> v = original;
    boost::sort::insert_sort(v.begin(), v.end());
    BOOST_CHECK(std::is_sorted(v.begin(), v.end()));
  }

  // integer_sort (spreadsort)
  {
    std::vector<DeletedAddress> v = original;
    boost::sort::spreadsort::integer_sort(v.begin(), v.end(), IntRightShift());
    BOOST_CHECK(std::is_sorted(v.begin(), v.end()));
  }

  // Large datasets exercising parallel code paths in sample_sort,
  // parallel_stable_sort, and block_indirect_sort.
  const size_t kLargeSize = 70000;
  std::vector<DeletedAddress> large_original(kLargeSize);
  for (size_t i = 0; i < kLargeSize; ++i) {
    large_original[i] = DeletedAddress(dist(gen));
  }

  // sample_sort (parallel mode triggers above 65536 elements)
  {
    std::vector<DeletedAddress> v = large_original;
    boost::sort::sample_sort(v.begin(), v.end());
    BOOST_CHECK(std::is_sorted(v.begin(), v.end()));
  }

  // parallel_stable_sort (parallel mode triggers above 65536 elements)
  {
    std::vector<DeletedAddress> v = large_original;
    boost::sort::parallel_stable_sort(v.begin(), v.end());
    BOOST_CHECK(std::is_sorted(v.begin(), v.end()));
  }

  // block_indirect_sort (parallel mode triggers above 131072 elements for sizeof=4)
  const size_t kBlockSize = 140000;
  std::vector<DeletedAddress> block_original(kBlockSize);
  for (size_t i = 0; i < kBlockSize; ++i) {
    block_original[i] = DeletedAddress(dist(gen));
  }
  {
    std::vector<DeletedAddress> v = block_original;
    boost::sort::block_indirect_sort(v.begin(), v.end());
    BOOST_CHECK(std::is_sorted(v.begin(), v.end()));
  }
}

}  // namespace

int test_main(int, char*[]) {
  TestCommonUtilAlgorithm();
  TestUninitFullMerge4();
  TestSortAlgorithms();

  return 0;
}
