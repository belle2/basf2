/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDHistograms.h>
#include <svd/dataobjects/SVDSummaryPlots.h>
#include <framework/dataobjects/EventMetaData.h>

#include <mdst/dataobjects/SoftwareTriggerResult.h>

#include <svd/calibration/SVDNoiseCalibrations.h>


#include <string>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>

namespace Belle2 {

  /** The SVD OccupancyAnalysis Module
   *
   */

  class SVDOccupancyAnalysisModule : public Module {

  public:

    SVDOccupancyAnalysisModule();

    virtual ~SVDOccupancyAnalysisModule();
    virtual void initialize() override;
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    /* user-defined parameters */
    std::string m_rootFileName = "";   /**< root file name */
    std::string m_ShaperDigitName = "SVDShaperDigits";   /**< ShaperDigit StoreArray name */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr = nullptr; /**< pointer at root file used for storing histograms */

    float m_group = 10000; /**< number of events to compute occupancy for occ VS time*/
    float m_minZS = 3; /**< minimum zero suppresion cut*/
    float m_maxZS = 6; /**< max zero suppression cut*/
    int m_pointsZS = 7; /**< number of steps for different ZS cuts*/
    bool m_FADCmode = true; /**< if true, ZS done with same algorithm as on FADC*/

  private:

    /// Store Object for reading the trigger decision.
    StoreObjPtr<SoftwareTriggerResult> m_resultStoreObjectPointer;
    bool m_skipRejectedEvents = false; /**< if true skip events rejected by HLT*/

    int m_nEvents = 0; /**< number of events*/
    StoreArray<SVDShaperDigit> m_svdShapers; /**<SVDShaperDigit StoreArray*/
    StoreObjPtr<EventMetaData> m_eventMetaData; /**<Event Meta Data StoreObjectPointer*/

    SVDNoiseCalibrations m_NoiseCal; /**<SVDNoise calibrations db object*/


    //! IDs of all SVD Modules to iterate over
    std::vector<VxdID> m_SVDModules;

    //layer summary
    float m_distr_Nbins = 10000; /**< number of bins of occupancy distributions*/
    float m_distr_min = 0; /**< min of occupancy distributions*/
    float m_distr_max = 100; /**< max of occupancy distributions*/

    TH1F* m_occ_L3U = nullptr; /**< occupancy distribution for L3 U-side sensors*/
    TH1F* m_occ_L3V = nullptr; /**< occupancy distribution for L3 V-side sensors*/
    TH1F* m_occ_L4U = nullptr; /**< occupancy distribution for L4 U-side sensors*/
    TH1F* m_occ_L4V = nullptr; /**< occupancy distribution for L4 V-side sensors*/
    TH1F* m_occ_L5U = nullptr; /**< occupancy distribution for L5 U-side sensors*/
    TH1F* m_occ_L5V = nullptr; /**< occupancy distribution for L5 V-side sensors*/
    TH1F* m_occ_L6U = nullptr; /**< occupancy distribution for L6 U-side sensors*/
    TH1F* m_occ_L6V = nullptr; /**< occupancy distribution for L6 V-side sensors*/

    //SHAPER
    SVDSummaryPlots* m_hit = nullptr; /**<hit number summary histogram*/
    SVDHistograms<TH1F>* m_histo_dist = nullptr; /**<occupancy distribution histograms*/
    SVDHistograms<TH1F>* m_histo_occ = nullptr; /**<occupancy histograms*/
    SVDHistograms<TH1F>* m_histo_zsOcc = nullptr; /**<occupancy VS ZScut histograms*/
    SVDHistograms<TH1F>* m_histo_zsOccSQ = nullptr; /**< occupancy VS ZS cut swuared histograms*/
    SVDHistograms<TH2F>* m_histo_occtdep = nullptr; /**< occupancy VS event number*/

  };
}


