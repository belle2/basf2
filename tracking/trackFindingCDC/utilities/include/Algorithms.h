/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/utilities/Range.h>

#include <vector>
#include <iterator>
#include <algorithm>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Returns the common category value if all items in a range belong to the same category
     *  or the default value if the categories of the items differ.
     */
    template <class Ts, class ACategoryFunction, class ACategory>
    ACategory common(const Ts& items, const ACategoryFunction& catFunc, const ACategory defaultCat)
    {
      auto it = std::begin(items);
      auto itEnd = std::end(items);
      return common(it, itEnd, catFunc, defaultCat);
    }

    /**
     *  Returns the common category value if all items in a range belong to the same category
     *  or the default value if the categories of the items differ.
     */
    template <class It, class ACategoryFunction, class ACategory>
    ACategory common(It itBegin, It itEnd, const ACategoryFunction& catFunc, const ACategory defaultCat)
    {
      if (itBegin == itEnd) return defaultCat; // empty case
      const ACategory cat = catFunc(*itBegin);
      for (It it = itBegin; it != itEnd; ++it) {
        if (cat != catFunc(*it)) {
          return defaultCat;
        };
      }
      return cat;
    }

    /**
     *  Erase remove idiom in a more concise form
     */
    template <class Ts, class APredicate>
    void erase_remove_if(Ts& ts, const APredicate& predicate)
    {
      ts.erase(std::remove_if(std::begin(ts), std::end(ts), predicate), std::end(ts));
    }

    /**
     *  Erase unique idiom in a more concise form
     */
    template <class Ts>
    void erase_unique(Ts& ts)
    {
      ts.erase(std::unique(std::begin(ts), std::end(ts)), std::end(ts));
    }

    /**
     *  Erase unique idiom in a more concise form
     */
    template <class Ts, class AEqual>
    void erase_unique(Ts& ts, const AEqual& equal)
    {
      ts.erase(std::unique(std::begin(ts), std::end(ts), equal), std::end(ts));
    }

    /**
     *  Counts the number of repetitions for each unique value in a range. Unique values must be adjacent.
     */
    template <class It>
    std::vector<std::pair<It, int> > unique_count(It itBegin, It itEnd)
    {
      std::vector<std::pair<It, int> > result;
      if (itBegin == itEnd) return result;
      It it = itBegin;
      result.emplace_back(it, 1);
      ++it;
      for (; it != itEnd; ++it) {
        if (*it == *result.back().first) {
          ++result.back().second;
        } else {
          result.emplace_back(it, 1);
        }
      }
      return result;
    }

    /**
     *  Counts the number of repetitions for each unique value in a range. Unique values must be adjacent.
     */
    template <class It, class AEqual>
    std::vector<std::pair<It, int> > unique_count(It itBegin, It itEnd, const AEqual& equal)
    {
      std::vector<std::pair<It, int> > result;
      if (itBegin == itEnd) return result;
      It it = itBegin;
      result.emplace_back(it, 1);
      ++it;
      for (; it != itEnd; ++it) {
        if (equal(*it, *result.back().first)) {
          ++result.back().second;
        } else {
          result.emplace_back(it, 1);
        }
      }
      return result;
    }

    /**
     *  Counts the number of repetitions for each unique value in a range. Unique values must be adjacent.
     */
    template <class It>
    std::vector<Range<It> > unique_ranges(It itBegin, It itEnd)
    {
      std::vector<std::pair<It, It> > result;
      if (itBegin == itEnd) return result;
      It it1 = itBegin;
      It it2 = itBegin + 1;
      result.emplace_back(it1, it2);
      for (; it2 != itEnd; ++it1, ++it2) {
        if (not(*it1 == *it2)) {
          result.emplace_back(it2, it2);
        }
        ++result.back().second;
      }
      return result;
    }

    /**
     *  Counts the number of repetitions for each unique value in a range. Unique values must be adjacent.
     */
    template <class It, class AEqual>
    std::vector<Range<It> > unique_ranges(It itBegin, It itEnd, const AEqual& equal)
    {
      std::vector<Range<It> > result;
      if (itBegin == itEnd) return result;
      It it1 = itBegin;
      It it2 = itBegin + 1;
      result.emplace_back(it1, it2);
      for (; it2 != itEnd; ++it1, ++it2) {
        if (not equal(*it1, *it2)) {
          result.emplace_back(it2, it2);
        }
        ++result.back().second;
      }
      return result;
    }

    /**
     *  Chunks a range of values into adjacent sub ranges that belong to the same category.
     */
    template <class It, class ACategoryFunction>
    std::vector<Range<It>> adjacent_groupby(It itBegin, It itEnd, const ACategoryFunction& catFunc)
    {
      std::vector<Range<It>> result;
      if (itBegin == itEnd) return result; // empty case

      It itFirstOfGroup = itBegin;
      auto catOfGroup = catFunc(*itBegin);

      for (It it = itBegin; it != itEnd; ++it) {
        auto cat = catFunc(*it);
        if (catOfGroup != cat) {
          result.emplace_back(itFirstOfGroup, it);
          itFirstOfGroup = it;
          catOfGroup = cat;
        }
      }
      result.emplace_back(itFirstOfGroup, itEnd);
      return result;
    }

    /**
     *  Makes adjacent pairs from an input range,
     *  invoking the map with two arguments and writes to the output iterator
     */
    template <class AInputIterator, class AOutputIterator, class ABinaryOperation>
    AOutputIterator transform_adjacent_pairs(AInputIterator itBegin,
                                             AInputIterator itEnd,
                                             AOutputIterator result,
                                             const ABinaryOperation& map)
    {
      if (itBegin == itEnd) return result;

      AInputIterator second = itBegin;
      ++second;
      while (second != itEnd) {
        *result = map(*itBegin, *second);
        ++result;
        ++itBegin;
        ++second;
      }
      return result;
    }

    /**
     *  Makes adjacent triples from an input range,
     *  invoking the map with three arguments and writes to the output iterator
     */
    template <class AInputIterator, class AOutputIterator, class ATrinaryOperation>
    AOutputIterator transform_adjacent_triples(AInputIterator itBegin,
                                               AInputIterator itEnd,
                                               AOutputIterator result,
                                               const ATrinaryOperation& map)
    {
      if (not(itBegin != itEnd)) return result;

      AInputIterator second = itBegin;
      ++second;
      if (not(second != itEnd)) return result;

      AInputIterator third = second;
      ++third;
      while (third != itEnd) {
        *result = map(*itBegin, *second, *third);
        ++result;
        ++itBegin;
        ++second;
        ++third;
      }
      return result;
    }

    /**
    Return a container which contains only the elements of inputContainer where pred is true.
    Shortcut for std::copy_if function.
    */
    template <class Ts, class TCopyIfPredicate>
    Ts copy_if(Ts const& inputContainer, TCopyIfPredicate pred)
    {
      Ts outputContainer;

      // copy only if predicate is true
      std::copy_if(inputContainer.begin(), inputContainer.end(), std::back_inserter(outputContainer), pred);
      return outputContainer;
    }


    /**
     * Shortcut for applying the std::any_of function.
     */
    template <class Ts, class AUnaryPredicate>
    bool any(const Ts& ts, const AUnaryPredicate& comparator)
    {
      return std::any_of(std::begin(ts), std::end(ts), comparator);
    }

    /**
     * Shortcut for applying std::find(..) != end.
     */
    template <class Ts, class AItem>
    bool is_in(const AItem& item, const Ts& ts)
    {
      return std::find(std::begin(ts), std::end(ts), item) != std::end(ts);
    };

    /**
     * Convenience function to obtain pointers from a range of objects
     */
    template <class T, class Ts>
    std::vector<T*> as_pointers(Ts& ts)
    {
      using std::begin;
      using std::end;
      std::size_t size = end(ts) - begin(ts);
      std::vector<T*> result(size, nullptr);
      std::transform(begin(ts), end(ts), result.begin(), [](T & t) { return std::addressof<T>(t);});
      return result;
    }
  }
}
