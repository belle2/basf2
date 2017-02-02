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
namespace genfit { class Track; }

namespace Belle2 {
  class MCParticle;
  class Track;
  class TrackFitResult;

  template< class T >
  class StoreArray;


  /** This module takes the MCParticles, the genfit Tracks, the genfit TrackCand,
   *  and the MCTrackCands input and produce a root file containing various histograms
   *  showing the performance of the tracking package: fitter, pattern recongnition algorithms.
   */
  class TrackingPerformanceEvaluationModule : public Module, PerformanceEvaluationBaseClass {

  public:

    TrackingPerformanceEvaluationModule();

    virtual ~TrackingPerformanceEvaluationModule();

    virtual void initialize();

    virtual void beginRun();

    virtual void event();

    virtual void endRun();

    virtual void terminate();

  private:

    //list of functions to fill histograms
    void fillTrackParams1DHistograms(const TrackFitResult* fitResult,
                                     MCParticleInfo mcParticleInfo); /**< fills err, resid and pull TH1F for each of the 5 track parameters*/

    void fillTrackErrParams2DHistograms(const TrackFitResult* fitResult); /**< fills TH2F*/

    void fillHitsUsedInTrackFitHistograms(const genfit::Track& track); /**< fill TH2F*/

    bool isTraceable(const MCParticle& the_mcParticle); /**< is traceable*/

    void addMoreEfficiencyPlots(TList* histoList); /**< add efficiency plots*/
    void addMoreInefficiencyPlots(TList* histoList); /**< add inefficiency plots*/

    /* user-defined parameters */
    std::string m_MCParticlesName; /**< MCParticle StoreArray name */
    std::string m_MCTrackCandsName; /**< MCTrackCand StoreArray name */
    std::string m_TrackCandsName; /**< TrackCand StoreArray name */
    std::string m_TracksName; /**< Track StoreArray name */


    /* list of histograms filled per MCParticle found in the event */

    TH1F* m_multiplicityTracks; /**< number of tracks per MCParticles*/
    TH1F* m_multiplicityFittedTracks; /**< number of fitted tracks per MCParticles*/
    TH1F* m_multiplicityFittedTracksPerMCTC; /**< number of fitted tracks per MCTrackCand*/
    TH1F* m_multiplicityMCParticles;  /**< number of MCParticles per fitted Track*/
    TH1F* m_multiplicityTrackCands;  /**< number of TrackCands per MCTrackCands*/
    TH1F* m_multiplicityMCTrackCands;  /**< number of MCTrackCands per TrackCands*/

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


    //histograms used for efficiency plots
    TH1F* m_h1_HitsTrackCandPerMCTrackCand;  /**< hits */
    TH1F* m_h1_HitsMCTrackCand;  /**< hits */

    TH3F* m_h3_MCParticle;  /**< efficiency*/
    TH3F* m_h3_TracksPerMCParticle;  /**< efficiency*/
    TH3F* m_h3_TrackswPXDHitsPerMCParticle;  /**< efficiency*/
    TH3F* m_h3_MCTrackCand;  /**< efficiency*/
    TH3F* m_h3_TracksPerMCTrackCand;  /**< efficiency*/

    TH3F* m_h3_MCParticle_plus;  /**< efficiency*/
    TH3F* m_h3_TracksPerMCParticle_plus;  /**< efficiency*/
    TH3F* m_h3_MCTrackCand_plus;  /**< efficiency*/
    TH3F* m_h3_TracksPerMCTrackCand_plus;  /**< efficiency*/
    TH3F* m_h3_MCParticle_minus;  /**< efficiency*/
    TH3F* m_h3_TracksPerMCParticle_minus;  /**< efficiency*/
    TH3F* m_h3_MCTrackCand_minus;  /**< efficiency*/
    TH3F* m_h3_TracksPerMCTrackCand_minus;  /**< efficiency*/

    //histograms used for purity plots
    TH3F* m_h3_MCParticlesPerTrack;  /**< purityy*/
    TH3F* m_h3_Tracks; /**< purity*/

  };
} // end of namespace


#endif /* TRACKINGPERFORMANCEEVALUAITONMODULE_H_ */
