/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <string>
#include "framework/core/Module.h"
#include "framework/datastore/StoreArray.h"
#include "trg/cdc/dataobjects/CDCTriggerTrack.h"
#include "trg/cdc/dataobjects/CDCTrigger3DFinderInfo.h"
#include "trg/cdc/dataobjects/CDCTriggerSegmentHit.h"
#include "trg/cdc/NDFinder.h"
#include "trg/cdc/Clusterizend.h"

namespace Belle2 {

  /// CDC Trigger NDFinder Module
  class CDCTriggerNDFinderModule : public Module {
  public:
    // Constructor
    CDCTriggerNDFinderModule();
    // Destructor
    virtual ~CDCTriggerNDFinderModule();

    // Module functions
    virtual void initialize() override;
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

  private:
    /// Name for TS hits
    std::string m_trackSegmentHitsName;
    /// Name for NDFinder tracks
    std::string m_ndFinderTracksName;
    /// Name for NDFinder Track To Hit Array
    std::string m_ndFinderTrackToHitArrayName;
    /// Name for the NDFinder Cluster Data
    std::string m_ndFinderInfosName;

    /// StoreArray for TS hits
    StoreArray<CDCTriggerSegmentHit> m_trackSegmentHits;
    /// StoreArray for 2D finder tracks from unpacker
    StoreArray<CDCTriggerTrack> m_ndFinderTracks;
    /// StoreArray for NDFinder Cluster Data
    StoreArray<CDCTrigger3DFinderInfo> m_ndFinderInfos;

    /// Instance of the 3D Track Finder
    NDFinder m_NDFinder;
    /// Minimum number of axial super layers for the cluster to be considered as a track
    unsigned short m_minSuperAxial;
    /// Minimum number of stereo super layers for the cluster to be considered as a track
    unsigned short m_minSuperStereo;
    /// Clustering with 3d volume: Number of global maximum searches per Hough space quadrant
    unsigned short m_iterations;
    /// Clustering with 3d volume: Max deletion in omega (number of cells in each direction from max)
    unsigned short m_omegaTrim;
    /// Clustering with 3d volume: Max deletion in phi (number of cells in each direction from max)
    unsigned short m_phiTrim;
    /// Switch for writing the full Hough space and the cluster information to the 3DFinderInfo class
    bool m_storeAdditionalReadout;
    /// File name of the axial hit patterns
    std::string m_axialFile;
    /// File name of the stereo hit patterns
    std::string m_stereoFile;
  };
}
