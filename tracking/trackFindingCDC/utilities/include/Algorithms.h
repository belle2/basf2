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
    template<class Ts, class ACategoryFunction, class Category>
    Category common(const Ts& items, const ACategoryFunction& catFunc, const Category defaultCat)
    {
      auto it = std::begin(items);
      auto itEnd = std::end(items);
      return common(it, itEnd, catFunc, defaultCat);
    }

    /**
     *  Returns the common category value if all items in a range belong to the same category
     *  or the default value if the categories of the items differ.
     */
    template<class It, class ACategoryFunction, class ACategory>
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
     * Chunks a range of values into adjacent sub ranges that belong to the same category.
     */
    template<class It, class ACategoryFunction>
    std::vector<Range<It> > adjacent_groupby(It itBegin, It itEnd, const ACategoryFunction& catFunc)
    {
      std::vector<Range<It> > result;
      if (itBegin == itEnd) return result; // empty case

      It itFirstOfGroup = itBegin;
      auto catOfGroup = catFunc(*itBegin);

      for (It it = itBegin; it != itEnd; ++it) {
        auto cat = catFunc(*it);
        if (catOfGroup != catFunc(*it)) {
          result.emplace_back(itFirstOfGroup, it);
          itFirstOfGroup = it;
          catOfGroup = cat;
        }
      }
      result.emplace_back(itFirstOfGroup, itEnd);
      return result;
    }

    /** Makes adjacent pairs from an input range,
     *  invoking the map with two arguments and writes to the output iterator
     */
    template<class AInputIterator, class AOutputIterator, class ABinaryOperation>
    AOutputIterator transform_adjacent_pairs(AInputIterator first, AInputIterator last,
                                             AOutputIterator result, const ABinaryOperation& map)
    {
      if (first == last) return result;

      AInputIterator second = first;
      ++second;
      while (second != last) {
        *result = map(*first, *second);
        ++result;
        ++first;
        ++second;
      }
      return result;
    }

    /** Makes adjacent triples from an input range,
     *  invoking the map with three arguments and writes to the output iterator
     */
    template<class AInputIterator, class AOutputIterator, class ATrinaryOperation>
    AOutputIterator transform_adjacent_triples(AInputIterator first, AInputIterator last,
                                               AOutputIterator result, const ATrinaryOperation& map)
    {
      if (not(first != last)) return result;

      AInputIterator second{first};
      ++second;
      if (not(second != last)) return result;

      AInputIterator third{second};
      ++third;
      while (third != last) {
        *result = map(*first, *second, *third);
        ++result;
        ++first;
        ++second;
        ++third;
      }
      return result;
    }
  } // end namespace TrackFindingCDC
} // end namespace Belle2
