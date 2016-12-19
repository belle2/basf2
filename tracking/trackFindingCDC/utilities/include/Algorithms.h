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

#include <iterator>
#include <vector>
#include <tracking/trackFindingCDC/utilities/Range.h>

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
      auto itRemoved = std::remove_if(std::begin(ts), std::end(ts), predicate);
      ts.erase(itRemoved, std::end(ts));
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
  }
}
