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
#include <tracking/trackFindingCDC/collectors/adders/AdderInterface.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /** Class to add relations between the matched items. */
    template <class ACollectorItem, class ACollectionItem>
    class RelationAdder : public AdderInterface<ACollectorItem, ACollectionItem> {
    private:
      /** Add a relation between the matched items. */
      void add(ACollectorItem& collectorItem, const ACollectionItem& collectionItem, Weight weight) override
      {
        collectorItem->addRelationTo(collectionItem, weight);
      }
    };
  }
}
