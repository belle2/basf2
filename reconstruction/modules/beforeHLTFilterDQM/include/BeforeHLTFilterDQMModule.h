/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/bklm/BKLMHit1d.h>
#include <arich/dataobjects/ARICHHit.h>
#include <top/dataobjects/TOPDigit.h>

#include <framework/core/HistoModule.h>
#include <string>
#include <TH2S.h>

namespace Belle2 {

  class EventMetaData;
  class TRGSummary;
  class KLMDigit;

  /** DQM Module for basic detector quantities before the HLT filter*/
  class BeforeHLTFilterDQMModule : public HistoModule {

  public:

    /** Constructor */
    BeforeHLTFilterDQMModule();

    /* Destructor */
    virtual ~BeforeHLTFilterDQMModule();

    /** Module function initialize */
    void initialize() override final;
    /** Module function beginRun */
    void beginRun() override final;
    /** Module function event */
    void event() override final;

    /**
     * Defines Histograms
    */
    void defineHisto() override final;

  private:

    /** Name of the histogram directory in ROOT file */
    std::string m_histogramDirectoryName;

    StoreObjPtr<EventMetaData> m_eventMetaData; /**< event meta data*/
    StoreObjPtr<TRGSummary> m_trgSummary; /**< trg summary */

    //KLM stuff
    StoreArray<BKLMHit1d> m_BklmHit1ds; /**< BKLM hit 1D*/
    StoreArray<KLMDigit> m_KLMDigits; /**< KLM digits*/
    const EKLMElementNumbers* m_eklmElementNumbers; /**< EKLM Element numbers. */

    //ARICH stuff
    StoreArray<ARICHHit>m_ARICHHits; /**< ARICH hits*/

    //TOP stuff
    StoreArray<TOPDigit> m_topDigits; /**< collection of TOP digits */

    //histograms (all)
    //index: 0 = passive veto; 1 = active veto
    TH1F* m_PlaneBKLMPhi[2]; /**< BKLM phi plane integrated occupancy */;
    TH1F* m_PlaneBKLMZ[2]; /**< BKLM z plane integrated occupancy */;
    TH1F* m_PlaneEKLM[2]; /**< EKLM plane integrated occupancy */;
    TH1F* m_ARICHOccupancy[2]; /**< ARICH Digit Occupancy*/
    TH1F* m_topOccupancy[2]; /**< TOP occupancy (good hits only) */
  };

}


