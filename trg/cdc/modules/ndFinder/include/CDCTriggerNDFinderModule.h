/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include "framework/core/Module.h"
#include "framework/datastore/StoreArray.h"
#include "trg/cdc/dataobjects/CDCTriggerTrack.h"
#include "trg/cdc/dataobjects/CDCTriggerSegmentHit.h"
#include "trg/cdc/NDFinder.h"

namespace Belle2 {

  /** CDC Trigger NDFinder Module */
  class CDCTriggerNDFinderModule : public Module {
  public:
    /** Constructor */
    CDCTriggerNDFinderModule();
    /** Destructor */
    virtual ~CDCTriggerNDFinderModule();

    /** Module functions */
    virtual void initialize() override;
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

  private:

    /** Name for TS hits */
    std::string m_TrackSegmentHitsName;
    /** Name for NDFinder tracks */
    std::string m_NDFinderTracksName;
    /** Name for NDFinder Track To Hit Array */
    std::string m_NDFinderTrackToHitArrayName;

    /** StoreArray for TS hits */
    StoreArray<CDCTriggerSegmentHit> m_TrackSegmentHits;
    /** StoreArray for 2D finder tracks from unpacker */
    StoreArray<CDCTriggerTrack> m_NDFinderTracks;

    /** Instance of the 3D Track Finder */
    NDFinder m_NDFinder;
    /** Cluster pruning: Minimum number of hits related to a cluster
     * for the cluster to be considered as a track */
    int m_minhits;
    /** Cluster pruning: Minimum number of axial hits related to a cluster
     * for the cluster to be considered as a track */
    int m_minhits_axial;
    /** Clustering: Minimum weight of a cell in Hough space
     * for the cell to be considered as a cluster member */
    int m_minweight;
    /** Clustering: Minimum number of neighbor cells with minweight
     * for a cell to be considered a core cell */
    int m_minpts;
    /** Track estimation: Minimum weight of a cluster member cell
     * relative to the peak weight of the cluster
     * for the cell to enter in the weighted mean
     * track parameter value estimation */
    double m_thresh;
    /** Hit to cluster assignment:
     * Minimum relative weight contribution to the largest cluster */
    double m_minassign;
    /** Clustering: consider diagonal neighbors */
    bool m_diagonal;
    /**Clustering: minimum number of cells for a cluster */
    int m_mincells;
    /** Print Hough planes and verbose output */
    bool m_verbose;
    /** File name of the axial hit patterns */
    std::string m_axialFile;
    /** File name of the stereo hit patterns */
    std::string m_stereoFile;
  };
}
