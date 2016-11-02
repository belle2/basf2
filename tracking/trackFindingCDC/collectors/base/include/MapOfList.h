/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <map>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * A helper class for a map of vectors with a convenience function for adding items.
     * Compare can be given for replacing the std::less as the default comparer function
     * in the map.
     * TODO: Better replace this by a eigen::matrix?
     */
    template<class FromItem, class ToItem, class Compare = std::less<FromItem>>
    class MapOfList : public std::map<FromItem, std::vector<ToItem>, Compare> {
    private:
      /// The super (parent) class.
      using Super = typename std::map<FromItem, std::vector<ToItem>, Compare>;
    public:
      /**
       * Add a new relation to the list. If the fromItem is already there,
       * add the newToItem to its list. If not, create a new entry with fromItem
       * and a list with the newToItem in it.
       */
      void emplaceOrAppend(FromItem fromItem, ToItem newToItem)
      {
        if (Super::find(fromItem) != Super::end()) {
          Super::operator [](fromItem).push_back(newToItem);
        } else {
          std::vector<ToItem> newToItemList;
          newToItemList.reserve(5);
          newToItemList.push_back(newToItem);
          Super::emplace(fromItem, newToItemList);
        }
      }
    };
  }
}
