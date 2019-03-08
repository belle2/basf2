/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bianca Scavino                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EFFPLOTSMODULE_H_
#define EFFPLOTSMODULE_H_

#include <framework/core/Module.h>
#include <tracking/modules/trackingPerformanceEvaluation/PerformanceEvaluationBaseClass.h>

#include <TTree.h>
#include <TFile.h>
#include <TList.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/MCParticleInfo.h>
#include <tracking/dataobjects/V0ValidationVertex.h>

namespace genfit { class Track; }

namespace Belle2 {
  class MCParticle;
  class V0;
  class Track;
  class TrackFitResult;

  template< class T >
  class StoreArray;

  /** This module takes the MCParticles, the RecoTracks and Tracks/V0 in input and produce a root file
   *  containing various histograms showing the efficiencies (as a function of different variables) of the
   *  V0 finding module. Efficiencies normalized to MCParticles and RecoTracks are both produced.
   *  It requires V0ValidationVertexs.
   */

  class EffPlotsModule : public Module, PerformanceEvaluationBaseClass {

  public:

    EffPlotsModule();

    ~EffPlotsModule();

    void initialize() override;

    void beginRun() override;

    void event() override;

    void endRun() override;

    void terminate() override;

  private:

    // cppcheck-suppress unusedPrivateFunction
    bool isV0(const MCParticle& the_mcParticle);

    bool isK_Short(const MCParticle& the_mcParticle);

    bool isLambda0(const MCParticle& the_mcParticle);

    int nMatchedDaughters(const MCParticle& the_mcParticle);

    /** user-defined parameters */
    std::string m_MCParticlesName;
    std::string m_V0sName;
    std::string m_RecoTracksName;
    std::string m_MCRecoTracksName;
    std::string m_TFRColName;
    std::string m_TrackColName;
    std::string m_V0sType;

    bool m_allHistograms;
    bool m_geometricalAccettance;

    MCParticle* mc_dau0 = nullptr;
    MCParticle* mc_dau1 = nullptr;

    TList* m_histoList_MCParticles = nullptr;
    TList* m_histoList_RecoTracks = nullptr;
    TList* m_histoList_Tracks = nullptr;
    TList* m_histoList_Efficiencies = nullptr;
    TList* m_histoList_GA = nullptr;
    TList* m_histoList_check = nullptr;

    /** list of histograms filled per MCParticle found in the event */
    TH1F* m_h1_MC_dau0_d0 = nullptr;
    TH1F* m_h1_MC_dau0_z0 = nullptr;
    TH1F* m_h1_MC_dau0_RMother = nullptr;
    TH3F* m_h3_MC_dau0 = nullptr;
    TH1F* m_h1_MC_dau0_pt = nullptr;
    TH1F* m_h1_MC_dau0_pz = nullptr;
    TH1F* m_h1_MC_dau0_p = nullptr;
    TH1F* m_h1_MC_dau0_phi = nullptr;
    TH1F* m_h1_MC_dau0_phi_BW = nullptr;
    TH1F* m_h1_MC_dau0_phi_barrel = nullptr;
    TH1F* m_h1_MC_dau0_phi_FW = nullptr;
    TH1F* m_h1_MC_dau0_theta = nullptr;
    TH1F* m_h1_MC_dau0_costheta = nullptr;
    TH1F* m_h1_MC_dau0_Mother_cosAngle = nullptr;
    TH1F* m_h1_MC_dau0_phiMother_total = nullptr;
    TH1F* m_h1_MC_dau0_phiMother_BW = nullptr;
    TH1F* m_h1_MC_dau0_phiMother_barrel = nullptr;
    TH1F* m_h1_MC_dau0_phiMother_FW = nullptr;
    TH1F* m_h1_MC_dau0_thetaMother = nullptr;
    TH1F* m_h1_MC_dau0_ptMother = nullptr;
    TH2F* m_h2_MC_dau0_2D = nullptr;
    TH2F* m_h2_MC_dau0_2D_BP = nullptr;
    TH2F* m_h2_MC_dau0_2DMother = nullptr;
    TH2F* m_h2_MC_dau0_pVScostheta = nullptr;
    TH1F* m_h1_MC_dau0_PDG = nullptr;

    TH1F* m_h1_MC_dau1_d0 = nullptr;
    TH1F* m_h1_MC_dau1_z0 = nullptr;
    TH1F* m_h1_MC_dau1_RMother = nullptr;
    TH3F* m_h3_MC_dau1 = nullptr;
    TH1F* m_h1_MC_dau1_pt = nullptr;
    TH1F* m_h1_MC_dau1_pz = nullptr;
    TH1F* m_h1_MC_dau1_p = nullptr;
    TH1F* m_h1_MC_dau1_phi = nullptr;
    TH1F* m_h1_MC_dau1_phi_BW = nullptr;
    TH1F* m_h1_MC_dau1_phi_barrel = nullptr;
    TH1F* m_h1_MC_dau1_phi_FW = nullptr;
    TH1F* m_h1_MC_dau1_theta = nullptr;
    TH1F* m_h1_MC_dau1_costheta = nullptr;
    TH1F* m_h1_MC_dau1_Mother_cosAngle = nullptr;
    TH1F* m_h1_MC_dau1_phiMother_total = nullptr;
    TH1F* m_h1_MC_dau1_phiMother_BW = nullptr;
    TH1F* m_h1_MC_dau1_phiMother_barrel = nullptr;
    TH1F* m_h1_MC_dau1_phiMother_FW = nullptr;
    TH1F* m_h1_MC_dau1_thetaMother = nullptr;
    TH1F* m_h1_MC_dau1_ptMother = nullptr;
    TH2F* m_h2_MC_dau1_2D = nullptr;
    TH2F* m_h2_MC_dau1_2D_BP = nullptr;
    TH2F* m_h2_MC_dau1_2DMother = nullptr;
    TH2F* m_h2_MC_dau1_pVScostheta = nullptr;
    TH1F* m_h1_MC_dau1_PDG = nullptr;

    TH1F* m_h1_MC_Mother_RMother = nullptr;
    TH3F* m_h3_MC_Mother = nullptr;
    TH1F* m_h1_MC_Mother_pt = nullptr;
    TH1F* m_h1_MC_Mother_pz = nullptr;
    TH1F* m_h1_MC_Mother_p = nullptr;
    TH1F* m_h1_MC_Mother_phi = nullptr;
    TH1F* m_h1_MC_Mother_phi_BW = nullptr;
    TH1F* m_h1_MC_Mother_phi_barrel = nullptr;
    TH1F* m_h1_MC_Mother_phi_FW = nullptr;
    TH1F* m_h1_MC_Mother_theta = nullptr;
    TH1F* m_h1_MC_Mother_costheta = nullptr;
    TH2F* m_h2_MC_Mother_2D = nullptr;
    TH2F* m_h2_MC_Mother_2D_BP = nullptr;
    TH2F* m_h2_MC_Mother_pVScostheta = nullptr;
    TH1F* m_h1_MC_Mother_PDG = nullptr;

    /** list of histograms filled per Tracks/V0 found in the event */
    TH1F* m_h1_track_dau0_d0 = nullptr;
    TH1F* m_h1_track_dau0_z0 = nullptr;
    TH1F* m_h1_track_dau0_RMother = nullptr;
    TH3F* m_h3_track_dau0 = nullptr;
    TH1F* m_h1_track_dau0_pt = nullptr;
    TH1F* m_h1_track_dau0_pz = nullptr;
    TH1F* m_h1_track_dau0_p = nullptr;
    TH1F* m_h1_track_dau0_phi = nullptr;
    TH1F* m_h1_track_dau0_phi_BW = nullptr;
    TH1F* m_h1_track_dau0_phi_barrel = nullptr;
    TH1F* m_h1_track_dau0_phi_FW = nullptr;
    TH1F* m_h1_track_dau0_theta = nullptr;
    TH1F* m_h1_track_dau0_costheta = nullptr;
    TH1F* m_h1_track_dau0_Mother_cosAngle = nullptr;
    TH1F* m_h1_track_dau0_phiMother_total = nullptr;
    TH1F* m_h1_track_dau0_phiMother_BW = nullptr;
    TH1F* m_h1_track_dau0_phiMother_barrel = nullptr;
    TH1F* m_h1_track_dau0_phiMother_FW = nullptr;
    TH1F* m_h1_track_dau0_thetaMother = nullptr;
    TH1F* m_h1_track_dau0_ptMother = nullptr;
    TH2F* m_h2_track_dau0_2D = nullptr;
    TH2F* m_h2_track_dau0_2D_BP = nullptr;
    TH2F* m_h2_track_dau0_2DMother = nullptr;
    TH2F* m_h2_track_dau0_pVScostheta = nullptr;

    TH1F* m_h1_track_dau1_d0 = nullptr;
    TH1F* m_h1_track_dau1_z0 = nullptr;
    TH1F* m_h1_track_dau1_RMother = nullptr;
    TH3F* m_h3_track_dau1 = nullptr;
    TH1F* m_h1_track_dau1_pt = nullptr;
    TH1F* m_h1_track_dau1_pz = nullptr;
    TH1F* m_h1_track_dau1_p = nullptr;
    TH1F* m_h1_track_dau1_phi = nullptr;
    TH1F* m_h1_track_dau1_phi_BW = nullptr;
    TH1F* m_h1_track_dau1_phi_barrel = nullptr;
    TH1F* m_h1_track_dau1_phi_FW = nullptr;
    TH1F* m_h1_track_dau1_theta = nullptr;
    TH1F* m_h1_track_dau1_costheta = nullptr;
    TH1F* m_h1_track_dau1_Mother_cosAngle = nullptr;
    TH1F* m_h1_track_dau1_phiMother_total = nullptr;
    TH1F* m_h1_track_dau1_phiMother_BW = nullptr;
    TH1F* m_h1_track_dau1_phiMother_barrel = nullptr;
    TH1F* m_h1_track_dau1_phiMother_FW = nullptr;
    TH1F* m_h1_track_dau1_thetaMother = nullptr;
    TH1F* m_h1_track_dau1_ptMother = nullptr;
    TH2F* m_h2_track_dau1_2D = nullptr;
    TH2F* m_h2_track_dau1_2D_BP = nullptr;
    TH2F* m_h2_track_dau1_2DMother = nullptr;
    TH2F* m_h2_track_dau1_pVScostheta = nullptr;

    TH1F* m_h1_V0_RMother = nullptr;
    TH3F* m_h3_V0 = nullptr;
    TH1F* m_h1_V0_pt = nullptr;
    TH1F* m_h1_V0_pz = nullptr;
    TH1F* m_h1_V0_p = nullptr;
    TH1F* m_h1_V0_phi = nullptr;
    TH1F* m_h1_V0_phi_BW = nullptr;
    TH1F* m_h1_V0_phi_barrel = nullptr;
    TH1F* m_h1_V0_phi_FW = nullptr;
    TH1F* m_h1_V0_theta = nullptr;
    TH1F* m_h1_V0_costheta = nullptr;
    TH2F* m_h2_V0_Mother_2D = nullptr;
    TH2F* m_h2_V0_Mother_2D_BP = nullptr;
    TH2F* m_h2_V0_Mother_pVScostheta = nullptr;

    /** list of histograms filled per RecoTracks found in the event */
    TH1F* m_h1_RecoTrack_dau0_d0 = nullptr;
    TH1F* m_h1_RecoTrack_dau0_z0 = nullptr;
    TH1F* m_h1_RecoTrack_dau0_RMother = nullptr;
    TH3F* m_h3_RecoTrack_dau0 = nullptr;
    TH1F* m_h1_RecoTrack_dau0_pt = nullptr;
    TH1F* m_h1_RecoTrack_dau0_pz = nullptr;
    TH1F* m_h1_RecoTrack_dau0_p = nullptr;
    TH1F* m_h1_RecoTrack_dau0_phi = nullptr;
    TH1F* m_h1_RecoTrack_dau0_phi_BW = nullptr;
    TH1F* m_h1_RecoTrack_dau0_phi_barrel = nullptr;
    TH1F* m_h1_RecoTrack_dau0_phi_FW = nullptr;
    TH1F* m_h1_RecoTrack_dau0_theta = nullptr;
    TH1F* m_h1_RecoTrack_dau0_costheta = nullptr;
    TH1F* m_h1_RecoTrack_dau0_Mother_cosAngle = nullptr;
    TH1F* m_h1_RecoTrack_dau0_phiMother_total = nullptr;
    TH1F* m_h1_RecoTrack_dau0_phiMother_BW = nullptr;
    TH1F* m_h1_RecoTrack_dau0_phiMother_barrel = nullptr;
    TH1F* m_h1_RecoTrack_dau0_phiMother_FW = nullptr;
    TH1F* m_h1_RecoTrack_dau0_thetaMother = nullptr;
    TH1F* m_h1_RecoTrack_dau0_ptMother = nullptr;
    TH2F* m_h2_RecoTrack_dau0_2D = nullptr;
    TH2F* m_h2_RecoTrack_dau0_2D_BP = nullptr;
    TH2F* m_h2_RecoTrack_dau0_2DMother = nullptr;
    TH2F* m_h2_RecoTrack_dau0_pVScostheta = nullptr;

    TH1F* m_h1_RecoTrack_dau1_d0 = nullptr;
    TH1F* m_h1_RecoTrack_dau1_z0 = nullptr;
    TH1F* m_h1_RecoTrack_dau1_RMother = nullptr;
    TH3F* m_h3_RecoTrack_dau1 = nullptr;
    TH1F* m_h1_RecoTrack_dau1_pt = nullptr;
    TH1F* m_h1_RecoTrack_dau1_pz = nullptr;
    TH1F* m_h1_RecoTrack_dau1_p = nullptr;
    TH1F* m_h1_RecoTrack_dau1_phi = nullptr;
    TH1F* m_h1_RecoTrack_dau1_phi_BW = nullptr;
    TH1F* m_h1_RecoTrack_dau1_phi_barrel = nullptr;
    TH1F* m_h1_RecoTrack_dau1_phi_FW = nullptr;
    TH1F* m_h1_RecoTrack_dau1_theta = nullptr;
    TH1F* m_h1_RecoTrack_dau1_costheta = nullptr;
    TH1F* m_h1_RecoTrack_dau1_Mother_cosAngle = nullptr;
    TH1F* m_h1_RecoTrack_dau1_phiMother_total = nullptr;
    TH1F* m_h1_RecoTrack_dau1_phiMother_BW = nullptr;
    TH1F* m_h1_RecoTrack_dau1_phiMother_barrel = nullptr;
    TH1F* m_h1_RecoTrack_dau1_phiMother_FW = nullptr;
    TH1F* m_h1_RecoTrack_dau1_thetaMother = nullptr;
    TH1F* m_h1_RecoTrack_dau1_ptMother = nullptr;
    TH2F* m_h2_RecoTrack_dau1_2D = nullptr;
    TH2F* m_h2_RecoTrack_dau1_2D_BP = nullptr;
    TH2F* m_h2_RecoTrack_dau1_2DMother = nullptr;
    TH2F* m_h2_RecoTrack_dau1_pVScostheta = nullptr;

    TH1F* m_h1_RecoTrack_Mother_RMother = nullptr;
    TH3F* m_h3_RecoTrack_Mother = nullptr;
    TH1F* m_h1_RecoTrack_Mother_pt = nullptr;
    TH1F* m_h1_RecoTrack_Mother_pz = nullptr;
    TH1F* m_h1_RecoTrack_Mother_p = nullptr;
    TH1F* m_h1_RecoTrack_Mother_phi = nullptr;
    TH1F* m_h1_RecoTrack_Mother_phi_BW = nullptr;
    TH1F* m_h1_RecoTrack_Mother_phi_barrel = nullptr;
    TH1F* m_h1_RecoTrack_Mother_phi_FW = nullptr;
    TH1F* m_h1_RecoTrack_Mother_theta = nullptr;
    TH1F* m_h1_RecoTrack_Mother_costheta = nullptr;
    TH2F* m_h2_RecoTrack_Mother_2D = nullptr;
    TH2F* m_h2_RecoTrack_Mother_2D_BP = nullptr;
    TH2F* m_h2_RecoTrack_Mother_pVScostheta = nullptr;

  };
} // end of namespace

#endif /* EFFPLOTSMODULE_H_*/
