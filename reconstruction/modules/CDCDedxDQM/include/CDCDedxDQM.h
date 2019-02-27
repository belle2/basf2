/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jitendra Kumar, Jake Bennett
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

#include "TH1D.h"
#include "TH2D.h"
#include "TString.h"
#include "TDirectory.h"
using std::vector;
using std::string;


namespace Belle2 {

  /**
   * This module to design collect CDC dEdx monitoring for DQM and only minimal information are
   * stored. All higher level calculation like fit etc is done using DQM analysis module.
   * Output of this module used as an input to DQM analysis.
   */

  class CDCDedxDQMModule : public HistoModule {

  public:

    /** Default constructor */
    CDCDedxDQMModule();

    /** Destructor */
    virtual ~CDCDedxDQMModule();

    /** Defination of histograms */
    virtual void defineHisto() override;

    /** Initialize the module */
    virtual void initialize() override;

    /** This method is called for each run */
    virtual void beginRun() override;

    /** This method is called for each event. All processing of the event
     * takes place in this method. */
    virtual void event() override;

    /** This method is called at the end of each run */
    virtual void endRun() override;

    /** End of the event processing. */
    virtual void terminate() override;



  private:

    StoreArray<CDCDedxTrack> m_cdcDedxTracks; /**< Store array for CDCDedxTrack */

    Int_t fCurrentEventNum; /**< variable to get run number */

    Bool_t isHadronfile; /**< Parameter-1 to switch binning */
    TString fCollType; /**< Parameter-2 to switch binning */

    TH1D* temp1D{nullptr}; /**< Dedx histogram per run */
    TH2D* temp2D{nullptr}; /**< Dedx vs P histogram per run */

    Int_t    nBinsdedx; /**< nbin of dedx range */
    Double_t nBinsdedxLE; /**< Lowedge of dedx */
    Double_t nBinsdedxUE; /**< Upedge of dedx */

    Int_t    nBinsP; /**< nbins of P range */
    Double_t nBinsPLE; /**< Lowedge of P range */
    Double_t nBinsPUE; /**< Upedge of P range */

  };

} // Belle2 namespace
