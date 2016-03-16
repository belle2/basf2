/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef V0FINDINGPERFORMANCEEVALUATIONMODULE_H_
#define V0FINDINGPERFORMANCEEVALUATIONMODULE_H_

#include <framework/core/Module.h>
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

// forward declarations
class TTree;
class TFile;
namespace genfit { class Track; }

namespace Belle2 {
  class MCParticle;
  class V0;

  template< class T >
  class StoreArray;

  /** This module takes the MCParticles, the genfit Tracks, the genfit TrackCand,
   *  and the MCTrackCands input and produce a root file containing various histograms
   *  showing the performance of the tracking package: fitter, pattern recongnition algorithms.
   */
  class V0findingPerformanceEvaluationModule : public Module {

  public:

    V0findingPerformanceEvaluationModule();

    virtual ~V0findingPerformanceEvaluationModule();

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
    void fillTrackParams1DHistograms(const TrackFitResult* fitResult,
                                     MCParticleInfo mcParticleInfo); /**< fills err, resid and pull TH1F for each of the 5 track parameters*/

    void addEfficiencyPlots(TList* graphList = NULL);

    void addInefficiencyPlots(TList* graphList = NULL);

    void addPurityPlots(TList* graphList = NULL);


    bool isV0(const MCParticle& the_mcParticle);

    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */
    std::string m_MCParticlesName; /**< MCParticle StoreArray name */
    std::string m_V0sName; /**< MCTrackCand StoreArray name */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr; /**< pointer at root file used for storing histograms */

    /* list of histograms filled per MCParticle found in the event */

    TH1F* m_multiplicityV0s; /**< number of V0s per MCParticles*/
    TH1F* m_multiplicityMCParticles;  /**< number of MCParticles per fitted Track*/

    //error on track parameters
    TH1F* m_h1_vtxX_err; /**< vtx error*/
    TH1F* m_h1_vtxY_err; /**< vtx error*/
    TH1F* m_h1_vtxZ_err; /**< vtx error*/
    TH1F* m_h1_momX_err; /**< mom error*/
    TH1F* m_h1_momY_err; /**< mom error*/
    TH1F* m_h1_momZ_err; /**< mom error*/
    //residuals on track parameters
    TH1F* m_h1_vtxX_res; /**< vtx resid*/
    TH1F* m_h1_vtxY_res; /**< vtx resid*/
    TH1F* m_h1_vtxZ_res; /**< vtx resid*/
    TH1F* m_h1_momX_res; /**< mom resid*/
    TH1F* m_h1_momY_res; /**< mom resid*/
    TH1F* m_h1_momZ_res; /**< mom resid*/
    //pulls on track parameters
    TH1F* m_h1_vtxX_pll; /**< vtx pull*/
    TH1F* m_h1_vtxY_pll; /**< vtx pull*/
    TH1F* m_h1_vtxZ_pll; /**< vtx pull*/
    TH1F* m_h1_momX_pll; /**< mom pull*/
    TH1F* m_h1_momY_pll; /**< mom pull*/
    TH1F* m_h1_momZ_pll; /**< mom pull*/

    TH1F* m_h1_pValue;

    //histograms used for efficiency plots

    TH3F* m_h3_MCParticle;
    TH3F* m_h3_V0sPerMCParticle;

    //histograms used for purity plots
    TH3F* m_h3_V0s;
    TH3F* m_h3_MCParticlesPerV0;

  };
} // end of namespace


#endif /* V0DFINDINGPERFORMANCEEVALUAITONMODULE_H_ */
