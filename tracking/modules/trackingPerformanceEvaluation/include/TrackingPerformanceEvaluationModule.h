/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <tracking/modules/trackingPerformanceEvaluation/PerformanceEvaluationBaseClass.h>

#include <TList.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>

#include <mdst/dataobjects/MCParticle.h>
#include <tracking/dataobjects/MCParticleInfo.h>

// forward declarations
namespace Belle2 {
  class Track;
  class TrackFitResult;

  /** This module takes the MCParticles, the Tracks, the RecoTrack,
   *  and the MCRecoTracks input and produce a root file containing various histograms
   *  showing the performance of the tracking package: fitter, pattern recongnition algorithms.
   */
  class TrackingPerformanceEvaluationModule : public Module, PerformanceEvaluationBaseClass {

  public:

    TrackingPerformanceEvaluationModule();

    ~TrackingPerformanceEvaluationModule();

    void initialize() override;

    void beginRun() override;

    void event() override;

    void endRun() override;

    void terminate() override;

  private:

    //list of functions to fill histograms
    void fillTrackParams1DHistograms(const TrackFitResult* fitResult,
                                     MCParticleInfo mcParticleInfo); /**< fills err, resid and pull TH1F for each of the 5 track parameters*/

    void fillTrackErrParams2DHistograms(const TrackFitResult* fitResult); /**< fills TH2F*/

    void fillHitsUsedInTrackFitHistograms(const Track& track); /**< fill TH2F*/

    bool isTraceable(const MCParticle& the_mcParticle); /**< is traceable*/

    void addMoreEfficiencyPlots(TList* histoList); /**< add efficiency plots*/
    void addMoreInefficiencyPlots(TList* histoList); /**< add inefficiency plots*/

    /* user-defined parameters */
    std::string m_MCParticlesName; /**< MCParticle StoreArray name */
    std::string m_MCRecoTracksName; /**< MCRecoTrack StoreArray name */
    std::string m_RecoTracksName; /**< RecoTrack StoreArray name */
    std::string m_TracksName; /**< Track StoreArray name */
    int m_ParticleHypothesis;  /**< Particle Hypothesis for the track fit (default: 211) */

    /* list of histograms filled per MCParticle found in the event */

    TH1F* m_multiplicityTracks = nullptr; /**< number of tracks per MCParticles*/
    TH1F* m_multiplicityRecoTracks = nullptr; /**< number of recoTracks per MCParticles*/
    TH1F* m_multiplicityMCRecoTracks = nullptr; /**< number of MCRecoTracks per MCParticles*/
    TH1F* m_multiplicityFittedTracks = nullptr; /**< number of fitted tracks per MCParticles*/
    TH1F* m_multiplicityFittedTracksPerMCRT = nullptr; /**< number of fitted tracks per MCRecoTrack*/
    TH1F* m_multiplicityMCParticlesPerTrack = nullptr;  /**< number of MCParticles per fitted Track*/
    TH1F* m_multiplicityRecoTracksPerMCRT = nullptr;  /**< number of RecoTracks per MCRecoTracks*/
    TH1F* m_multiplicityMCRecoTracksPerRT = nullptr;  /**< number of MCRecoTracks per RecoTracks*/

    //error on track parameters
    TH1F* m_h1_d0_err = nullptr; /**< error */
    TH1F* m_h1_phi_err = nullptr; /**< error */
    TH1F* m_h1_omega_err = nullptr; /**< error */
    TH1F* m_h1_z0_err = nullptr; /**< error */
    TH1F* m_h1_cotTheta_err = nullptr; /**< error */
    //residuals on track parameters
    TH1F* m_h1_d0_res = nullptr; /**< error */
    TH1F* m_h1_phi_res = nullptr; /**< error */
    TH1F* m_h1_omega_res = nullptr; /**< error */
    TH1F* m_h1_z0_res = nullptr; /**< error */
    TH1F* m_h1_cotTheta_res = nullptr; /**< error */
    //residuals on momentum parameters
    TH1F* m_h1_px_res = nullptr; /**< px residual */
    TH1F* m_h1_py_res = nullptr; /**< py residual */
    TH1F* m_h1_pz_res = nullptr; /**< pz residual */
    TH1F* m_h1_p_res = nullptr; /**< p residual */
    TH1F* m_h1_pt_res = nullptr; /**< pt residual */
    //residuals on position parameters
    TH1F* m_h1_x_res = nullptr; /**< x residual */
    TH1F* m_h1_y_res = nullptr; /**< y residual */
    TH1F* m_h1_z_res = nullptr; /**< z residual */
    TH1F* m_h1_r_res = nullptr; /**< R residual (in cylindrical coordinates) */
    TH1F* m_h1_rtot_res = nullptr; /**< r residual (3D distance) */
    //pulls on track parameters
    TH1F* m_h1_d0_pll = nullptr; /**< pull  distribution d0*/
    TH1F* m_h1_phi_pll = nullptr; /**< pull  distribution phi*/
    TH1F* m_h1_omega_pll = nullptr; /**< pull  distribution omega*/
    TH1F* m_h1_z0_pll = nullptr; /**< pull  distribution z0*/
    TH1F* m_h1_cotTheta_pll = nullptr; /**< pull distribution cotTheta*/



    TH2F* m_h2_d0errphi0err_xy = nullptr; /**< error */
    TH2F* m_h2_d0errphi0err_rz = nullptr; /**< error */
    TH2F* m_h2_z0errcotThetaerr_xy = nullptr; /**< error */

    TH2F* m_h2_VXDhitsPR_xy = nullptr; /**< PR */
    TH2F* m_h2_VXDhitsPR_rz = nullptr; /**< PR */
    TH1F* m_h1_nVXDhitsPR = nullptr; /**< PR */
    TH1F* m_h1_nVXDhitsWeighted = nullptr; /**< weighted */
    TH1F* m_h1_nVXDhitsUsed = nullptr; /**< hits used */
    TH1F* m_h1_nCDChitsPR = nullptr; /**< PR */
    TH1F* m_h1_nCDChitsWeighted = nullptr; /**< weighted */
    TH1F* m_h1_nCDChitsUsed = nullptr;/**<used */
    TH1F* m_h1_nHitDetID = nullptr; /**< det ID */
    TH2F* m_h2_TrackPointFitWeightVXD = nullptr; /**< TP */
    TH2F* m_h2_TrackPointFitWeightCDC = nullptr; /**< TP */

    TH1F* m_h1_pValue = nullptr; /**< p val */

    TH2F* m_h2_OmegaerrOmegaVSpt = nullptr;  /**< error */

    TH2F* m_h2_z0errVSpt_wtpxd = nullptr;  /**< error */
    TH2F* m_h2_z0errVSpt_wfpxd = nullptr;  /**< error */
    TH2F* m_h2_z0errVSpt_wpxd = nullptr;  /**< error */
    TH2F* m_h2_z0errVSpt_wopxd = nullptr; /**< error */
    TH2F* m_h2_z0errVSpt = nullptr; /**< error */
    TH2F* m_h2_d0errVSpt_wtpxd = nullptr; /**< error */
    TH2F* m_h2_d0errVSpt_wfpxd = nullptr; /**< error */
    TH2F* m_h2_d0errVSpt_wpxd = nullptr; /**< error */
    TH2F* m_h2_d0errVSpt_wopxd = nullptr; /**< error */
    TH2F* m_h2_d0errVSpt = nullptr; /**< error */
    TH2F* m_h2_d0errMSVSpt = nullptr; /**< error */

    TH2F* m_h2_chargeVSchargeMC = nullptr; /**< charge comparison */

    //histograms used for efficiency plots
    TH1F* m_h1_HitsRecoTrackPerMCRecoTrack = nullptr;  /**< hits */
    TH1F* m_h1_HitsMCRecoTrack = nullptr;  /**< hits */

    TH3F* m_h3_MCParticle = nullptr;  /**< efficiency*/
    TH3F* m_h3_MCParticleswPXDHits = nullptr;  /**< efficiency*/
    TH3F* m_h3_TracksPerMCParticle = nullptr;  /**< efficiency*/
    TH3F* m_h3_TrackswPXDHitsPerMCParticle = nullptr;  /**< efficiency*/
    TH3F* m_h3_RecoTrackswPXDHitsPerMCParticle = nullptr;  /**< efficiency*/
    TH3F* m_h3_RecoTrackswPXDHitsPerMCParticlewPXDHits = nullptr;  /**< efficiency*/
    TH3F* m_h3_MCRecoTrack = nullptr;  /**< efficiency*/
    TH3F* m_h3_TracksPerMCRecoTrack = nullptr;  /**< efficiency*/

    TH3F* m_h3_MCParticle_plus = nullptr;  /**< efficiency*/
    TH3F* m_h3_TracksPerMCParticle_plus = nullptr;  /**< efficiency*/
    TH3F* m_h3_MCRecoTrack_plus = nullptr;  /**< efficiency*/
    TH3F* m_h3_TracksPerMCRecoTrack_plus = nullptr;  /**< efficiency*/
    TH3F* m_h3_MCParticle_minus = nullptr;  /**< efficiency*/
    TH3F* m_h3_TracksPerMCParticle_minus = nullptr;  /**< efficiency*/
    TH3F* m_h3_MCRecoTrack_minus = nullptr;  /**< efficiency*/
    TH3F* m_h3_TracksPerMCRecoTrack_minus = nullptr;  /**< efficiency*/

    //histograms used for purity plots
    TH3F* m_h3_MCParticlesPerTrack = nullptr;  /**< purity*/
    TH3F* m_h3_Tracks = nullptr; /**< purity*/

  };
} // end of namespace
