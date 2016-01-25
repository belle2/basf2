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
    template<class FromItem, class ToItem>
    class MapOfList : public std::map<FromItem, std::vector<ToItem>> {
    private:
      typedef typename std::map<FromItem, std::vector<ToItem>> Super;
    public:
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
