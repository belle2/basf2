/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <vector>
#include <algorithm>


#include <tracking/trackFindingCDC/topology/CDCWire.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCWireHit.h>

#include <tracking/trackFindingCDC/ca/SortableVector.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A generic vector used for different hit aggregates.
    /** details */

    template<class T>
    class CDCGenHitVector : public SortableVector<T> {

    public:
      /// Returns the common stereo type of all tracking entities.
      /** This checks if all tracking entities are located in the same superlayer and \n
       *  returns the stereo type of the later. Returns INVALID_STEREOTYPE if the superlayer \n
       *  is not shared among the tracking entities. */
      StereoType getStereoType() const
      { return isValidISuperLayer(getISuperLayer()) ? this->front()->getStereoType() : INVALID_STEREOTYPE; }

      /// Returns the common super layer id of all stored tracking entities
      /** This checks if all tracking entities are located in the same superlayer and \n
       *  returns the superlayer id of the later. Returns INVALID_ISUPERLAYER if the superlayer \n
       *  is not shared among the tracking entities. */
      ILayerType getISuperLayer() const
      {
        if (this->empty()) return INVALID_ISUPERLAYER;
        const T& firstItem = this->front();
        const ILayerType iSuperLayer = firstItem->getISuperLayer();
        auto sameSL = [iSuperLayer](const T & item) { return item->getISuperLayer() == iSuperLayer; };
        return  std::all_of(this->begin(), this->end(), sameSL) ? iSuperLayer : INVALID_ISUPERLAYER;
      }

    }; //class

  } // namespace TrackFindingCDC
} // namespace Belle2

