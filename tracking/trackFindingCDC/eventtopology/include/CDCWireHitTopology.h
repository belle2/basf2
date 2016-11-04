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
#include <memory>

namespace Belle2 {
  class CDCHit;

  namespace TrackFindingCDC {
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
      void fill(std::shared_ptr<ConstVectorRange<CDCWireHit> > wireHits);

      /// Getter for the wire hit that is based on the given CDCHit.
      const Belle2::TrackFindingCDC::CDCWireHit* getWireHit(const Belle2::CDCHit* ptrHit) const;

      /// Constant getter for the wire hits
      const ConstVectorRange<CDCWireHit>& getWireHits() const
      {
        B2ASSERT("CDCWireHitTopology was not filled with hits. Have you executed the WireHitPreparer?", m_wireHits);
        return *m_wireHits;
      }

      /// Getter for the wire hits
      ConstVectorRange<CDCWireHit> getWireHits()
      {
        B2ASSERT("CDCWireHitTopology was not filled with hits. Have you executed the WireHitPreparer?", m_wireHits);
        return *m_wireHits;
      }

    private:
      /// Memory for the wire hits to be stored
      std::shared_ptr<ConstVectorRange<CDCWireHit> > m_wireHits {nullptr}; //!

      /// Macro turning CDCWireHitTopology into a root class
      ClassDef(CDCWireHitTopology, 1);

    }; // class

  } // namespace TrackFindingCDC
} // namespace Belle2
