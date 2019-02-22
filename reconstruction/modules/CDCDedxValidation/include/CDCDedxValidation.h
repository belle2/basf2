/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                   *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jitendra Kumar
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#pragma once

#include <framework/core/Module.h>
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/database/DBObjPtr.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/ECLCluster.h>

#include <reconstruction/dataobjects/CDCDedxTrack.h>
#include <reconstruction/dbobjects/CDCDedxRunGain.h>


#include "TH1D.h"
#include "TH2D.h"
#include "TF1.h"
#include "TList.h"
#include "TFile.h"
#include "TString.h"

namespace Belle2 {

  /** Extracts dE/dx information for calibration testing. Writes a ROOT file. */
  class CDCDedxValidationModule : public HistoModule {

  public:

    /** Default constructor */
    CDCDedxValidationModule();

    /** Destructor */
    virtual ~CDCDedxValidationModule() {}

    /** Initialize the module */
    virtual void initialize() override;

    /** This method is called for each run */
    virtual void beginRun() override;

    /** This method is called for each event. All processing of the event takes place in this method. */
    virtual void event() override;

    /** This method is called at the end of each run */
    virtual void endRun() override;

    /** End of the event processing. */
    virtual void terminate() override;


    /** Definition of relavant list of histograms */
    void DefineHistograms(TString level, Int_t iR);

    /** Fill defined histograms*/
    void FillHistograms(CDCDedxTrack* dedxTrack, const TrackFitResult* mTrack);

    /** Extra higher level histograms such as trends*/
    void ExtractHistograms(TString level);

    /** Apply standard selection of tracks*/
    Bool_t IsSelectedTrack(const TrackFitResult* mTrack);

    /** Set D0 cut window on tracks*/
    void setD0Cut(Double_t value) {fD0Window = value;}

    /** Set Z0 cut window on tracks*/
    void setZ0Cut(Double_t value) {fZ0Window = value;}


  private:

    StoreArray<CDCDedxTrack> m_cdcDedxTracks; /**< Store array for CDCDedxTrack */

    Double_t fD0Window; //*D0 window cut*/
    Double_t fZ0Window; //*Z0 window cut*/
    Int_t fnRunCounter; //*Total runs used*/
    Int_t fiRun; //*Current run counter*/

    Int_t    fnBinsdedx; //nbin of dedx range
    Double_t fnBinsdedxLE; //lowedge of dedx
    Double_t fnBinsdedxUE; //upedge of dedx

    Int_t fnRuns; //*Number of runs ref*/
    Int_t fCurrentRunNum; //*current run number*/
    Double_t fcRunGain; //* existing run gain*/
    Double_t fTrkEoverP; //* E/p ratio for cut*/

    TFile* fFileOutput = nullptr;     //! Write final objects to file for RG
    TList* fBasic = nullptr;   //! List of basic histos
    TList* fPRdEdx = nullptr;   //! List of per run dedx histos
    TList* fPRdEdxinP = nullptr; //!list per run dedx in P histos

    std::string fOutFileName; //name of output ROOT file
    std::string fCollType; //collision type

    std::vector<Double_t> TotMean;   //Mean of dedx by Fit
    std::vector<Double_t> TotMeanE;  //Mean Error of dedx by Fit
    std::vector<Double_t> TotSigma;  //Sigma of dedx by Fit
    std::vector<Double_t> TotSigmaE; //Sigma Error of dedx by Fit
    std::vector<Int_t> TotRunN;    //veector array of runs processed

    std::vector<TH1D*> hdEdx_PR; /**< histogram array per run */
    DBObjPtr<CDCDedxRunGain> m_DBRunGain; /**< Run gain DB object */

  };

} // Belle2 namespace
