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
#include <TTree.h>
#include <TFile.h>
#include <TList.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>

#include <mdst/dataobjects/MCParticle.h>
#include <tracking/modules/trackingPerformanceEvaluation/MCParticleInfo.h>

// forward declarations
class TTree;
class TFile;
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
  class TrackingPerformanceEvaluationModule : public Module {

  public:

    TrackingPerformanceEvaluationModule();

    virtual ~TrackingPerformanceEvaluationModule();

    virtual void initialize();

    virtual void beginRun();

    virtual void event();

    virtual void endRun();

    virtual void terminate();

  private:

    TList* m_histoList;

    //list of functions to create histograms:
    TH1F* createHistogram1D(const char* name, const char* title,
                            Int_t nbins, Double_t min, Double_t max,
                            const char* xtitle, TList* histoList = NULL);

    TH2F* createHistogram2D(const char* name, const char* title,
                            Int_t nbinsX, Double_t minX, Double_t maxX, const char* titleX,
                            Int_t nbinsY, Double_t minY, Double_t maxY, const char* titleY,
                            TList* histoList = NULL);

    TH3F* createHistogram3D(const char* name, const char* title,
                            Int_t nbinsX, Double_t minX, Double_t maxX, const char* titleX,
                            Int_t nbinsY, Double_t minY, Double_t maxY, const char* titleY,
                            Int_t nbinsZ, Double_t minZ, Double_t maxZ, const char* titleZ,
                            TList* histoList = NULL);

    TH3F* createHistogram3D(const char* name, const char* title,
                            Int_t nbinsX, Double_t* binsX, const char* titleX,
                            Int_t nbinsY, Double_t* binsY, const char* titleY,
                            Int_t nbinsZ, Double_t* binsZ, const char* titleZ,
                            TList* histoList = NULL);

    TH1* duplicateHistogram(const char* newname, const char* newtitle,
                            TH1* h, TList* histoList = NULL);


    TH1F* createHistogramsRatio(const char* name, const char* title,
                                TH1* hNum, TH1* hDen, bool isEffPlot,
                                int axisRef);


    //list of functions to fill histograms
    void fillTrackParams1DHistograms(const TrackFitResult* fitResult, MCParticleInfo mcParticleInfo); /**< fills err, resid and pull TH1F for each of the 5 track parameters*/

    void fillTrackErrParams2DHistograms(const TrackFitResult* fitResult); /**< fills TH2F*/

    void fillHitsUsedInTrackFitHistograms(const genfit::Track& track);

    void addEfficiencyPlots(TList* graphList = NULL);

    void addInefficiencyPlots(TList* graphList = NULL);

    void addPurityPlots(TList* graphList = NULL);



    bool isTraceable(const MCParticle& the_mcParticle);

    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */
    std::string m_MCParticlesName; /**< MCParticle StoreArray name */
    std::string m_MCTrackCandsName; /**< MCTrackCand StoreArray name */
    std::string m_TrackCandsName; /**< TrackCand StoreArray name */
    std::string m_TracksName; /**< Track StoreArray name */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr; /**< pointer at root file used for storing histograms */

    /* list of histograms filled per MCParticle found in the event */

    TH1F* m_multiplicityTracks; /**< number of tracks per MCParticles*/
    TH1F* m_multiplicityFittedTracks; /**< number of fitted tracks per MCParticles*/
    TH1F* m_multiplicityFittedTracksPerMCTC; /**< number of fitted tracks per MCTrackCand*/
    TH1F* m_multiplicityMCParticles;  /**< number of MCParticles per fitted Track*/
    TH1F* m_multiplicityTrackCands;  /**< number of TrackCands per MCTrackCands*/
    TH1F* m_multiplicityMCTrackCands;  /**< number of MCTrackCands per TrackCands*/

    //error on track parameters
    TH1F* m_h1_d0_err; /**< */
    TH1F* m_h1_phi_err; /**< */
    TH1F* m_h1_omega_err; /**< */
    TH1F* m_h1_z0_err; /**< */
    TH1F* m_h1_cotTheta_err; /**< */
    //residuals on track parameters
    TH1F* m_h1_d0_res; /**< */
    TH1F* m_h1_phi_res; /**< */
    TH1F* m_h1_omega_res; /**< */
    TH1F* m_h1_z0_res; /**< */
    TH1F* m_h1_cotTheta_res; /**< */
    //pulls on track parameters
    TH1F* m_h1_d0_pll; /**< */
    TH1F* m_h1_phi_pll; /**< */
    TH1F* m_h1_omega_pll; /**< */
    TH1F* m_h1_z0_pll; /**< */
    TH1F* m_h1_cotTheta_pll; /**< */



    TH2F* m_h2_d0errphi0err_xy;
    TH2F* m_h2_d0errphi0err_zt;
    TH2F* m_h2_z0errcotThetaerr_xy;

    TH2F* m_h2_VXDhitsUsed_xy;
    TH1F* m_h1_nVXDhitsUsed;
    TH1F* m_h1_nHitDetID;

    TH1F* m_h1_pValue;

    TH2F* m_h2_d0errVSpt;
    TH2F* m_h2_d0errMSVSpt;


    //histograms used for efficiency plots
    TH1F* m_h1_HitsTrackCandPerMCTrackCand;
    TH1F* m_h1_HitsMCTrackCand;

    TH3F* m_h3_MCParticle;
    TH3F* m_h3_TracksPerMCParticle;
    TH3F* m_h3_MCTrackCand;
    TH3F* m_h3_TracksPerMCTrackCand;

    TH3F* m_h3_MCParticle_plus;
    TH3F* m_h3_TracksPerMCParticle_plus;
    TH3F* m_h3_MCTrackCand_plus;
    TH3F* m_h3_TracksPerMCTrackCand_plus;
    TH3F* m_h3_MCParticle_minus;
    TH3F* m_h3_TracksPerMCParticle_minus;
    TH3F* m_h3_MCTrackCand_minus;
    TH3F* m_h3_TracksPerMCTrackCand_minus;

    //histograms used for purity plots
    TH1F* m_h1_HitsMCTrackCandPerTrackCand;
    TH1F* m_h1_HitsTrackCand;

    TH3F* m_h3_MCParticlesPerTrack;
    TH3F* m_h3_Tracks;

    //debugging variables
    //    Int_t m_nFittedTracks;
    //    Int_t m_nMCParticles;

  };
} // end of namespace


#endif /* TRACKINGPERFORMANCEEVALUAITONMODULE_H_ */
