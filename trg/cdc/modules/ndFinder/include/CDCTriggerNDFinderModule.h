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
#include "trg/cdc/dataobjects/CDCTrigger3DHTrack.h"
#include "trg/cdc/dataobjects/CDCTriggerSegmentHit.h"
#include "trg/cdc/NDFinder.h"

namespace Belle2 {

  // CDC Trigger NDFinder Module
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
    // Sets the CDC quadrant according to the phi prediction
    short getNDFinderQuadrant(const CDCTrigger3DHTrack& ndFinderTrack);
    // Calculates the 3DHough total momentum estimate
    double getNDFinderTotalMomentum(const CDCTrigger3DHTrack& ndFinderTrack);
    // Name for TS hits
    std::string m_trackSegmentHitsName;
    // Name for NDFinder tracks
    std::string m_ndFinderTracksName;
    // Name for NDFinder Track To Hit Array
    std::string m_ndFinderTrackToHitArrayName;

    // StoreArray for TS hits
    StoreArray<CDCTriggerSegmentHit> m_trackSegmentHits;
    // StoreArray for NDFinder tracks
    StoreArray<CDCTrigger3DHTrack> m_ndFinderTracks;

    // Instance of the 3DHough Track Finder (NDFinder/3DFinder)
    NDFinder m_NDFinder;
    // Minimum number of axial super layers for the peak to be considered as a track
    unsigned short m_minSuperAxial;
    // Minimum number of stereo super layers for the peak to be considered as a track
    unsigned short m_minSuperStereo;
    // Peak finding: Number of global maximum searches per Hough space section
    unsigned short m_iterations;
    // Peak finding: Max deletion in omega (number of cells in each direction from max)
    unsigned short m_omegaTrim;
    // Peak finding: Max deletion in phi (number of cells in each direction from max)
    unsigned short m_phiTrim;
    // Switch for saving the full Hough space
    bool m_storeHoughSpace;
    // File name of the axial hit patterns
    std::string m_axialFile;
    // File name of the stereo hit patterns
    std::string m_stereoFile;
  };
}
