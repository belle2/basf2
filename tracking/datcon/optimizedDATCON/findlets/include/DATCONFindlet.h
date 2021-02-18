/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Christian Wessel                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/datcon/optimizedDATCON/findlets/SpacePointLoaderAndPreparer.h>
#include <tracking/datcon/optimizedDATCON/findlets/FastInterceptFinder2D.h>
#include <tracking/datcon/optimizedDATCON/findlets/RawTrackCandCleaner.h>
// #include <tracking/datcon/optimizedDATCON/findlets/ToPXDExtrapolator.h>
// #include <tracking/datcon/optimizedDATCON/findlets/ROICalculator.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>

#include <string>
#include <vector>

#include <TH1D.h>
#include <TH2D.h>
#include <TFile.h>

namespace Belle2 {
  class ModuleParamList;
  class HitDataCache;

  /**
   * Findlet for performing the DATCON ROI calculation.
   */
  class DATCONFindlet : public TrackFindingCDC::Findlet<> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<>;

  public:
    /// Constructor for adding the subfindlets
    DATCONFindlet();

    /// Default desctructor
    ~DATCONFindlet();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Function to call all the sub-findlets
    void apply() override;

    /// Clear the object pools
    void beginEvent() override;

    /// Write ROOT file and terminate.
    /// Will be deleted when ROOT stuff is deleted.
    void terminate() override;

  private:

    /// Initialize ROOT histograms that are for debugging.
    /// Will be deleted when ROOT stuff is deleted.
    void initializeHists();
    /// Analyze track candidates.
    void analyseSPTCs();

    /// Findlets:
    /// Load SVDSpacePoints and prepare them for Hough-based tracking
    /// by calculating the conformal mapped x and y values of the 3D SpacePoint
    SpacePointLoaderAndPreparer m_spacePointLoaderAndPreparer;

    /// Hough Space intercept finder
    FastInterceptFinder2D m_interceptFinder;

    /// Raw track candidate cleaner
    RawTrackCandCleaner m_rawTCCleaner;

    // container to share data between findlets

    /// Vector containing the cached hit information
    std::vector<HitDataCache> m_hits;

    /// Vector containint raw track candidates
    std::vector<std::vector<HitDataCache*>> m_rawTrackCandidates;

    /// A track candidate is a vector of SpacePoint, and in each event multple track candidates
    /// will be created, which are stored in a vector themselves.
    std::vector<SpacePointTrackCand> m_trackCandidates;

    /// ROOT histograms for debugging. Will be deleted when optimization and debugging is done.
    /// ROOT file name
    TFile* m_rootFile;
    /// see histogram name for description
    TH1D* m_nMCParticlesPerEvent;
    /// see histogram name for description
    TH1D* m_nSVDSPsPerEvent;
    /// see histogram name for description
    TH1D* m_trackCandsPerEvent;
    /// see histogram name for description
    TH1D* m_DiffTrackCandsMCParticlesPerEvent;
    /// see histogram name for description
    TH1D* m_hitsPerTrackCand;

    /// see histogram name for description
    TH2D* m_nTrackCandsvsnMCParticles2D;
    /// see histogram name for description
    TH2D* m_nTrackCandsvsnSVDSpacePoints2D;
    /// see histogram name for description
    TH2D* m_nDiffTrackCandsMCParticlesvsnSVDSpacePoints2D;
    /// see histogram name for description
    TH2D* m_trackCands2D;

    /// StoreArrays needed for analysing and debugging during development
    StoreArray<MCParticle> m_storeMCParticles;
    /// StoreArrays needed for analysing and debugging during development
    StoreArray<SpacePoint> m_storeSVDSpacePoints;
  };
}
