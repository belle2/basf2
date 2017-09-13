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

#include <tracking/trackFindingCDC/mclookup/ITrackType.h>
#include <tracking/trackFindingCDC/numerics/ERightLeft.h>
#include <tracking/trackFindingCDC/numerics/Index.h>

namespace Belle2 {
  class MCParticle;
  class CDCSimHit;
  class CDCHit;

  namespace TrackFindingCDC {

    class Vector3D;
    class Vector2D;

    /// Interface class to the Monte Carlo information for individual hits
    /** This class provides a stable interface for the underlying implementation for look ups
     *  into the Monte Carlo informations for the first stage of the algorithm.*/
    class CDCMCHitLookUp {

    public:
      /// Getter for the singletone instance
      static const CDCMCHitLookUp& getInstance();

      /// Method for forwarding a request to fill the Monte Carlo lookup tables from Python
      void fill() const;

    public:
      /// Getter for the CDCSimHit which is related to the CDCHit contained in the given wire hit
      const Belle2::CDCSimHit* getSimHit(const CDCHit* ptrHit) const;

      /// Getter for the MCParticle which is related to the CDCHit contained in the given wire hit
      const Belle2::MCParticle* getMCParticle(const CDCHit* ptrHit) const;


      /// Getter for the two dimensional reference position of the wire the given hit is located on - mainly for the python event display
      const Vector2D getRefPos2D(const CDCHit* ptrHit) const;

      /// Getter for the reference drift length in the two dimensional projection
      float getRefDriftLength(const CDCHit* ptrHit) const;

      /// Getter for the three dimensional position of the primary ionisation for the hit.
      const Vector3D getRecoPos3D(const CDCHit* ptrHit) const;

      /// Getter for the three dimensional position of the ionisation of the primary simulated hit for the hit.
      const Vector3D getClosestPrimaryRecoPos3D(const CDCHit* ptrHit) const;

    public:
      /// Indicates if the hit was reassigned to a different mc particle because it was caused by a secondary.
      bool isReassignedSecondary(const CDCHit* ptrHit) const;

      /// Getter for the closest simulated hit of a primary particle to the given hit - may return nullptr of no closest is found
      const CDCSimHit* getClosestPrimarySimHit(const CDCHit* ptrHit) const;

      /// Returns the track id for the hit
      ITrackType getMCTrackId(const CDCHit* ptrHit) const;

      /// Returns if this hit is considered background
      bool isBackground(const CDCHit* ptrHit) const;

      /// Returns the position if the wire hit in the track along the travel direction
      Index getInTrackId(const CDCHit* ptrHit) const;

      /// Returns the id of the segment in the track.
      Index getInTrackSegmentId(const CDCHit* ptrHit) const;

      /// Returns the number of superlayers the track traversed until this hit.
      Index getNPassedSuperLayers(const CDCHit* ptrHit) const;

      /// Returns the number of loops the track traversed until this hit.
      Index getNLoops(const CDCHit* ptrHit) const;

      /// Returns the true right left passage information
      ERightLeft getRLInfo(const CDCHit* ptrHit) const;
    };
  }
}
