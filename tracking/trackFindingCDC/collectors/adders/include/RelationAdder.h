/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
