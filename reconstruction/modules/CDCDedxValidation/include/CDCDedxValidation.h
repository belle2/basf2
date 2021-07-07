/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/database/DBObjPtr.h>

#include <mdst/dataobjects/TrackFitResult.h>

#include <reconstruction/dataobjects/CDCDedxTrack.h>
#include <reconstruction/dbobjects/CDCDedxRunGain.h>


#include "TH1D.h"
#include "TF1.h"
#include "TList.h"
#include "TFile.h"
#include "TString.h"

namespace Belle2 {

  /**
   * First version commited on Feb 21 2019
   * Extracts dE/dx information for validation and writes a ROOT file.
   * Input to this module is bhabha or radbhabha or hadron skimmed files only.
   * See reconstruction/examples/ExtractCDCdEdxValidation.C to extract output file and make nice plots.
   * See reconstruction/examples/runCDCdEdxValidation.py to run this module.
   */
  class CDCDedxValidationModule : public HistoModule {

  public:

    /**
     * Default constructor
     * contain list of members with initial values
     */
    CDCDedxValidationModule();

    /**
     * Default destructor
     */
    virtual ~CDCDedxValidationModule() {}

    /**
     * Initialize
     * This is inherited from base class
     */
    virtual void initialize() override;

    /**
     * Fuction to execute each run
     * This is inherited from base class
     */
    virtual void beginRun() override;

    /**
     * fuction to execute event (event by event)
     * This is inherited from base class
     */
    virtual void event() override;

    /**
     * fuction is called after each event
     * This is inherited from base class
     */
    virtual void endRun() override;

    /**
     * Terminate after all data processed
     * This is inherited from base class
     */
    virtual void terminate() override;


    /**
     * Defination of histograms
     * This contain a list of histogram for validation
     */
    void DefineHistograms(TString level, Int_t iR);

    /**
     * Filling histograms
     * This will fill histogram defined histograms in above function
     */
    void FillHistograms(CDCDedxTrack* dedxTrack, const TrackFitResult* mTrack);

    /**
     * Extrating histogram and some calucation
     * Higher level histograms are filled after each run or full processing
     */
    void ExtractHistograms(TString level);

    /**
     * Track selection
     * A clean way to impliment selections on tracks (so far few only)
     */
    Bool_t IsSelectedTrack(const TrackFitResult* mTrack);

    /**
     * d0 Selection
     * set/change d0 while executing this module from external script
     */
    void setD0Cut(Double_t value) {fD0Window = value;}

    /**
     * z0 Selection
     * set/change z0 while executing this module from external script
     */
    void setZ0Cut(Double_t value) {fZ0Window = value;}


  private:

    /**
     * Data members for objects, cuts and others
     * @param outputFileName    Name of output file (default is CDCdEdxValidation.root)
     * @param SampleType        Select type of skim files (bhabha or radbhabha or hadron)
     * @param fnRuns            an Upper bound to total runs as an input
     */

    StoreArray<CDCDedxTrack> m_cdcDedxTracks; /**< Store array for CDCDedxTrack */

    Double_t fD0Window; /**< d0 window cut */
    Double_t fZ0Window; /**< z0 window cut */
    Int_t fnRunCounter; /**< Total runs used counter */
    Int_t fiRun; /**< Current run number */

    Int_t    fnBinsdedx; /**< nbin of dedx range */
    Double_t fnBinsdedxLE; /**< low edge of dedx */
    Double_t fnBinsdedxUE; /**< up edge of dedx */

    Int_t fnRuns; /**< Number of runs ref */
    Int_t fCurrentRunNum; /**< current run number */
    Double_t fcRunGain; /**< existing run gain */
    Double_t fTrkEoverP; /**< E/p ratio for cut */

    TFile* fFileOutput = nullptr;     /**< Write final objects to file for RG */
    TList* fBasic = nullptr;   /**< List of basic histos */
    TList* fPRdEdx = nullptr;   /**< List of per run dedx histos */
    TList* fPRdEdxinP = nullptr;/**< list per run dedx in P histos */

    std::string fOutFileName; /**< name of output ROOT file */
    std::string fCollType; /**< collision type */

    std::vector<Double_t> TotMean;   /**< Mean of dedx by Fit */
    std::vector<Double_t> TotMeanE;  /**< Mean Error of dedx by Fit */
    std::vector<Double_t> TotSigma;  /**< Sigma of dedx by Fit */
    std::vector<Double_t> TotSigmaE; /**< Sigma Error of dedx by Fit */
    std::vector<Int_t> TotRunN;    /**< veector array of runs processed */

    std::vector<TH1D*> hdEdx_PR; /**< histogram array per run */
    DBObjPtr<CDCDedxRunGain> m_DBRunGain; /**< Run gain DB object */

  };

}
