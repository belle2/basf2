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

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <Eigen/Dense>

#include <framework/core/Module.h>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCRecoSegment2D;
  }

  class SegmentQualityCheckModule : public Module {
  public:

    /**
     * Constructor to set the module parameters.
     */
    SegmentQualityCheckModule();

    /**
     * Empty destructor.
     */
    ~SegmentQualityCheckModule() {};

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
    bool isGoodSegment(const TrackFindingCDC::CDCRecoSegment2D& segment);

    double m_param_maximalMeanOfDriftLength;    /**< Maximal mean of the drift length for a segment to be used later. */
    std::string m_param_badTrackCands;          /**< Name of the Store Array for the bad segments for testing. */
    std::string m_param_recoSegments;           /**< Name of the Store Array for the segments from the local track finder. */
  };
}
