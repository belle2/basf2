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

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/Range.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/TDCCountTranslatorBase.h>

#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCSimpleSimulation;

    /// Class representating the wire hit arrangement in the current event in the whole of the central drift chamber.
    /** This class stores the hits in the CDC that have been unpacked from the raw CDCHits and geometrical information
     *  of the wire position an initial estimate of the drift length is attached.
     *  Also a pair of oriented wire hits (with an assoziate right left passage hypotheses) is stored in here.
     *  In this way track finders do not need to (re)create hits multiple times but simply refer to them.
     *
     *  For faster lookup the hits are sorted by their corresponding wire id and their reference drift radius
     *  (and their right left passage hypothese in case of
     *  So they can be retieved by their respective (super) layer as a range.
     */
    class CDCWireHitTopology : public TObject {

    public:
      /// Getter of the singletone instance
      static CDCWireHitTopology& getInstance();

      /// Create the StoreObject for the WireHitTopology.
      static void initialize();

      /** Fill the topology with preconstructed wire hits*/
      void fill(const std::vector<CDCWireHit>& wireHits);

      /// Getter for the wire hit that is based on the given CDCHit.
      const Belle2::TrackFindingCDC::CDCWireHit*
      getWireHit(const Belle2::CDCHit* ptrHit) const;

      /// Constant getter for the wire hits
      const std::vector<Belle2::TrackFindingCDC::CDCWireHit>& getWireHits() const
      { return m_wireHits; }

      /// Getter for the wire hits
      std::vector<Belle2::TrackFindingCDC::CDCWireHit>& getWireHits()
      { return m_wireHits; }

    private:
      /// Memory for the wire hits to be stored
      std::vector<CDCWireHit> m_wireHits; //!

      ClassDef(CDCWireHitTopology, 1);
    }; //class


  } // namespace TrackFindingCDC
} // namespace Belle2
