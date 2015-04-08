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

#include <string>
#include <vector>

#include <framework/core/Module.h>
#include <tracking/modules/trackFinderCDC/TrackFinderCDCFromSegmentsModule.h>
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeProcessorImplementation.h>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCRecoSegment2D;
    class CDCTrack;
  }

  class SegmentQuadTreeModule : public TrackFinderCDCFromSegmentsModule {

    typedef TrackFindingCDC::QuadTreeProcessorSegments::QuadTree SegmentQuadTree;

  public:

    SegmentQuadTreeModule();

    void generate(std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments, std::vector<TrackFindingCDC::CDCTrack>& tracks);

  private:
    const double m_rMin = -0.15; /**< Minimum in r direction*/
    const double m_rMax = 0.15; /**< Maximum in r direction*/
    const int m_nbinsTheta = 8192;
    SegmentQuadTree m_quadTree;

    unsigned int m_param_level;
    unsigned int m_param_minimumItems;

    void quadTreeSearch(std::vector<TrackFindingCDC::CDCRecoSegment2D>& recoSegments, std::vector<TrackFindingCDC::CDCTrack>& tracks);

    void printDebugInformation();

    void printQuadTree(SegmentQuadTree* node);
  };
}
