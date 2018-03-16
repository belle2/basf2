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

    bool isV0(const MCParticle& the_mcParticle);

    bool isK_Short(const MCParticle& the_mcParticle);

    bool isLambda0(const MCParticle& the_mcParticle);

    int nMatchedDaughters(const MCParticle& the_mcParticle);

    /* user-defined parameters */
    std::string m_MCParticlesName;
    std::string m_V0sName;
    std::string m_RecoTracksName;
    std::string m_MCRecoTracksName;
    std::string m_TFRColName;
    std::string m_TrackColName;
    std::string m_V0sType;

    bool m_allHistograms;
    bool m_geometricalAccettance;

    MCParticle* mc_dau0;
    MCParticle* mc_dau1;

    TList* m_histoList_MCParticles;
    TList* m_histoList_RecoTracks;
    TList* m_histoList_Tracks;
    TList* m_histoList_Efficiencies;
    TList* m_histoList_GA;
    TList* m_histoList_check;

    /* list of histograms filled per MCParticle found in the event */
    TH1F* m_h1_MC_dau0_d0;
    TH1F* m_h1_MC_dau0_z0;
    TH1F* m_h1_MC_dau0_RMother;
    TH3F* m_h3_MC_dau0;
    TH1F* m_h1_MC_dau0_pt;
    TH1F* m_h1_MC_dau0_pz;
    TH1F* m_h1_MC_dau0_p;
    TH1F* m_h1_MC_dau0_phi;
    TH1F* m_h1_MC_dau0_phi_BW;
    TH1F* m_h1_MC_dau0_phi_barrel;
    TH1F* m_h1_MC_dau0_phi_FW;
    TH1F* m_h1_MC_dau0_theta;
    TH1F* m_h1_MC_dau0_costheta;
    TH1F* m_h1_MC_dau0_Mother_cosAngle;
    TH1F* m_h1_MC_dau0_phiMother_total;
    TH1F* m_h1_MC_dau0_phiMother_BW;
    TH1F* m_h1_MC_dau0_phiMother_barrel;
    TH1F* m_h1_MC_dau0_phiMother_FW;
    TH1F* m_h1_MC_dau0_thetaMother;
    TH1F* m_h1_MC_dau0_ptMother;
    TH2F* m_h2_MC_dau0_2D;
    TH2F* m_h2_MC_dau0_2D_BP;
    TH2F* m_h2_MC_dau0_2DMother;
    TH2F* m_h2_MC_dau0_pVScostheta;
    TH1F* m_h1_MC_dau0_PDG;

    TH1F* m_h1_MC_dau1_d0;
    TH1F* m_h1_MC_dau1_z0;
    TH1F* m_h1_MC_dau1_RMother;
    TH3F* m_h3_MC_dau1;
    TH1F* m_h1_MC_dau1_pt;
    TH1F* m_h1_MC_dau1_pz;
    TH1F* m_h1_MC_dau1_p;
    TH1F* m_h1_MC_dau1_phi;
    TH1F* m_h1_MC_dau1_phi_BW;
    TH1F* m_h1_MC_dau1_phi_barrel;
    TH1F* m_h1_MC_dau1_phi_FW;
    TH1F* m_h1_MC_dau1_theta;
    TH1F* m_h1_MC_dau1_costheta;
    TH1F* m_h1_MC_dau1_Mother_cosAngle;
    TH1F* m_h1_MC_dau1_phiMother_total;
    TH1F* m_h1_MC_dau1_phiMother_BW;
    TH1F* m_h1_MC_dau1_phiMother_barrel;
    TH1F* m_h1_MC_dau1_phiMother_FW;
    TH1F* m_h1_MC_dau1_thetaMother;
    TH1F* m_h1_MC_dau1_ptMother;
    TH2F* m_h2_MC_dau1_2D;
    TH2F* m_h2_MC_dau1_2D_BP;
    TH2F* m_h2_MC_dau1_2DMother;
    TH2F* m_h2_MC_dau1_pVScostheta;
    TH1F* m_h1_MC_dau1_PDG;

    TH1F* m_h1_MC_Mother_RMother;
    TH3F* m_h3_MC_Mother;
    TH1F* m_h1_MC_Mother_pt;
    TH1F* m_h1_MC_Mother_pz;
    TH1F* m_h1_MC_Mother_p;
    TH1F* m_h1_MC_Mother_phi;
    TH1F* m_h1_MC_Mother_phi_BW;
    TH1F* m_h1_MC_Mother_phi_barrel;
    TH1F* m_h1_MC_Mother_phi_FW;
    TH1F* m_h1_MC_Mother_theta;
    TH1F* m_h1_MC_Mother_costheta;
    TH2F* m_h2_MC_Mother_2D;
    TH2F* m_h2_MC_Mother_2D_BP;
    TH2F* m_h2_MC_Mother_pVScostheta;
    TH1F* m_h1_MC_Mother_PDG;

    /* list of histograms filled per Tracks/V0 found in the event */
    TH1F* m_h1_track_dau0_d0;
    TH1F* m_h1_track_dau0_z0;
    TH1F* m_h1_track_dau0_RMother;
    TH3F* m_h3_track_dau0;
    TH1F* m_h1_track_dau0_pt;
    TH1F* m_h1_track_dau0_pz;
    TH1F* m_h1_track_dau0_p;
    TH1F* m_h1_track_dau0_phi;
    TH1F* m_h1_track_dau0_phi_BW;
    TH1F* m_h1_track_dau0_phi_barrel;
    TH1F* m_h1_track_dau0_phi_FW;
    TH1F* m_h1_track_dau0_theta;
    TH1F* m_h1_track_dau0_costheta;
    TH1F* m_h1_track_dau0_Mother_cosAngle;
    TH1F* m_h1_track_dau0_phiMother_total;
    TH1F* m_h1_track_dau0_phiMother_BW;
    TH1F* m_h1_track_dau0_phiMother_barrel;
    TH1F* m_h1_track_dau0_phiMother_FW;
    TH1F* m_h1_track_dau0_thetaMother;
    TH1F* m_h1_track_dau0_ptMother;
    TH2F* m_h2_track_dau0_2D;
    TH2F* m_h2_track_dau0_2D_BP;
    TH2F* m_h2_track_dau0_2DMother;
    TH2F* m_h2_track_dau0_pVScostheta;

    TH1F* m_h1_track_dau1_d0;
    TH1F* m_h1_track_dau1_z0;
    TH1F* m_h1_track_dau1_RMother;
    TH3F* m_h3_track_dau1;
    TH1F* m_h1_track_dau1_pt;
    TH1F* m_h1_track_dau1_pz;
    TH1F* m_h1_track_dau1_p;
    TH1F* m_h1_track_dau1_phi;
    TH1F* m_h1_track_dau1_phi_BW;
    TH1F* m_h1_track_dau1_phi_barrel;
    TH1F* m_h1_track_dau1_phi_FW;
    TH1F* m_h1_track_dau1_theta;
    TH1F* m_h1_track_dau1_costheta;
    TH1F* m_h1_track_dau1_Mother_cosAngle;
    TH1F* m_h1_track_dau1_phiMother_total;
    TH1F* m_h1_track_dau1_phiMother_BW;
    TH1F* m_h1_track_dau1_phiMother_barrel;
    TH1F* m_h1_track_dau1_phiMother_FW;
    TH1F* m_h1_track_dau1_thetaMother;
    TH1F* m_h1_track_dau1_ptMother;
    TH2F* m_h2_track_dau1_2D;
    TH2F* m_h2_track_dau1_2D_BP;
    TH2F* m_h2_track_dau1_2DMother;
    TH2F* m_h2_track_dau1_pVScostheta;

    TH1F* m_h1_V0_RMother;
    TH3F* m_h3_V0;
    TH1F* m_h1_V0_pt;
    TH1F* m_h1_V0_pz;
    TH1F* m_h1_V0_p;
    TH1F* m_h1_V0_phi;
    TH1F* m_h1_V0_phi_BW;
    TH1F* m_h1_V0_phi_barrel;
    TH1F* m_h1_V0_phi_FW;
    TH1F* m_h1_V0_theta;
    TH1F* m_h1_V0_costheta;
    TH2F* m_h2_V0_Mother_2D;
    TH2F* m_h2_V0_Mother_2D_BP;
    TH2F* m_h2_V0_Mother_pVScostheta;

    /* list of histograms filled per RecoTracks found in the event */
    TH1F* m_h1_RecoTrack_dau0_d0;
    TH1F* m_h1_RecoTrack_dau0_z0;
    TH1F* m_h1_RecoTrack_dau0_RMother;
    TH3F* m_h3_RecoTrack_dau0;
    TH1F* m_h1_RecoTrack_dau0_pt;
    TH1F* m_h1_RecoTrack_dau0_pz;
    TH1F* m_h1_RecoTrack_dau0_p;
    TH1F* m_h1_RecoTrack_dau0_phi;
    TH1F* m_h1_RecoTrack_dau0_phi_BW;
    TH1F* m_h1_RecoTrack_dau0_phi_barrel;
    TH1F* m_h1_RecoTrack_dau0_phi_FW;
    TH1F* m_h1_RecoTrack_dau0_theta;
    TH1F* m_h1_RecoTrack_dau0_costheta;
    TH1F* m_h1_RecoTrack_dau0_Mother_cosAngle;
    TH1F* m_h1_RecoTrack_dau0_phiMother_total;
    TH1F* m_h1_RecoTrack_dau0_phiMother_BW;
    TH1F* m_h1_RecoTrack_dau0_phiMother_barrel;
    TH1F* m_h1_RecoTrack_dau0_phiMother_FW;
    TH1F* m_h1_RecoTrack_dau0_thetaMother;
    TH1F* m_h1_RecoTrack_dau0_ptMother;
    TH2F* m_h2_RecoTrack_dau0_2D;
    TH2F* m_h2_RecoTrack_dau0_2D_BP;
    TH2F* m_h2_RecoTrack_dau0_2DMother;
    TH2F* m_h2_RecoTrack_dau0_pVScostheta;

    TH1F* m_h1_RecoTrack_dau1_d0;
    TH1F* m_h1_RecoTrack_dau1_z0;
    TH1F* m_h1_RecoTrack_dau1_RMother;
    TH3F* m_h3_RecoTrack_dau1;
    TH1F* m_h1_RecoTrack_dau1_pt;
    TH1F* m_h1_RecoTrack_dau1_pz;
    TH1F* m_h1_RecoTrack_dau1_p;
    TH1F* m_h1_RecoTrack_dau1_phi;
    TH1F* m_h1_RecoTrack_dau1_phi_BW;
    TH1F* m_h1_RecoTrack_dau1_phi_barrel;
    TH1F* m_h1_RecoTrack_dau1_phi_FW;
    TH1F* m_h1_RecoTrack_dau1_theta;
    TH1F* m_h1_RecoTrack_dau1_costheta;
    TH1F* m_h1_RecoTrack_dau1_Mother_cosAngle;
    TH1F* m_h1_RecoTrack_dau1_phiMother_total;
    TH1F* m_h1_RecoTrack_dau1_phiMother_BW;
    TH1F* m_h1_RecoTrack_dau1_phiMother_barrel;
    TH1F* m_h1_RecoTrack_dau1_phiMother_FW;
    TH1F* m_h1_RecoTrack_dau1_thetaMother;
    TH1F* m_h1_RecoTrack_dau1_ptMother;
    TH2F* m_h2_RecoTrack_dau1_2D;
    TH2F* m_h2_RecoTrack_dau1_2D_BP;
    TH2F* m_h2_RecoTrack_dau1_2DMother;
    TH2F* m_h2_RecoTrack_dau1_pVScostheta;

    TH1F* m_h1_RecoTrack_Mother_RMother;
    TH3F* m_h3_RecoTrack_Mother;
    TH1F* m_h1_RecoTrack_Mother_pt;
    TH1F* m_h1_RecoTrack_Mother_pz;
    TH1F* m_h1_RecoTrack_Mother_p;
    TH1F* m_h1_RecoTrack_Mother_phi;
    TH1F* m_h1_RecoTrack_Mother_phi_BW;
    TH1F* m_h1_RecoTrack_Mother_phi_barrel;
    TH1F* m_h1_RecoTrack_Mother_phi_FW;
    TH1F* m_h1_RecoTrack_Mother_theta;
    TH1F* m_h1_RecoTrack_Mother_costheta;
    TH2F* m_h2_RecoTrack_Mother_2D;
    TH2F* m_h2_RecoTrack_Mother_2D_BP;
    TH2F* m_h2_RecoTrack_Mother_pVScostheta;

  };
} // end of namespace

#endif /* EFFPLOTSMODULE_H_*/
