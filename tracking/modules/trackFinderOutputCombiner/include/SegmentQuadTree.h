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
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeProcessorTemplate.h>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCRecoSegment2D;
  }

  class SegmentQuadTreeModule : public Module {

    typedef TrackFindingCDC::QuadTreeTemplate<int, float, TrackFindingCDC::QuadTreeItem<TrackFindingCDC::CDCRecoSegment2D>>
        SegmentQuadTree;

  public:

    /**
     * Constructor to set the module parameters.
     */
    SegmentQuadTreeModule();

    /**
     * Initialize the module. Create the StoreArray.
     */
    void initialize();

    /**
     * Empty begin run.
     */
    void beginRun() {};

    /**
     * In the event the hits are sorted.
     */
    void event();

    /**
     * Empty end run.
     */
    void endRun() {};

    /**
     * Empty terminate.
     */
    void terminate() {};

  private:
    std::string m_param_recoSegments;           /**< Name of the Store Array for the segments from the local track finder. */
    const double m_rMin = -0.15; /**< Minimum in r direction*/
    const double m_rMax = 0.15; /**< Maximum in r direction*/
    const int m_nbinsTheta = 8192;
    SegmentQuadTree m_quadTree;

    void quadTreeSearch(std::vector<TrackFindingCDC::CDCRecoSegment2D>& recoSegments);
  };
}
