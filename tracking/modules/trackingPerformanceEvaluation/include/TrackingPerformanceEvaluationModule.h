/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef TRACKINGPERFORMANCEEVALUATIONMODULE_H_
#define TRACKINGPERFORMANCEEVALUATIONMODULE_H_

#include <framework/core/Module.h>
#include <tracking/modules/trackingPerformanceEvaluation/PerformanceEvaluationBaseClass.h>

#include <TTree.h>
#include <TFile.h>
#include <TList.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>

#include <mdst/dataobjects/MCParticle.h>
#include <tracking/dataobjects/MCParticleInfo.h>

// forward declarations
namespace Belle2 {
  class MCParticle;
  class Track;
  class TrackFitResult;

  template< class T >
  class StoreArray;


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

    TH1F* m_multiplicityTracks; /**< number of tracks per MCParticles*/
    TH1F* m_multiplicityRecoTracks; /**< number of recoTracks per MCParticles*/
    TH1F* m_multiplicityMCRecoTracks; /**< number of MCRecoTracks per MCParticles*/
    TH1F* m_multiplicityFittedTracks; /**< number of fitted tracks per MCParticles*/
    TH1F* m_multiplicityFittedTracksPerMCRT; /**< number of fitted tracks per MCRecoTrack*/
    TH1F* m_multiplicityMCParticlesPerTrack;  /**< number of MCParticles per fitted Track*/
    TH1F* m_multiplicityRecoTracksPerMCRT;  /**< number of RecoTracks per MCRecoTracks*/
    TH1F* m_multiplicityMCRecoTracksPerRT;  /**< number of MCRecoTracks per RecoTracks*/

    //error on track parameters
    TH1F* m_h1_d0_err; /**< error */
    TH1F* m_h1_phi_err; /**< error */
    TH1F* m_h1_omega_err; /**< error */
    TH1F* m_h1_z0_err; /**< error */
    TH1F* m_h1_cotTheta_err; /**< error */
    //residuals on track parameters
    TH1F* m_h1_d0_res; /**< error */
    TH1F* m_h1_phi_res; /**< error */
    TH1F* m_h1_omega_res; /**< error */
    TH1F* m_h1_z0_res; /**< error */
    TH1F* m_h1_cotTheta_res; /**< error */
    //residuals on momentum parameters
    TH1F* m_h1_px_res; /**< px residual */
    TH1F* m_h1_py_res; /**< py residual */
    TH1F* m_h1_pz_res; /**< pz residual */
    TH1F* m_h1_p_res; /**< p residual */
    TH1F* m_h1_pt_res; /**< pt residual */
    //residuals on position parameters
    TH1F* m_h1_x_res; /**< x residual */
    TH1F* m_h1_y_res; /**< y residual */
    TH1F* m_h1_z_res; /**< z residual */
    TH1F* m_h1_r_res; /**< R residual (in cylindrical coordinates) */
    TH1F* m_h1_rtot_res; /**< r residual (3D distance) */
    //pulls on track parameters
    TH1F* m_h1_d0_pll; /**< error */
    TH1F* m_h1_phi_pll; /**< error */
    TH1F* m_h1_omega_pll; /**< error */
    TH1F* m_h1_z0_pll; /**< error */
    TH1F* m_h1_cotTheta_pll; /**< error */



    TH2F* m_h2_d0errphi0err_xy; /**< error */
    TH2F* m_h2_d0errphi0err_rz; /**< error */
    TH2F* m_h2_z0errcotThetaerr_xy; /**< error */

    TH2F* m_h2_VXDhitsPR_xy; /**< PR */
    TH2F* m_h2_VXDhitsPR_rz; /**< PR */
    TH1F* m_h1_nVXDhitsPR; /**< PR */
    TH1F* m_h1_nVXDhitsWeighted; /**< weighted */
    TH1F* m_h1_nVXDhitsUsed; /**< hits used */
    TH1F* m_h1_nCDChitsPR; /**< PR */
    TH1F* m_h1_nCDChitsWeighted; /**< weighted */
    TH1F* m_h1_nCDChitsUsed;/**<used */
    TH1F* m_h1_nHitDetID; /**< det ID */
    TH2F* m_h2_TrackPointFitWeightVXD; /**< TP */
    TH2F* m_h2_TrackPointFitWeightCDC; /**< TP */

    TH1F* m_h1_pValue; /**< p val */

    TH2F* m_h2_OmegaerrOmegaVSpt;  /**< error */

    TH2F* m_h2_z0errVSpt_wpxd;  /**< error */
    TH2F* m_h2_z0errVSpt_wopxd; /**< error */
    TH2F* m_h2_z0errVSpt; /**< error */
    TH2F* m_h2_d0errVSpt_wpxd; /**< error */
    TH2F* m_h2_d0errVSpt_wopxd; /**< error */
    TH2F* m_h2_d0errVSpt; /**< error */
    TH2F* m_h2_d0errMSVSpt; /**< error */

    TH2F* m_h2_chargeVSchargeMC; //**< charge comparison */

    //histograms used for efficiency plots
    TH1F* m_h1_HitsRecoTrackPerMCRecoTrack;  /**< hits */
    TH1F* m_h1_HitsMCRecoTrack;  /**< hits */

    TH3F* m_h3_MCParticle;  /**< efficiency*/
    TH3F* m_h3_MCParticleswPXDHits;  /**< efficiency*/
    TH3F* m_h3_TracksPerMCParticle;  /**< efficiency*/
    TH3F* m_h3_TrackswPXDHitsPerMCParticle;  /**< efficiency*/
    TH3F* m_h3_RecoTrackswPXDHitsPerMCParticle;  /**< efficiency*/
    TH3F* m_h3_RecoTrackswPXDHitsPerMCParticlewPXDHits;  /**< efficiency*/
    TH3F* m_h3_MCRecoTrack;  /**< efficiency*/
    TH3F* m_h3_TracksPerMCRecoTrack;  /**< efficiency*/

    TH3F* m_h3_MCParticle_plus;  /**< efficiency*/
    TH3F* m_h3_TracksPerMCParticle_plus;  /**< efficiency*/
    TH3F* m_h3_MCRecoTrack_plus;  /**< efficiency*/
    TH3F* m_h3_TracksPerMCRecoTrack_plus;  /**< efficiency*/
    TH3F* m_h3_MCParticle_minus;  /**< efficiency*/
    TH3F* m_h3_TracksPerMCParticle_minus;  /**< efficiency*/
    TH3F* m_h3_MCRecoTrack_minus;  /**< efficiency*/
    TH3F* m_h3_TracksPerMCRecoTrack_minus;  /**< efficiency*/

    //histograms used for purity plots
    TH3F* m_h3_MCParticlesPerTrack;  /**< purityy*/
    TH3F* m_h3_Tracks; /**< purity*/

  };
} // end of namespace


#endif /* TRACKINGPERFORMANCEEVALUAITONMODULE_H_ */
