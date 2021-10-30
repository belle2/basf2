/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>

#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <reconstruction/dataobjects/CDCDedxTrack.h>
#include <reconstruction/dbobjects/CDCDedxRunGain.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>

#include "TMath.h"
#include "TH1D.h"
#include "TH2D.h"

namespace Belle2 {

  /**
   * This module to design collect CDC dEdx monitoring
   * for DQM and only minimal information are stored.
   * All higher level calculation like fit etc is done
   * using DQM analysis module. Output of this module
   * used as an input to DQM analysis.
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

    /** get index val of cdc wire/layer/parameters */
    double getIndexVal(int iWire, TString what);


  private:

    DBObjPtr<CDCDedxRunGain> m_DBRunGain; /**< Run gain DB object */

    StoreObjPtr<EventMetaData> m_MetaDataPtr; /**< Store array for metadata info*/
    StoreObjPtr<SoftwareTriggerResult> m_TrgResult; /**< Store array for Trigger selection */
    StoreArray<CDCDedxTrack> m_cdcDedxTracks; /**< Store array for CDCDedxTrack */

    Int_t m_exp{ -1}; /**< exp number */
    Int_t m_run{ -1}; /**< run number */
    Int_t m_event{ -1}; /**< number of event */

    Int_t m_nEvt{0}; /**< accepted events */
    Int_t m_nBEvt{ 0}; /**< bhabha events  */
    Int_t m_nHEvt{0}; /**< hadron events  */

    Double_t m_rungain{ -99.0}; /**< previous rungain */
    std::vector <double> m_adc[14336]; /**< adc per wire for wire status */

    TH1D* hMeta{nullptr}; /**< metadata */
    TH1D* hdEdx{nullptr}; /**< dedx */
    TH2D* hdEdxvsP{nullptr}; /**< dedx vs p*/
    TH2D* hdEdxvsPhi{nullptr}; /**< dedx vs phi */
    TH2D* hdEdxvsCosth{nullptr}; /**< dedx vs costh */
    TH2D* hdEdxvsEvt{nullptr}; /**< dedx vs event*/
    TH2F* hWireStatus{nullptr}; /**< dead wire status */
    TH2F* hWires{nullptr}; /**< all wire mapping  */

  };
} // Belle2 namespace
