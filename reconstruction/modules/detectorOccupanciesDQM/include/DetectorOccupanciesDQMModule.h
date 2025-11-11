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
#include <framework/core/HistoModule.h>

#include <string>
#include <TH1F.h>
#include <TProfile.h>

namespace Belle2 {
  class EventMetaData;
  class TRGSummary;
  class BKLMHit1d;
  class KLMDigit;
  class EKLMElementNumbers;
  class ARICHHit;
  class TOPDigit;
  class ECLCalDigit;


  /** DQM Module for basic detector quantities before the HLT filter*/
  class DetectorOccupanciesDQMModule : public HistoModule {

  public:

    /** Constructor */
    DetectorOccupanciesDQMModule();

    /* Destructor */
    virtual ~DetectorOccupanciesDQMModule();

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

    //ECL stuff
    double m_eclEnergyThr; /**< Energy threshold (in MeV) for ECL occupancy histogram */
    StoreArray<ECLCalDigit> m_eclCalDigits; /**< collection of ECL digits */

    //histograms (all)
    //index: 0 = passive veto; 1 = active veto
    TH1F* m_BKLM_PlanePhi_Occupancy[2]; /**< BKLM phi plane integrated occupancy */;
    TH1F* m_BKLM_PlaneZ_Occupancy[2]; /**< BKLM z plane integrated occupancy */;
    TH1F* m_EKLM_Plane_Occupancy[2]; /**< EKLM plane integrated occupancy */;
    TH1F* m_BKLM_TimeRPC[2]; /**< RPC Hit Time */;
    TH1F* m_BKLM_TimeScintillator[2]; /**< BKLM Scintillator Hit Time */;
    TH1F* m_EKLM_TimeScintillator[2]; /**< EKLM  Scintillator Hit Time */;
    TH1F* m_ARICH_Occupancy[2]; /**< ARICH Digit Occupancy*/
    TH1F* m_TOP_Occupancy[2]; /**< TOP occupancy (good hits only) */
    TProfile* m_ECL_Occupancy[2]; /**< ECL occupancy (hits above 5 MeV) */
  };

}


