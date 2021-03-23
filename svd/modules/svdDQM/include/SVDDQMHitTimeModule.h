/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <mdst/dataobjects/SoftwareTriggerResult.h>

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/TRGSummary.h>

#include "TH1F.h"

#include <svd/dataobjects/SVDEventInfo.h>
#include <svd/dataobjects/SVDCluster.h>
#include <framework/dataobjects/EventT0.h>

namespace Belle2 {

  /**
   * This module to design collect the svd hit time for different detectors trigger timing and physics processes.
   */

  class SVDDQMHitTimeModule : public HistoModule {

  public:

    /** Default constructor */
    SVDDQMHitTimeModule();

    /** Destructor */
    virtual ~SVDDQMHitTimeModule();

    /** Definition of histograms */
    virtual void defineHisto() override;

    /** Initialize the module */
    virtual void initialize() override;

    /** This method is called for each run */
    virtual void beginRun() override;

    /** This method is called for each event. All processing of the event
     * takes place in this method. */
    virtual void event() override;

  private:

    /** Name of the histogram directory in ROOT file */
    std::string m_histogramDirectoryName;

    /** SVDClusters StoreArray name */
    std::string m_storeSVDClustersName;
    /** SVDEventInfo StoreObjPtry name */
    std::string m_storeSVDEventInfoName;

    /** if TRUE: svdTime back in SVD time reference*/
    bool m_desynchSVDTime = false;

    StoreObjPtr<TRGSummary>    m_objTrgSummary;   /**< Trigger Summary data object */

    /** L1 timing source from getTimeType() in TRGSummary
     * See ETimingTYpe in mdst/dataobjects/include/TRGSummary.h (but information is obsolete...)
     * (2019/11/11) release-04-00-02, return values from getTimType() are
     * 0=ecltrg, 3=cdctrg, 5=delayed Bhabha, 7=random trigger from gdl, (toptrg is undefined yet) */
    int m_L1TimingSrc = -1;

    StoreObjPtr<SoftwareTriggerResult> m_TrgResult; /**< Trigger selection data object */
    StoreObjPtr<EventT0> m_eventT0 ;  /**< EventT0 data object */
    StoreObjPtr<SVDEventInfo> m_svdEventInfo ;  /**< SVDEventInfo data object */
    StoreArray<SVDCluster> m_clusters ;  /**< Store array for clusters*/

    TH1F* m_l3v_bhabha_L1_ECLTRG{nullptr}; /**< svd time histogram for bhabha events wrt the ECL trigger time*/
    TH1F* m_l3vEvtT0_bhabha_L1_ECLTRG{nullptr}; /**< svd time histogram for bhabha events wrt the ECL trigger time*/
    TH1F* m_l3v_hadron_L1_ECLTRG{nullptr}; /**< svd time histogram for hadronic events wrt the ECL trigger time */
    TH1F* m_l3vEvtT0_hadron_L1_ECLTRG{nullptr}; /**< svd time histogram for hadronic events wrt the ECL trigger time */
    TH1F* m_l3v_mumu_L1_ECLTRG{nullptr};   /**< svd time histogram for mu mu events wrt the ECL trigger time*/
    TH1F* m_l3vEvtT0_mumu_L1_ECLTRG{nullptr};   /**< svd time histogram for mu mu events wrt the ECL trigger time*/

    TH1F* m_l3v_bhabha_L1_CDCTRG{nullptr}; /**< svd time histogram for bhabha events wrt the CDC trigger time*/
    TH1F* m_l3vEvtT0_bhabha_L1_CDCTRG{nullptr}; /**< svd time histogram for bhabha events wrt the CDC trigger time*/
    TH1F* m_l3v_hadron_L1_CDCTRG{nullptr}; /**< svd time histogram for hadronic events wrt the CDC trigger time */
    TH1F* m_l3vEvtT0_hadron_L1_CDCTRG{nullptr}; /**< svd time histogram for hadronic events wrt the CDC trigger time */
    TH1F* m_l3v_mumu_L1_CDCTRG{nullptr};   /**< svd time histogram for mu mu events wrt the CDC trigger time*/
    TH1F* m_l3vEvtT0_mumu_L1_CDCTRG{nullptr};   /**< svd time histogram for mu mu events wrt the CDC trigger time*/

  };

} // Belle2 namespace
