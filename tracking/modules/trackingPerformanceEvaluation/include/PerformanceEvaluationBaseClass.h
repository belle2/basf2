/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TFile.h>
#include <TList.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>

// forward declarations
class TFile;

namespace Belle2 {

  /** This module takes the MCParticles, the genfit Tracks, the genfit TrackCand,
   *  and the MCTrackCands input and produce a root file containing various histograms
   *  showing the performance of the tracking package: fitter, pattern recongnition algorithms.
   */
  class PerformanceEvaluationBaseClass  {

  public:

    PerformanceEvaluationBaseClass();

    virtual ~PerformanceEvaluationBaseClass();

    TList* m_histoList = nullptr; /**< List of performance-evaluation histograms */

    TList* m_histoList_multiplicity = nullptr; /**< List of multiplicity histograms */
    TList* m_histoList_evtCharacterization = nullptr; /**< List of event-characterization histograms */
    TList* m_histoList_trkQuality = nullptr; /**< List of track-quality histograms */
    TList* m_histoList_firstHit = nullptr; /**< List of first-hit-position histograms */
    TList* m_histoList_pr = nullptr; /**< List of pattern-recognition histograms */
    TList* m_histoList_fit = nullptr; /**< List of track-fit histograms */
    TList* m_histoList_efficiency = nullptr; /**< List of efficiency histograms */
    TList* m_histoList_purity = nullptr; /**< List of purity histograms */
    TList* m_histoList_others = nullptr; /**< List of other performance-evaluation histograms */

    //list of functions to create histograms:
    /// Create a 1D histogram and add it to the TList of 1D-histograms
    TH1F* createHistogram1D(const char* name, const char* title,
                            Int_t nbins, Double_t min, Double_t max,
                            const char* xtitle, TList* histoList = nullptr);

    /// Create a 1D histogram and add it to the TList of 1D-histograms
    TH1F* createHistogram1D(const char* name, const char* title,
                            Int_t nbins, Double_t* bins,
                            const char* xtitle, TList* histoList = nullptr);

    /// Create a 2D histogram and add it to the TList of 2D-histograms
    TH2F* createHistogram2D(const char* name, const char* title,
                            Int_t nbinsX, Double_t minX, Double_t maxX, const char* titleX,
                            Int_t nbinsY, Double_t minY, Double_t maxY, const char* titleY,
                            TList* histoList = nullptr); /**< Create 2D histogram */

    /// Create a 2D histogram and add it to the TList of 2D-histograms
    TH2F* createHistogram2D(const char* name, const char* title,
                            Int_t nbinsX, Double_t* binsX, const char* titleX,
                            Int_t nbinsY, Double_t* binsY, const char* titleY,
                            TList* histoList = nullptr);

    /// Create a 3D histogram and add it to the TList of 3D-histograms
    TH3F* createHistogram3D(const char* name, const char* title,
                            Int_t nbinsX, Double_t minX, Double_t maxX, const char* titleX,
                            Int_t nbinsY, Double_t minY, Double_t maxY, const char* titleY,
                            Int_t nbinsZ, Double_t minZ, Double_t maxZ, const char* titleZ,
                            TList* histoList = nullptr);

    /// Create a 3D histogram and add it to the TList of 3D-histograms
    TH3F* createHistogram3D(const char* name, const char* title,
                            Int_t nbinsX, Double_t* binsX, const char* titleX,
                            Int_t nbinsY, Double_t* binsY, const char* titleY,
                            Int_t nbinsZ, Double_t* binsZ, const char* titleZ,
                            TList* histoList = nullptr);

    /// Make a copy of a 1D histogram and add it to the TList of 1D-histograms
    TH1* duplicateHistogram(const char* newname, const char* newtitle,
                            TH1* h, TList* histoList = nullptr);


    /// Make a new 1D histogram from the ratio of two others and add it to the TList of 1D-histograms
    TH1F* createHistogramsRatio(const char* name, const char* title,
                                TH1* hNum, TH1* hDen, bool isEffPlot,
                                int axisRef);



    /// Create pt-, theta- and phi-efficiency 1D histograms and add them to the TList of 1D-histograms
    void addEfficiencyPlots(TList* graphList = nullptr, TH3F* h3_xPerMCParticle = nullptr, TH3F* h3_MCParticle = nullptr);
    /// Create pt-, theta- and phi-inefficiency 1D histograms and add them to the TList of 1D-histograms
    void addInefficiencyPlots(TList* graphList = nullptr, TH3F* h3_xPerMCParticle = nullptr,
                              TH3F* h3_MCParticle = nullptr);
    /// Create pt-, theta- and phi-purity 1D histograms and add them to the TList of 1D-histograms
    void addPurityPlots(TList* graphList = nullptr, TH3F* h3_xPerMCParticle = nullptr, TH3F* h3_MCParticle = nullptr);

    /// Create a 1D efficiency histogram and add it to the TList of 1D-histograms
    TH1F* effPlot1D(TH1F* h1_den, TH1F* h1_num,
                    const char* name, const char* title, bool geo_accettance, TList* histoList = nullptr);

    /// Create a 1D efficiency histogram and add it to the TList of 1D-histograms
    TH1F* effPlot1D(TH1F* h1_MC, TH1F* h1_RecoTrack, TH1F* h1_Track,
                    const char* name, const char* title, TList* histoList = nullptr);

    /// Create a 2D efficiency histogram and add it to the TList of 2D-histograms
    TH2F* effPlot2D(TH2F* h2_den, TH2F* h2_num,
                    const char* name, const char* title, bool geo_accettance, TList* histoList = nullptr);

    /// Create a 2D efficiency histogram and add it to the TList of 2D-histograms
    TH2F* effPlot2D(TH2F* h2_MC, TH2F* h2_RecoTrack, TH2F* h2_Track,
                    const char* name, const char* title, TList* histoList);

    /// Create a 1D efficiency histogram for geometric acceptance and add it to the TList of 1D-histograms
    TH1F* geoAcc1D(TH1F* h1_den, TH1F* h1_num,
                   const char* name, const char* title, TList* histoList = nullptr);

    /// Create a 2D efficiency histogram for geometric acceptance and add it to the TList of 2D-histograms
    TH2F* geoAcc2D(TH2F* h2_den, TH2F* h2_num,
                   const char* name, const char* title, TList* histoList = nullptr);

    /// Create a 1D efficiency histogram for V0 finding and add it to the TList of 1D-histograms
    TH1F* V0FinderEff(TH1F* h1_dau0, TH1F* h1_dau1, TH1F* h1_Mother,
                      const char* name, const char* title, TList* histoList = nullptr);


    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr = nullptr; /**< pointer at root file used for storing histograms */

  };
} // end of namespace
