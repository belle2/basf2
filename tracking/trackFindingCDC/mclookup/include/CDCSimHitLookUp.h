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

#include <tracking/trackFindingCDC/numerics/ERightLeft.h>

#include <tracking/trackFindingCDC/utilities/MayBePtr.h>

#include <map>
#include <vector>

namespace Belle2 {

  class CDCHit;
  class CDCSimHit;

  namespace TrackFindingCDC {
    class CDCMCMap;
    class CDCRecoHit3D;
    class CDCRecoHit2D;
    class CDCRLWireHit;
    class CDCWireHit;
    class Vector3D;

    /// Singletone class to gather local information about the hits.
    /** Because of the reassignment of secondary hits and the different definition of the right
     *  left passage in the CDCSimHit compared to the definition used in tracking finding, we provide this class
     *  to collect an manage localised information about each hit such as the local direction of travel and said
     *  left right passage information.
     */
    class CDCSimHitLookUp {

    public:
      /// Getter for the singletone instance
      static const CDCSimHitLookUp& getInstance();

      /// Default constructor - for cppcheck
      CDCSimHitLookUp() = default;

      /// Clear all information from the last event
      void clear();

      /// Gather the information about the right left passage using the CDCMCMap
      void fill(const CDCMCMap* ptrMCMap);

    private:
      /// Constructs the relation from reassigned secondary to a close by primary hit from the same MCParticle
      void fillPrimarySimHits();

      /// Helper function to find the closest primary hit for the given CDCSimHit from the same MCParticle - nullptr if no suitable hit can be found
      MayBePtr<const CDCSimHit> getClosestPrimarySimHit(const CDCSimHit* ptrSimHit) const;

    public:
      /// Look up and return the closest primary simulated hit for the given CDCHit - if no suitable hit can be found return the secondary
      const CDCSimHit* getClosestPrimarySimHit(const CDCHit* ptrHit) const;

      /// Calculate the local direction of flight. If the hit is secondary take the direction of flight from a close by primary - null vector if it cannot be assumed this way
      Vector3D getDirectionOfFlight(const CDCHit* ptrHit);

    private:
      /// Construct the look up relation for the right left passage information as used in track finding
      void fillRLInfo();

    public:
      /// Look up the Monte Carlo right left passage information for the given hit.
      ERightLeft getRLInfo(const CDCHit* ptrHit) const;

      /// Look up the position of the primary ionisation from related simulated hit.
      Vector3D getRecoPos3D(const CDCHit* ptrHit) const;

      /// Look up the drift length from the primary ionisation to the wire from related simulated hit.
      double getDriftLength(const CDCHit* ptrHit) const;

      /// Look up the position of the primary ionisation from the closest primary simulated hit.
      /// If no primary sim hit is available use the information from the secondary hit
      Vector3D getClosestPrimaryRecoPos3D(const CDCHit* ptrHit) const;

      /// Look up the drift length from the primary ionisation to the wire from related simulated hit.
      /// If no primary sim hit is available use the information from the secondary hit
      double getClosestPrimaryDriftLength(const CDCHit* ptrHit) const;

    public:
      /// Retrieve the wire hit the given CDCHit form the given wire hits
      const CDCWireHit* getWireHit(const CDCHit* ptrHit,
                                   const std::vector<CDCWireHit>& wireHits) const;

      /// Retrieve the wire hit including right left passage information for the given CDCHit form the given wire hits
      CDCRLWireHit getRLWireHit(const CDCHit* ptrHit,
                                const std::vector<CDCWireHit>& wireHits) const;

      /// Construct an CDCRecoHit3D from the (potential secondary) CDCSimHit information related to the CDCHit.
      CDCRecoHit3D getRecoHit3D(const CDCHit* ptrHit,
                                const std::vector<CDCWireHit>& wireHits) const;

      /// Construct an CDCRecoHit3D from the closest primary CDCSimHit information related to the CDCHit.
      CDCRecoHit3D getClosestPrimaryRecoHit3D(const CDCHit* ptrHit,
                                              const std::vector<CDCWireHit>& wireHits) const;

      /// Construct an CDCRecoHit2D from the (potential secondary) CDCSimHit information related to the CDCHit.
      CDCRecoHit2D getRecoHit2D(const CDCHit* ptrHit,
                                const std::vector<CDCWireHit>& wireHits) const;

      /// Construct an CDCRecoHit2D from the closest primary CDCSimHit information related to the CDCHit.
      CDCRecoHit2D getClosestPrimaryRecoHit2D(const CDCHit* ptrHit,
                                              const std::vector<CDCWireHit>& wireHits) const;

    private:
      /// Reference to the CDCMCMap to be used in this event
      const CDCMCMap* m_ptrMCMap;

      /// Memory for the look up relation of close primary CDCSimHits
      std::map<const CDCHit*, MayBePtr<const CDCSimHit> >  m_primarySimHits;

      /// Memory for the look up relation of the right left passage information as defined in tracking.
      std::map<const CDCHit*, ERightLeft> m_rightLeftInfos;

    };
  }
}
