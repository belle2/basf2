/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ewan Hill  (ehill@mail.ubc.ca)                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <reconstruction/dataobjects/CDCDedxTrack.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/database/DBObjPtr.h>

#include "TH1F.h"
#include "TString.h"
#include "TDirectory.h"
using std::vector;
using std::string;

#include <framework/dataobjects/EventT0.h>

namespace Belle2 {

  /**
   * This module to design collect CDC dEdx monitoring for DQM and only minimal information are
   * stored. All higher level calculation like fit etc is done using DQM analysis module.
   * Output of this module used as an input to DQM analysis.
   */

  class EventT0DQMModule : public HistoModule {

  public:

    /** Default constructor */
    EventT0DQMModule();

    /** Destructor */
    virtual ~EventT0DQMModule();

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

    StoreObjPtr<SoftwareTriggerResult> m_TrgResult; /**< Store array for Trigger selection */
    StoreObjPtr<EventT0> m_eventT0 ;  /**< Store array for event t0 */

    TH1F* h_eventT0_ECL_bhabha{nullptr}; /**< event t0 histogram for ECL and bha bha events */
    TH1F* h_eventT0_CDC_bhabha{nullptr}; /**< event t0 histogram for CDC and bha bha events */
    TH1F* h_eventT0_TOP_bhabha{nullptr}; /**< event t0 histogram for TOP and bha bha events */
    TH1F* h_eventT0_ECL_hadron{nullptr}; /**< event t0 histogram for ECL and hadronic events */
    TH1F* h_eventT0_CDC_hadron{nullptr}; /**< event t0 histogram for CDC and hadronic events */
    TH1F* h_eventT0_TOP_hadron{nullptr}; /**< event t0 histogram for TOP and hadronic events */

  };

} // Belle2 namespace
