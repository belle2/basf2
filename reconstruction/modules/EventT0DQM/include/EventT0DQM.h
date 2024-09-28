/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <mdst/dataobjects/SoftwareTriggerResult.h>

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/TRGSummary.h>

#include <framework/dataobjects/EventT0.h>

#include <TH1F.h>
#include <TH1D.h>
#include <TDirectory.h>

namespace Belle2 {

  /**
   * This module to design collect the event t0 values base on different detectors and physics processes.
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

  private:


    StoreObjPtr<TRGSummary>    m_objTrgSummary;   /**< Trigger Summary data object */

    StoreObjPtr<SoftwareTriggerResult> m_TrgResult; /**< Store array for Trigger selection */
    StoreObjPtr<EventT0> m_eventT0 ;  /**< Store array for event t0 */

    // ECL trigger based EventT0 histograms
    TH1F* m_histEventT0_ECL_bhabha_L1_ECLTRG{nullptr}; /**< event t0 histogram for ECL, HLT bha bha events, L1 time by ECL trigger */
    TH1F* m_histEventT0_CDC_bhabha_L1_ECLTRG{nullptr}; /**< event t0 histogram for CDC, HLT bha bha events, L1 time by ECL trigger */
    TH1F* m_histEventT0_TOP_bhabha_L1_ECLTRG{nullptr}; /**< event t0 histogram for TOP, HLT bha bha events, L1 time by ECL trigger */
    TH1F* m_histEventT0_SVD_bhabha_L1_ECLTRG{nullptr}; /**< event t0 histogram for SVD, HLT bha bha events, L1 time by ECL trigger */

    TH1F* m_histEventT0_ECL_hadron_L1_ECLTRG{nullptr}; /**< event t0 histogram for ECL, HLT hadronic events, L1 time by ECL trigger */
    TH1F* m_histEventT0_CDC_hadron_L1_ECLTRG{nullptr}; /**< event t0 histogram for CDC, HLT hadronic events, L1 time by ECL trigger */
    TH1F* m_histEventT0_TOP_hadron_L1_ECLTRG{nullptr}; /**< event t0 histogram for TOP, HLT hadronic events, L1 time by ECL trigger */
    TH1F* m_histEventT0_SVD_hadron_L1_ECLTRG{nullptr}; /**< event t0 histogram for SVD, HLT hadronic events, L1 time by ECL trigger */

    TH1F* m_histEventT0_ECL_mumu_L1_ECLTRG{nullptr};   /**< event t0 histogram for ECL, HLT mu mu events, L1 time by ECL trigger */
    TH1F* m_histEventT0_CDC_mumu_L1_ECLTRG{nullptr};   /**< event t0 histogram for CDC, HLT mu mu events, L1 time by ECL trigger */
    TH1F* m_histEventT0_TOP_mumu_L1_ECLTRG{nullptr};   /**< event t0 histogram for TOP, HLT mu mu events, L1 time by ECL trigger */
    TH1F* m_histEventT0_SVD_mumu_L1_ECLTRG{nullptr};   /**< event t0 histogram for SVD, HLT mu mu events, L1 time by ECL trigger */

    // CDC trigger based EventT0 histograms
    TH1F* m_histEventT0_ECL_bhabha_L1_CDCTRG{nullptr}; /**< event t0 histogram for ECL, HLT bha bha events, L1 time by CDC trigger */
    TH1F* m_histEventT0_CDC_bhabha_L1_CDCTRG{nullptr}; /**< event t0 histogram for CDC, HLT bha bha events, L1 time by CDC trigger */
    TH1F* m_histEventT0_TOP_bhabha_L1_CDCTRG{nullptr}; /**< event t0 histogram for TOP, HLT bha bha events, L1 time by CDC trigger */
    TH1F* m_histEventT0_SVD_bhabha_L1_CDCTRG{nullptr}; /**< event t0 histogram for SVD, HLT bha bha events, L1 time by CDC trigger */

    TH1F* m_histEventT0_ECL_hadron_L1_CDCTRG{nullptr}; /**< event t0 histogram for ECL, HLT hadronic events, L1 time by CDC trigger */
    TH1F* m_histEventT0_CDC_hadron_L1_CDCTRG{nullptr}; /**< event t0 histogram for CDC, HLT hadronic events, L1 time by CDC trigger */
    TH1F* m_histEventT0_TOP_hadron_L1_CDCTRG{nullptr}; /**< event t0 histogram for TOP, HLT hadronic events, L1 time by CDC trigger */
    TH1F* m_histEventT0_SVD_hadron_L1_CDCTRG{nullptr}; /**< event t0 histogram for SVD, HLT hadronic events, L1 time by CDC trigger */

    TH1F* m_histEventT0_ECL_mumu_L1_CDCTRG{nullptr};   /**< event t0 histogram for ECL, HLT mu mu events, L1 time by CDC trigger */
    TH1F* m_histEventT0_CDC_mumu_L1_CDCTRG{nullptr};   /**< event t0 histogram for CDC, HLT mu mu events, L1 time by CDC trigger */
    TH1F* m_histEventT0_TOP_mumu_L1_CDCTRG{nullptr};   /**< event t0 histogram for TOP, HLT mu mu events, L1 time by CDC trigger */
    TH1F* m_histEventT0_SVD_mumu_L1_CDCTRG{nullptr};   /**< event t0 histogram for SVD, HLT mu mu events, L1 time by CDC trigger */

    // TOP trigger based EventT0 histograms
    TH1F* m_histEventT0_ECL_bhabha_L1_TOPTRG{nullptr}; /**< event t0 histogram for ECL, HLT bha bha events, L1 time by TOP trigger */
    TH1F* m_histEventT0_CDC_bhabha_L1_TOPTRG{nullptr}; /**< event t0 histogram for CDC, HLT bha bha events, L1 time by TOP trigger */
    TH1F* m_histEventT0_TOP_bhabha_L1_TOPTRG{nullptr}; /**< event t0 histogram for TOP, HLT bha bha events, L1 time by TOP trigger */
    TH1F* m_histEventT0_SVD_bhabha_L1_TOPTRG{nullptr}; /**< event t0 histogram for SVD, HLT bha bha events, L1 time by TOP trigger */

    TH1F* m_histEventT0_ECL_hadron_L1_TOPTRG{nullptr}; /**< event t0 histogram for ECL, HLT hadronic events, L1 time by TOP trigger */
    TH1F* m_histEventT0_CDC_hadron_L1_TOPTRG{nullptr}; /**< event t0 histogram for CDC, HLT hadronic events, L1 time by TOP trigger */
    TH1F* m_histEventT0_TOP_hadron_L1_TOPTRG{nullptr}; /**< event t0 histogram for TOP, HLT hadronic events, L1 time by TOP trigger */
    TH1F* m_histEventT0_SVD_hadron_L1_TOPTRG{nullptr}; /**< event t0 histogram for SVD, HLT hadronic events, L1 time by TOP trigger */

    TH1F* m_histEventT0_ECL_mumu_L1_TOPTRG{nullptr};   /**< event t0 histogram for ECL, HLT mu mu events, L1 time by TOP trigger */
    TH1F* m_histEventT0_CDC_mumu_L1_TOPTRG{nullptr};   /**< event t0 histogram for CDC, HLT mu mu events, L1 time by TOP trigger */
    TH1F* m_histEventT0_TOP_mumu_L1_TOPTRG{nullptr};   /**< event t0 histogram for TOP, HLT mu mu events, L1 time by TOP trigger */
    TH1F* m_histEventT0_SVD_mumu_L1_TOPTRG{nullptr};   /**< event t0 histogram for SVD, HLT mu mu events, L1 time by TOP trigger */

    /// EventT0 algorithms for which to calculate fractions of abundance
    const char* c_eventT0Algorithms[6] = {"ECL", "SVD", "CDC hit based", "CDC full grid #chi^{2}", "TOP", "Any"};
    /// Fraction of events with EventT0 from a given algorithm, HLT hadronic events, L1 time by ECL trigger
    TH1D* m_histAlgorithmSourceFractionsHadronL1ECLTRG{nullptr};
    /// Fraction of events with EventT0 from a given algorithm, HLT hadronic events, L1 time by CDC trigger
    TH1D* m_histAlgorithmSourceFractionsHadronL1CDCTRG{nullptr};
    /// Fraction of events with EventT0 from a given algorithm, HLT hadronic events, L1 time by TOP trigger
    TH1D* m_histAlgorithmSourceFractionsHadronL1TOPTRG{nullptr};
    /// Fraction of events with EventT0 from a given algorithm, HLT bhabha events, L1 time by ECL trigger
    TH1D* m_histAlgorithmSourceFractionsBhaBhaL1ECLTRG{nullptr};
    /// Fraction of events with EventT0 from a given algorithm, HLT bhabha events, L1 time by CDC trigger
    TH1D* m_histAlgorithmSourceFractionsBhaBhaL1CDCTRG{nullptr};
    /// Fraction of events with EventT0 from a given algorithm, HLT bhabha events, L1 time by TOP trigger
    TH1D* m_histAlgorithmSourceFractionsBhaBhaL1TOPTRG{nullptr};
    /// Fraction of events with EventT0 from a given algorithm, HLT mumu events, L1 time by ECL trigger
    TH1D* m_histAlgorithmSourceFractionsMuMuL1ECLTRG{nullptr};
    /// Fraction of events with EventT0 from a given algorithm, HLT mumu events, L1 time by CDC trigger
    TH1D* m_histAlgorithmSourceFractionsMuMuL1CDCTRG{nullptr};
    /// Fraction of events with EventT0 from a given algorithm, HLT mumu events, L1 time by TOP trigger
    TH1D* m_histAlgorithmSourceFractionsMuMuL1TOPTRG{nullptr};

    /// Fill fraction histogram with values
    void fillHistogram(TH1D* hist, const bool hasAnyT0, const bool hasECLT0, const bool hasSVDT0,
                       const bool hasCDCHitT0, const bool hasCDCGridT0, const bool hasTOPT0);


  };

} // Belle2 namespace
