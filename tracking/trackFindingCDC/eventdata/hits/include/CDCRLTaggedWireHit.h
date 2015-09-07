/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/InfoTypes.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCWireHit;

    /// A wire hit with an associated right left passage hypotheses that is freely setable.
    class CDCRLTaggedWireHit {

    public:
      /// Constructor from a wire hit and the right left passage hypotheses.
      CDCRLTaggedWireHit(const CDCWireHit* wireHit,
                         const ERightLeft rlInfo = ERightLeft::c_Unknown) :
        m_wireHit(wireHit),
        m_rlInfo(rlInfo)
      {;}

      /// Pointer access to the wire hit
      const Belle2::TrackFindingCDC::CDCWireHit* operator->() const
      { return m_wireHit; }

      /// Getter for the wire hits
      const CDCWireHit* getWireHit() const
      { return m_wireHit; }

      /// Getter for the right left passage hypothesis
      ERightLeft getRLInfo() const
      { return m_rlInfo; }

      /// Setter for the right left passage hypothesis
      void setRLInfo(const ERightLeft rlInfo)
      { m_rlInfo = rlInfo; }

    private:
      /// Reference to the wire hit
      const CDCWireHit* m_wireHit;

      /// Memory for the right left passage hypotheses assoziated with the hit.
      ERightLeft m_rlInfo = ERightLeft::c_Unknown;

    };
  } // end namespace TrackFindingCDC
} // end namespace Belle2
