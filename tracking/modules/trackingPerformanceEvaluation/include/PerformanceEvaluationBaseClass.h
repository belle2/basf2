/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef PERFORMANCEEVALUATIONBASECLASS_H_
#define PERFORMANCEEVALUATIONBASECLASS_H_

#include <framework/core/Module.h>
#include <TDirectory.h>
#include <TCollection.h>
#include <TTree.h>
#include <TFile.h>
#include <TList.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>

// forward declarations
class TTree;
class TFile;

namespace Belle2 {

  template< class T >
  class StoreArray;

  /** This module takes the MCParticles, the genfit Tracks, the genfit TrackCand,
   *  and the MCTrackCands input and produce a root file containing various histograms
   *  showing the performance of the tracking package: fitter, pattern recongnition algorithms.
   */
  class PerformanceEvaluationBaseClass  {

  public:

    PerformanceEvaluationBaseClass();

    virtual ~PerformanceEvaluationBaseClass();

    TList* m_histoList = nullptr;

    TList* m_histoList_multiplicity = nullptr;
    TList* m_histoList_evtCharacterization = nullptr;
    TList* m_histoList_trkQuality = nullptr;
    TList* m_histoList_firstHit = nullptr;
    TList* m_histoList_pr = nullptr;
    TList* m_histoList_fit = nullptr;
    TList* m_histoList_efficiency = nullptr;
    TList* m_histoList_purity = nullptr;
    TList* m_histoList_others = nullptr;

    //list of functions to create histograms:
    TH1F* createHistogram1D(const char* name, const char* title,
                            Int_t nbins, Double_t min, Double_t max,
                            const char* xtitle, TList* histoList = NULL);  /**< thf */

    TH1F* createHistogram1D(const char* name, const char* title,
                            Int_t nbins, Double_t* bins,
                            const char* xtitle, TList* histoList = NULL);

    TH2F* createHistogram2D(const char* name, const char* title,
                            Int_t nbinsX, Double_t minX, Double_t maxX, const char* titleX,
                            Int_t nbinsY, Double_t minY, Double_t maxY, const char* titleY,
                            TList* histoList = NULL);  /**< thf */

    TH2F* createHistogram2D(const char* name, const char* title,
                            Int_t nbinsX, Double_t* binsX, const char* titleX,
                            Int_t nbinsY, Double_t* binsY, const char* titleY,
                            TList* histoList = NULL);

    TH3F* createHistogram3D(const char* name, const char* title,
                            Int_t nbinsX, Double_t minX, Double_t maxX, const char* titleX,
                            Int_t nbinsY, Double_t minY, Double_t maxY, const char* titleY,
                            Int_t nbinsZ, Double_t minZ, Double_t maxZ, const char* titleZ,
                            TList* histoList = NULL);  /**< thf */

    TH3F* createHistogram3D(const char* name, const char* title,
                            Int_t nbinsX, Double_t* binsX, const char* titleX,
                            Int_t nbinsY, Double_t* binsY, const char* titleY,
                            Int_t nbinsZ, Double_t* binsZ, const char* titleZ,
                            TList* histoList = NULL);  /**< thf */

    TH1* duplicateHistogram(const char* newname, const char* newtitle,
                            TH1* h, TList* histoList = NULL);  /**< thf */


    TH1F* createHistogramsRatio(const char* name, const char* title,
                                TH1* hNum, TH1* hDen, bool isEffPlot,
                                int axisRef);  /**< thf */



    void addEfficiencyPlots(TList* graphList = NULL, TH3F* h3_xPerMCParticle = NULL, TH3F* h3_MCParticle = NULL);  /**< efficiency */
    void addInefficiencyPlots(TList* graphList = NULL, TH3F* h3_xPerMCParticle = NULL,
                              TH3F* h3_MCParticle = NULL);  /**< inefficiency */
    void addPurityPlots(TList* graphList = NULL, TH3F* h3_xPerMCParticle = NULL, TH3F* h3_MCParticle = NULL);  /**< purity */

    TH1F* effPlot1D(TH1F* h1_den, TH1F* h1_num,
                    const char* name, const char* title, bool geo_accettance, TList* histoList = NULL);

    TH1F* effPlot1D(TH1F* h1_MC, TH1F* h1_RecoTrack, TH1F* h1_Track,
                    const char* name, const char* title, TList* histoList = NULL);

    TH2F* effPlot2D(TH2F* h2_den, TH2F* h2_num,
                    const char* name, const char* title, bool geo_accettance, TList* histoList = NULL);

    TH2F* effPlot2D(TH2F* h2_MC, TH2F* h2_RecoTrack, TH2F* h2_Track,
                    const char* name, const char* title, TList* histoList);

    TH1F* geoAcc1D(TH1F* h1_den, TH1F* h1_num,
                   const char* name, const char* title, TList* histoList = NULL);

    TH2F* geoAcc2D(TH2F* h2_den, TH2F* h2_num,
                   const char* name, const char* title, TList* histoList = NULL);

    TH1F* V0FinderEff(TH1F* h1_dau0, TH1F* h1_dau1, TH1F* h1_Mother,
                      const char* name, const char* title, TList* histoList = NULL);


    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr = nullptr; /**< pointer at root file used for storing histograms */

  };
} // end of namespace


#endif /* PERFORMANCEEVALUAITONBASECLASS_H_ */
