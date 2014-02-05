/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCWIREHITTOPOLOGY_H
#define CDCWIREHITTOPOLOGY_H

#include <cdc/dataobjects/CDCHit.h>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/topology/CDCWire.h>
#include <tracking/cdcLocalTracking/topology/CDCWireLayer.h>
#include <tracking/cdcLocalTracking/topology/CDCWireSuperLayer.h>


#include <tracking/cdcLocalTracking/eventdata/entities/CDCWireHit.h>
#include <tracking/cdcLocalTracking/eventdata/entities/CDCRLWireHit.h>

#include <tracking/cdcLocalTracking/algorithms/SortableVector.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class representating the sense wire arrangement in the whole of the central drift chamber.
    class CDCWireHitTopology : public UsedTObject {

      /// Default constructor for ROOT compatibility.
      CDCWireHitTopology() {;}

      /// Empty deconstructor
      ~CDCWireHitTopology() {;}

      /// Fill the topology from the raw cdc hits
      void fill(const std::string& cdcHitsStoreArrayName = "");

      /// Clear content of the topology after the event is processed
      void clear();

      /// Getter for the oriented wire hit with the opposite orientation.
      const CDCRLWireHit& getReverseOf(const CDCRLWireHit& rlWireHit) const;

      /// Getter for the two oriented wire hits that are passed on the given wire hit
      std::pair<const CDCRLWireHit*, const CDCRLWireHit*> getRLWireHitPair(const CDCWireHit& wireHit) const;


    private:
      SortableVector<CDCWireHit> m_wireHits; ///< Memory for the wire hits to be stored
      SortableVector<CDCRLWireHit> m_rlWireHits; ///< Memory for the oriented wire hits to be stored

    private:
      /// ROOT Macro to make CDCWireTopology a ROOT class
      ClassDefInCDCLocalTracking(CDCWireHitTopology, 1);

    }; //class


  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCWIRETOPOLOGY
