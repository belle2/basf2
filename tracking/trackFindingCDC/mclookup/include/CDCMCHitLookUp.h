/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/mclookup/ITrackType.h>
#include <tracking/trackingUtilities/numerics/ERightLeft.h>
#include <tracking/trackingUtilities/numerics/Index.h>

#include <Math/Vector3D.h>
#include <Math/Vector2D.h>

namespace Belle2 {
  class MCParticle;
  class CDCSimHit;
  class CDCHit;

  namespace TrackFindingCDC {

    /// Interface class to the Monte Carlo information for individual hits
    /** This class provides a stable interface for the underlying implementation for look ups
     *  into the Monte Carlo information for the first stage of the algorithm.*/
    class CDCMCHitLookUp {

    public:
      /// Getter for the singletone instance
      static const CDCMCHitLookUp& getInstance();

      /// Default constructor, needs to be public for initialization in CDCMCManager
      CDCMCHitLookUp() = default;

      /// Singleton: Delete copy constructor and assignment operator
      CDCMCHitLookUp(CDCMCHitLookUp&) = delete;

      /// Operator =.
      CDCMCHitLookUp& operator=(const CDCMCHitLookUp&) = delete;

      /// Method for forwarding a request to fill the Monte Carlo lookup tables from Python
      static void fill();

    public:
      /// Getter for the CDCSimHit which is related to the CDCHit contained in the given wire hit
      static const Belle2::CDCSimHit* getSimHit(const CDCHit* ptrHit);

      /// Getter for the MCParticle which is related to the CDCHit contained in the given wire hit
      static const Belle2::MCParticle* getMCParticle(const CDCHit* ptrHit);


      /// Getter for the two dimensional reference position of the wire the given hit is located on - mainly for the python event display
      static const ROOT::Math::XYVector getRefPos2D(const CDCHit* ptrHit);

      /// Getter for the reference drift length in the two dimensional projection
      static float getRefDriftLength(const CDCHit* ptrHit);

      /// Getter for the three dimensional position of the primary ionisation for the hit.
      static const ROOT::Math::XYZVector getRecoPos3D(const CDCHit* ptrHit);

      /// Getter for the three dimensional position of the ionisation of the primary simulated hit for the hit.
      static const ROOT::Math::XYZVector getClosestPrimaryRecoPos3D(const CDCHit* ptrHit);

    public:
      /// Indicates if the hit was reassigned to a different mc particle because it was caused by a secondary.
      static bool isReassignedSecondary(const CDCHit* ptrHit);

      /// Getter for the closest simulated hit of a primary particle to the given hit - may return nullptr of no closest is found
      static const CDCSimHit* getClosestPrimarySimHit(const CDCHit* ptrHit);

      /// Returns the track id for the hit
      static ITrackType getMCTrackId(const CDCHit* ptrHit);

      /// Returns if this hit is considered background
      static bool isBackground(const CDCHit* ptrHit);

      /// Returns the position of the wire hit in the track along the travel direction
      static TrackingUtilities::Index getInTrackId(const CDCHit* ptrHit);

      /// Returns the id of the segment in the track.
      static TrackingUtilities::Index getInTrackSegmentId(const CDCHit* ptrHit);

      /// Returns the number of superlayers the track traversed until this hit.
      static TrackingUtilities::Index getNPassedSuperLayers(const CDCHit* ptrHit);

      /// Returns the number of loops the track traversed until this hit.
      static TrackingUtilities::Index getNLoops(const CDCHit* ptrHit);

      /// Returns the true right left passage information
      static TrackingUtilities::ERightLeft getRLInfo(const CDCHit* ptrHit);
    };
  }
}
