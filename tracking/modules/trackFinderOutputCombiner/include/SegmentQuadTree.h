/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCFromSegmentsModule.h>

#include <string>
#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCRecoSegment2D;
    class CDCTrack;
    class CDCRecoHit2D;
    class CDCTrajectory2D;
    class CDCTrajectory3D;
  }

  /** Experimental testing module for handling segments in a quad tree instead of hits. Not to be used in production. */
  class SegmentQuadTreeModule : public TrackFinderCDCFromSegmentsModule {

  public:

    /**
     * Constructor.
     */
    SegmentQuadTreeModule();

    /** Also initialize the debug output */
    void initialize();

    /**
     * Used the QuadTree to generate tracks from segments.
     */
    void generate(std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments, std::vector<TrackFindingCDC::CDCTrack>& tracks);

  private:
    const double m_rMin = -0.15; /**< Minimum in r direction*/
    const double m_rMax = 0.15; /**< Maximum in r direction*/
    const int m_nbinsTheta =
      8192; /**< Maximum in theta-direction (theta is actually an unsigned int here - we use the LookUp to get the correct angles) */

    unsigned int m_param_level; /**< Maximum Level for QuadTreeSearch. */
    unsigned int m_param_minimumItems; /**< Minimum number of hits in one QuadTreeCell. */
    bool m_param_debugOutput; /**< Create debug output or not. */

    /** Do the quad tree search */
    void quadTreeSearch(std::vector<TrackFindingCDC::CDCRecoSegment2D>& recoSegments, std::vector<TrackFindingCDC::CDCTrack>& tracks);

    /** Helper function to add a hit to a track */
    void addHitToTrack(TrackFindingCDC::CDCTrack& track, const TrackFindingCDC::CDCRecoHit2D& hit,
                       const TrackFindingCDC::CDCTrajectory3D& trajectory3D);
  };
}
