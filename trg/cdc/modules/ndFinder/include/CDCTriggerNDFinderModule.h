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
#include "trg/cdc/dataobjects/CDCTrigger3DFinderInfo.h"
#include "trg/cdc/dataobjects/CDCTriggerSegmentHit.h"
#include "trg/cdc/NDFinder.h"
#include "trg/cdc/Clusterizend.h"

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
    /** Name for the NDFinder Cluster Data */
    std::string m_NDFinderInfosName;

    /** StoreArray for TS hits */
    StoreArray<CDCTriggerSegmentHit> m_TrackSegmentHits;
    /** StoreArray for 2D finder tracks from unpacker */
    StoreArray<CDCTriggerTrack> m_NDFinderTracks;
    /** StoreArray for NDFinder Cluster Data */
    StoreArray<CDCTrigger3DFinderInfo> m_NDFinderInfos;

    /** Instance of the 3D Track Finder */
    NDFinder m_NDFinder;
    /** Cluster pruning: Minimum number of axial super layer hits related to a cluster
     * for the cluster to be considered as a track */
    int m_minSuperAxial;
    /** Cluster pruning: Minimum number of stereo super layer hits related to a cluster
     * for the cluster to be considered as a track */
    int m_minSuperStereo;
    /** Clustering: Minimum weight of a cell in Hough space
     * for the cell to be considered as a cluster member */
    int m_minWeight;
    /** Clustering: Minimum number of neighbor cells with minWeight
     * for a cell to be considered a core cell */
    int m_minPts;
    /** Track estimation: Minimum weight of a cluster member cell
     * relative to the peak weight of the cluster
     * for the cell to enter in the weighted mean
     * track parameter value estimation */
    double m_thresh;
    /** Clustering: consider diagonal neighbors */
    bool m_diagonal;
    /**Clustering: minimum number of cells for a cluster */
    int m_minCells;
    /** Clustering method: When true: dbscan, when false: fixed three
     * dimensional volume */
    bool m_dbscanning;
    /** Clustering with 3d volume: Cut on the total weight in this volume */
    int m_minTotalWeight;
    /** Clustering with 3d volume: Cut on the peak cell weight */
    int m_minPeakWeight;
    /** Clustering with 3d volume: Number of global maximum searches per Hough space */
    int m_iterations;
    /** Clustering with 3d volume: Max deletion in omega (number of cells in each direction from max */
    int m_omegaTrim;
    /** Clustering with 3d volume: Max deletion in phi (number of cells in each direction from max */
    int m_phiTrim;
    /** Clustering with 3d volume: Max deletion in theta (number of cells in each direction from max */
    int m_thetaTrim;
    /** Print Hough planes and verbose output */
    bool m_verbose;
    /** File name of the axial hit patterns */
    std::string m_axialFile;
    /** File name of the stereo hit patterns */
    std::string m_stereoFile;
  };
}
