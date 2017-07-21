/*
  Copyright (c) Fuji, Goro (gfx) <gfuji@cpan.org>, 2011
  Copyright (c) Alexander Zaitsev <zamazan4ik@gmail.com>, 2016
  Distributed under the Boost Software License, Version 1.0. (See
  accompanying file LICENSE_1_0.txt or copy at
  http://www.boost.org/LICENSE_1_0.txt)
  See http://www.boost.org/ for latest version.

  Ported from:
    https://github.com/gfx/cpp-TimSort/blob/master/timsort.hpp
*/

#ifndef BOOST_SORT_TIMSORT_HPP
#define BOOST_SORT_TIMSORT_HPP

#include <vector>
#include <iterator>
#include <algorithm>
#include <utility>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>

#include <boost/move/move.hpp>

namespace boost { namespace sort
{

// ---------------------------------------
// Declaration
// ---------------------------------------

/**
 * Same as std::stable_sort(first, last).
 */
template<typename RandomAccessIterator>
void timsort(const RandomAccessIterator first, const RandomAccessIterator last);

/**
 * Same as std::stable_sort(first, last, compare).
 */
template<typename RandomAccessIterator, typename Compare>
void timsort(const RandomAccessIterator first, const RandomAccessIterator last, Compare compare);


template<typename Value, typename Compare>
class Comparator
{
public:
    typedef Value value_type;
    typedef Compare func_type;

    Comparator(Compare f) : less_(f)
    {
    }

    Comparator(const Comparator<value_type, func_type> &other) : less_(other.less_)
    {
    }

    bool lt(value_type x, value_type y)
    {
        return less_(x, y);
    }

    bool le(value_type x, value_type y)
    {
        return less_(x, y) || !less_(y, x);
    }

    bool gt(value_type x, value_type y)
    {
        return !less_(x, y) && less_(y, x);
    }

    bool ge(value_type x, value_type y)
    {
        return !less_(x, y);
    }

    func_type &less_function()
    {
        return less_;
    }

private:
    func_type less_;
};


//Implementation
template<typename RandomAccessIterator, typename Compare>
class TimSort
{
    typedef RandomAccessIterator iter_t;
    typedef typename std::iterator_traits<iter_t>::value_type value_t;
    typedef typename std::iterator_traits<iter_t>::reference ref_t;
    typedef typename std::iterator_traits<iter_t>::difference_type diff_t;
    typedef Comparator<const value_t &, Compare> compare_t;

    static const int MIN_MERGE = 32;

    compare_t comp_;

    static const int MIN_GALLOP = 7;

    int minGallop_; // default to MIN_GALLOP

    std::vector<value_t> tmp_; // temp storage for merges
    typedef typename std::vector<value_t>::iterator tmp_iter_t;

    struct run
    {
        iter_t base;
        diff_t len;

        run(const iter_t b, const diff_t l) : base(b), len(l)
        {
        }
    };

    std::vector<run> pending_;

    static void sort(const iter_t lo, const iter_t hi, compare_t c)
    {
        diff_t nRemaining = (hi - lo);
        if (nRemaining < 2)
        {
            return; // nothing to do
        }

        if (nRemaining < MIN_MERGE)
        {
            const diff_t initRunLen = countRunAndMakeAscending(lo, hi, c);
            binarySort(lo, hi, lo + initRunLen, c);
            return;
        }

        TimSort ts(c);
        const diff_t minRun = minRunLength(nRemaining);
        iter_t cur = lo;
        do
        {
            diff_t runLen = countRunAndMakeAscending(cur, hi, c);

            if (runLen < minRun)
            {
                const diff_t force = std::min(nRemaining, minRun);
                binarySort(cur, cur + force, cur + runLen, c);
                runLen = force;
            }

            ts.pushRun(cur, runLen);
            ts.mergeCollapse();

            cur += runLen;
            nRemaining -= runLen;
        }
        while (nRemaining != 0);

        ts.mergeForceCollapse();
    } // sort()

    static void binarySort(const iter_t lo, const iter_t hi, iter_t start, compare_t compare)
    {
        if (start == lo)
        {
            ++start;
        }
        for (; start < hi; ++start)
        {
            /*const*/ value_t pivot = boost::move(*start);

            const iter_t pos = std::upper_bound(lo, start, pivot, compare.less_function());
            for (iter_t p = start; p > pos; --p)
            {
                *p = boost::move(*(p - 1));
            }
            *pos = boost::move(pivot);
        }
    }

    static diff_t countRunAndMakeAscending(const iter_t lo, const iter_t hi, compare_t compare)
    {
        iter_t runHi = lo + 1;
        if (runHi == hi)
        {
            return 1;
        }

        if (compare.lt(*(runHi++), *lo))
        { // descending
            while (runHi < hi && compare.lt(*runHi, *(runHi - 1)))
            {
                ++runHi;
            }
            std::reverse(lo, runHi);
        }
        else
        { // ascending
            while (runHi < hi && compare.ge(*runHi, *(runHi - 1)))
            {
                ++runHi;
            }
        }

        return runHi - lo;
    }

    static diff_t minRunLength(diff_t n)
    {
        diff_t r = 0;
        while (n >= MIN_MERGE)
        {
            r |= (n & 1);
            n >>= 1;
        }
        return n + r;
    }

    TimSort(compare_t c) : comp_(c), minGallop_(MIN_GALLOP)
    {
    }

    void pushRun(const iter_t runBase, const diff_t runLen)
    {
        pending_.push_back(run(runBase, runLen));
    }

    void mergeCollapse()
    {
        while (pending_.size() > 1)
        {
            diff_t n = pending_.size() - 2;

            if ((n > 0 && pending_[n - 1].len <= pending_[n].len + pending_[n + 1].len) ||
                (n > 1 && pending_[n - 2].len <= pending_[n - 1].len + pending_[n].len))
            {
                if (pending_[n - 1].len < pending_[n + 1].len)
                {
                    --n;
                }
                mergeAt(n);
            }
            else if (pending_[n].len <= pending_[n + 1].len)
            {
                mergeAt(n);
            }
            else
            {
                break;
            }
        }
    }

    void mergeForceCollapse()
    {
        while (pending_.size() > 1)
        {
            diff_t n = pending_.size() - 2;

            if (n > 0 && pending_[n - 1].len < pending_[n + 1].len)
            {
                --n;
            }
            mergeAt(n);
        }
    }

    void mergeAt(const diff_t i)
    {
        const diff_t stackSize = pending_.size();

        iter_t base1 = pending_[i].base;
        diff_t len1 = pending_[i].len;
        iter_t base2 = pending_[i + 1].base;
        diff_t len2 = pending_[i + 1].len;

        pending_[i].len = len1 + len2;

        if (i == stackSize - 3)
        {
            pending_[i + 1] = pending_[i + 2];
        }

        pending_.pop_back();

        const diff_t k = gallopRight(*base2, base1, len1, 0);

        base1 += k;
        len1 -= k;

        if (len1 == 0)
        {
            return;
        }

        len2 = gallopLeft(*(base1 + (len1 - 1)), base2, len2, len2 - 1);
        if (len2 == 0)
        {
            return;
        }

        if (len1 <= len2)
        {
            mergeLo(base1, len1, base2, len2);
        }
        else
        {
            mergeHi(base1, len1, base2, len2);
        }
    }

    template<typename Iter>
    diff_t gallopLeft(ref_t key, const Iter base, const diff_t len, const diff_t hint)
    {
        diff_t lastOfs = 0;
        diff_t ofs = 1;

        if (comp_.gt(key, *(base + hint)))
        {
            const diff_t maxOfs = len - hint;
            while (ofs < maxOfs && comp_.gt(key, *(base + (hint + ofs))))
            {
                lastOfs = ofs;
                ofs = (ofs << 1) + 1;

                if (ofs <= 0)
                { // int overflow
                    ofs = maxOfs;
                }
            }
            if (ofs > maxOfs)
            {
                ofs = maxOfs;
            }

            lastOfs += hint;
            ofs += hint;
        }
        else
        {
            const diff_t maxOfs = hint + 1;
            while (ofs < maxOfs && comp_.le(key, *(base + (hint - ofs))))
            {
                lastOfs = ofs;
                ofs = (ofs << 1) + 1;

                if (ofs <= 0)
                {
                    ofs = maxOfs;
                }
            }
            if (ofs > maxOfs)
            {
                ofs = maxOfs;
            }

            const diff_t tmp = lastOfs;
            lastOfs = hint - ofs;
            ofs = hint - tmp;
        }

        return std::lower_bound(base + (lastOfs + 1), base + ofs, key, comp_.less_function()) - base;
    }

    template<typename Iter>
    diff_t gallopRight(ref_t key, const Iter base, const diff_t len, const diff_t hint)
    {
        diff_t ofs = 1;
        diff_t lastOfs = 0;

        if (comp_.lt(key, *(base + hint)))
        {
            const diff_t maxOfs = hint + 1;
            while (ofs < maxOfs && comp_.lt(key, *(base + (hint - ofs))))
            {
                lastOfs = ofs;
                ofs = (ofs << 1) + 1;

                if (ofs <= 0)
                {
                    ofs = maxOfs;
                }
            }
            if (ofs > maxOfs)
            {
                ofs = maxOfs;
            }

            const diff_t tmp = lastOfs;
            lastOfs = hint - ofs;
            ofs = hint - tmp;
        }
        else
        {
            const diff_t maxOfs = len - hint;
            while (ofs < maxOfs && comp_.ge(key, *(base + (hint + ofs))))
            {
                lastOfs = ofs;
                ofs = (ofs << 1) + 1;

                if (ofs <= 0)
                { // int overflow
                    ofs = maxOfs;
                }
            }
            if (ofs > maxOfs)
            {
                ofs = maxOfs;
            }

            lastOfs += hint;
            ofs += hint;
        }

        return std::upper_bound(base + (lastOfs + 1), base + ofs, key, comp_.less_function()) - base;
    }

    void mergeLo(const iter_t base1, diff_t len1, const iter_t base2, diff_t len2)
    {
        copy_to_tmp(base1, len1);

        tmp_iter_t cursor1 = tmp_.begin();
        iter_t cursor2 = base2;
        iter_t dest = base1;

        *(dest++) = boost::move(*(cursor2++));
        if (--len2 == 0)
        {
            boost::move(cursor1, cursor1 + len1, dest);
            return;
        }
        if (len1 == 1)
        {
            boost::move(cursor2, cursor2 + len2, dest);
            *(dest + len2) = boost::move(*cursor1);
            return;
        }

        int minGallop(minGallop_);

        // outer:
        while (true)
        {
            diff_t count1 = 0;
            diff_t count2 = 0;

            bool break_outer = false;
            do
            {
                if (comp_.lt(*cursor2, *cursor1))
                {
                    *(dest++) = boost::move(*(cursor2++));
                    ++count2;
                    count1 = 0;
                    if (--len2 == 0)
                    {
                        break_outer = true;
                        break;
                    }
                }
                else
                {
                    *(dest++) = boost::move(*(cursor1++));
                    ++count1;
                    count2 = 0;
                    if (--len1 == 1)
                    {
                        break_outer = true;
                        break;
                    }
                }
            }
            while ((count1 | count2) < minGallop);
            if (break_outer)
            {
                break;
            }

            do
            {
                count1 = gallopRight(*cursor2, cursor1, len1, 0);
                if (count1 != 0)
                {
                    boost::move_backward(cursor1, cursor1 + count1, dest + count1);
                    dest += count1;
                    cursor1 += count1;
                    len1 -= count1;

                    if (len1 <= 1)
                    {
                        break_outer = true;
                        break;
                    }
                }
                *(dest++) = boost::move(*(cursor2++));
                if (--len2 == 0)
                {
                    break_outer = true;
                    break;
                }

                count2 = gallopLeft(*cursor1, cursor2, len2, 0);
                if (count2 != 0)
                {
                    boost::move(cursor2, cursor2 + count2, dest);
                    dest += count2;
                    cursor2 += count2;
                    len2 -= count2;
                    if (len2 == 0)
                    {
                        break_outer = true;
                        break;
                    }
                }
                *(dest++) = boost::move(*(cursor1++));
                if (--len1 == 1)
                {
                    break_outer = true;
                    break;
                }

                --minGallop;
            }
            while ((count1 >= MIN_GALLOP) | (count2 >= MIN_GALLOP));
            if (break_outer)
            {
                break;
            }

            if (minGallop < 0)
            {
                minGallop = 0;
            }
            minGallop += 2;
        } // end of "outer" loop

        minGallop_ = std::min(minGallop, 1);

        if (len1 == 1)
        {
            boost::move(cursor2, cursor2 + len2, dest);
            *(dest + len2) = boost::move(*cursor1);
        }
        else
        {
            boost::move(cursor1, cursor1 + len1, dest);
        }
    }

    void mergeHi(const iter_t base1, diff_t len1, const iter_t base2, diff_t len2)
    {
        copy_to_tmp(base2, len2);

        iter_t cursor1 = base1 + (len1 - 1);
        tmp_iter_t cursor2 = tmp_.begin() + (len2 - 1);
        iter_t dest = base2 + (len2 - 1);

        *(dest--) = boost::move(*(cursor1--));
        if (--len1 == 0)
        {
            boost::move(tmp_.begin(), tmp_.begin() + len2, dest - (len2 - 1));
            return;
        }
        if (len2 == 1)
        {
            dest -= len1;
            cursor1 -= len1;
            boost::move_backward(cursor1 + 1, cursor1 + (1 + len1), dest + (1 + len1));
            *dest = boost::move(*cursor2);
            return;
        }

        int minGallop(minGallop_);

        // outer:
        while (true)
        {
            diff_t count2 = 0;
            diff_t count1 = 0;

            bool break_outer = false;
            do
            {
                if (comp_.lt(*cursor2, *cursor1))
                {
                    *(dest--) = boost::move(*(cursor1--));
                    ++count1;
                    count2 = 0;
                    if (--len1 == 0)
                    {
                        break_outer = true;
                        break;
                    }
                }
                else
                {
                    *(dest--) = boost::move(*(cursor2--));
                    ++count2;
                    count1 = 0;
                    if (--len2 == 1)
                    {
                        break_outer = true;
                        break;
                    }
                }
            }
            while ((count1 | count2) < minGallop);
            if (break_outer)
            {
                break;
            }

            do
            {
                count1 = len1 - gallopRight(*cursor2, base1, len1, len1 - 1);
                if (count1 != 0)
                {
                    dest -= count1;
                    cursor1 -= count1;
                    len1 -= count1;
                    boost::move_backward(cursor1 + 1, cursor1 + (1 + count1), dest + (1 + count1));

                    if (len1 == 0)
                    {
                        break_outer = true;
                        break;
                    }
                }
                *(dest--) = boost::move(*(cursor2--));
                if (--len2 == 1)
                {
                    break_outer = true;
                    break;
                }

                count2 = len2 - gallopLeft(*cursor1, tmp_.begin(), len2, len2 - 1);
                if (count2 != 0)
                {
                    dest -= count2;
                    cursor2 -= count2;
                    len2 -= count2;
                    boost::move(cursor2 + 1, cursor2 + (1 + count2), dest + 1);
                    if (len2 <= 1)
                    {
                        break_outer = true;
                        break;
                    }
                }
                *(dest--) = boost::move(*(cursor1--));
                if (--len1 == 0)
                {
                    break_outer = true;
                    break;
                }

                minGallop--;
            }
            while ((count1 >= MIN_GALLOP) | (count2 >= MIN_GALLOP));
            if (break_outer)
            {
                break;
            }

            if (minGallop < 0)
            {
                minGallop = 0;
            }
            minGallop += 2;
        } // end of "outer" loop

        minGallop_ = std::min(minGallop, 1);

        if (len2 == 1)
        {
            dest -= len1;
            boost::move_backward(cursor1 + (1 - len1), cursor1 + 1, dest + (1 + len1));
            *dest = boost::move(*cursor2);
        }
        else
        {
            boost::move(tmp_.begin(), tmp_.begin() + len2, dest - (len2 - 1));
        }
    }

    void copy_to_tmp(const iter_t begin, const diff_t len)
    {
        tmp_.clear();
        tmp_.reserve(len);
        boost::move(begin, begin + len, std::back_inserter(tmp_));
    }

    // the only interface is the friend timsort() function
    template<typename IterT, typename LessT>
    friend void timsort(IterT first, IterT last, LessT c);
};


/*! \brief Timsort algorithm using random access iterators.

  \details Timsort was designed to take advantage of partial orderings that already exist in most real-world data.
  Timsort operates by finding runs, subsequences of at least two elements that are either non-descending
  (each element is equal to or greater than its predecessor) or strictly descending
  (each element is lower than its predecessor). If it is descending, it must be strictly descending,
  since descending runs are later reversed by a simple swap of elements from both ends converging in the middle.
  After obtaining such a run in the given array, Timsort processes it, and then searches for the next run.

   \param[in] first Iterator pointer to first element.
   \param[in] last Iterator pointing to one beyond the end of data.

   \pre [@c first, @c last) is a valid range.
   \pre @c RandomAccessIter @c value_type is mutable.
   \pre @c RandomAccessIter @c value_type is <a href="http://en.cppreference.com/w/cpp/concept/LessThanComparable">LessThanComparable</a>
   \post The elements in the range [@c first, @c last) are sorted in ascending order.

   \return @c void.

   \throws std::exception Propagates exceptions if any of the element comparisons, the element swaps (or moves),
   functors, or any operations on iterators throw.

   \warning Throwing an exception may cause data loss.
   \warning Invalid arguments cause undefined behaviour.

   \remark O(N*log(N)) </em>operations worst-case, where:
   \remark  *  N is @c last - @c first,

*/

template<typename RandomAccessIterator>
void timsort(const RandomAccessIterator first, const RandomAccessIterator last)
{
    typedef typename std::iterator_traits<RandomAccessIterator>::value_type value_type;
    timsort(first, last, std::less<value_type>());
}

/*! \brief Timsort algorithm using random access iterators and a comparison functor.

  \details Timsort was designed to take advantage of partial orderings that already exist in most real-world data.
  Timsort operates by finding runs, subsequences of at least two elements that are either non-descending
  (each element is equal to or greater than its predecessor) or strictly descending
  (each element is lower than its predecessor). If it is descending, it must be strictly descending,
  since descending runs are later reversed by a simple swap of elements from both ends converging in the middle.
  After obtaining such a run in the given array, Timsort processes it, and then searches for the next run.

   \param[in] first Iterator pointer to first element.
   \param[in] last Iterator pointing to one beyond the end of data.
   \param[in] compare A binary functor that returns whether the first element passed to it should go before the second in order.

   \pre [@c first, @c last) is a valid range.
   \pre @c RandomAccessIter @c value_type is mutable.
   \pre @c RandomAccessIter @c value_type is <a href="http://en.cppreference.com/w/cpp/concept/LessThanComparable">LessThanComparable</a>
   \post The elements in the range [@c first, @c last) are sorted in ascending order.

   \return @c void.

   \throws std::exception Propagates exceptions if any of the element comparisons, the element swaps (or moves),
   functors, or any operations on iterators throw.

   \warning Throwing an exception may cause data loss.
   \warning Invalid arguments cause undefined behaviour.

   \remark O(N*log(N)) </em>operations worst-case, where:
   \remark  *  N is @c last - @c first,

*/

template<typename RandomAccessIterator, typename Compare>
void timsort(const RandomAccessIterator first, const RandomAccessIterator last, Compare compare)
{
    TimSort<RandomAccessIterator, Compare>::sort(first, last, compare);
}

/*! \brief Timsort algorithm using range.

  \details Timsort was designed to take advantage of partial orderings that already exist in most real-world data.
  Timsort operates by finding runs, subsequences of at least two elements that are either non-descending
  (each element is equal to or greater than its predecessor) or strictly descending
  (each element is lower than its predecessor). If it is descending, it must be strictly descending,
  since descending runs are later reversed by a simple swap of elements from both ends converging in the middle.
  After obtaining such a run in the given array, Timsort processes it, and then searches for the next run.

   \param[in] range Range [first, last) for sorting.

   \pre [@c first, @c last) is a valid range.
   \pre @c RandomAccessIter @c value_type is mutable.
   \pre @c RandomAccessIter @c value_type is <a href="http://en.cppreference.com/w/cpp/concept/LessThanComparable">LessThanComparable</a>
   \post The elements in the range [@c first, @c last) are sorted in ascending order.

   \return @c void.

   \throws std::exception Propagates exceptions if any of the element comparisons, the element swaps (or moves),
   functors, or any operations on iterators throw.

   \warning Throwing an exception may cause data loss.
   \warning Invalid arguments cause undefined behaviour.

   \remark O(N*log(N)) </em>operations worst-case, where:
   \remark  *  N is @c last - @c first,

*/

template<typename Range>
void timsort(Range& range)
{
    timsort(boost::begin(range), boost::end(range));
}

/*! \brief Timsort algorithm using range and a comparison functor.

  \details Timsort was designed to take advantage of partial orderings that already exist in most real-world data.
  Timsort operates by finding runs, subsequences of at least two elements that are either non-descending
  (each element is equal to or greater than its predecessor) or strictly descending
  (each element is lower than its predecessor). If it is descending, it must be strictly descending,
  since descending runs are later reversed by a simple swap of elements from both ends converging in the middle.
  After obtaining such a run in the given array, Timsort processes it, and then searches for the next run.

   \param[in] range Range [first, last) for sorting.
   \param[in] compare A binary functor that returns whether the first element passed to it should go before the second in order.

   \pre [@c first, @c last) is a valid range.
   \pre @c RandomAccessIter @c value_type is mutable.
   \pre @c RandomAccessIter @c value_type is <a href="http://en.cppreference.com/w/cpp/concept/LessThanComparable">LessThanComparable</a>
   \post The elements in the range [@c first, @c last) are sorted in ascending order.

   \return @c void.

   \throws std::exception Propagates exceptions if any of the element comparisons, the element swaps (or moves),
   functors, or any operations on iterators throw.

   \warning Throwing an exception may cause data loss.
   \warning Invalid arguments cause undefined behaviour.

   \remark O(N*log(N)) </em>operations worst-case, where:
   \remark  *  N is @c last - @c first,

*/

template<typename Range, typename Compare>
void timsort(Range& range, Compare compare)
{
    timsort(boost::begin(range), boost::end(range), compare);
}

}
}

#endif // BOOST_SORT_TIMSORT_HPP