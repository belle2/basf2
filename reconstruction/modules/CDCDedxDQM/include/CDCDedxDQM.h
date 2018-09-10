/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <reconstruction/dataobjects/CDCDedxTrack.h>

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/database/DBObjPtr.h>

#include "TH1F.h"
#include "TH2F.h"
#include "TString.h"
#include "TDirectory.h"


//import some useful namespace
using std::cout;
using std::endl;
using std::vector;
using std::string;

namespace Belle2 {

  /** Extracts dE/dx information for calibration testing. Writes a ROOT file.
   */
  class CDCDedxDQMModule : public HistoModule {

  public:

    /** Default constructor */
    CDCDedxDQMModule();

    /** Destructor */
    virtual ~CDCDedxDQMModule();

    /** Defination of histograms */
    virtual void defineHisto();

    /** Initialize the module */
    virtual void initialize();

    /** This method is called for each run */
    virtual void beginRun();

    /** This method is called for each event. All processing of the event
     * takes place in this method. */
    virtual void event();

    /** This method is called at the end of each run */
    virtual void endRun();

    /** End of the event processing. */
    virtual void terminate();



  private:

    StoreArray<CDCDedxTrack> m_cdcDedxTracks; /**< Store array for CDCDedxTrack */

    Int_t fCurrentEventNum; /**< variable to get run number */

    Bool_t isHadronfile; /**< Parameter-1 to switch binning */
    TString fCollType; /**< Parameter-2 to switch binning */

    TH1F* temp1D; /**< Dedx histogram per run */
    TH2F* temp2D; /**< Dedx vs P histogram per run */

    Int_t    nBinsdedx; /**< nbin of dedx range */
    Double_t nBinsdedxLE; /**< Lowedge of dedx */
    Double_t nBinsdedxUE; /**< Upedge of dedx */

    Int_t    nBinsP; /**< nbins of P range */
    Double_t nBinsPLE; /**< Lowedge of P range */
    Double_t nBinsPUE; /**< Upedge of P range */

  };

} // Belle2 namespace
