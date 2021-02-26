/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ludovico Massaccesi                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/calibration/SVDFADCMaskedStrips.h>
#include <svd/dataobjects/SVDHistograms.h>
#include <vxd/dataobjects/VxdID.h>
#include <TH1.h>
#include <TH2.h>
#include <string>
#include <vector>

namespace Belle2 {
  namespace SVD {
    /** The SVD dose-monitoring DQM module.
     *
     * Fills histograms of the SVD's occupancy, occupancy vs time since
     * last injection and time in beam cycle, occupancy by layer, by
     * sensor type, by trigger type, by injection type, by presence of
     * injection veto, by ... (TODO).
     *
     * @sa https://agira.desy.de/browse/BII-7853
     */
    class SVDDQMDoseModule : public HistoModule {
    public:
      SVDDQMDoseModule(); /**< Default constructor, defines parameters. */

      void initialize() override final; /**< Overrides HistoModule::initialize */
      void beginRun() override final; /**< Overrides HistoModule::beginRun */
      void event() override final; /**< Overrides HistoModule::event */
      void defineHisto() override final; /**< Overrides HistoModule::defineHisto */

    private:
      // Steerable data members (parameters)
      std::string m_histogramDirectoryName; /**< Name of the histograms' directory in the ROOT file. */
      std::string m_SVDShaperDigitsName; /**< Name of the StoreArray of SVDShaperDigit to use. */
      double m_revolutionTime;

      // Inputs
      StoreArray<RawFTSW> m_rawTTD; /**< Input: DAQ status. */
      StoreArray<SVDShaperDigit> m_digits; /**< Input: raw hits. */

      // Outputs (histograms)
      /** Hists of the instantaneous occupancy distrib. per sensor. */
      SVDHistograms<TH1F>* m_occupancy = nullptr;
      /** Hists of the total hits in each time bin (time since inj. and time in cycle) per sensor. */
      SVDHistograms<TH2F>* m_nHitsVsTime = nullptr;
      /** Hist of the total evts in each time bin (time since inj. and time in cycle). */
      TH2F* h_nEvtsVsTime = nullptr;

      /** Hists of the instantaneous occupancy distribution for L3 U side.
       *  - `h_occupancyL3[0]`: averaged on all layer 3
       *  - `h_occupancyL3[1]`: L3.X.1 averaged on ladders
       *  - `h_occupancyL3[2]`: L3.X.2 averaged on ladders
       */
      TH1F* h_occupancyL3U[3] = {0}; // Init to all zeros
      /** Hists of the instantaneous occupancy distribution for L4 U side.
       *  - `h_occupancyL4[0]`: averaged on all layer 4
       *  - `h_occupancyL4[1]`: L4.X.1 averaged on ladders
       *  - ...
       *  - `h_occupancyL4[3]`: L4.X.3 averaged on ladders
       */
      TH1F* h_occupancyL4U[4] = {0}; // Init to all zeros
      /** Hists of the instantaneous occupancy distribution for L5 U side.
       *  - `h_occupancyL4[0]`: averaged on all layer 5
       *  - `h_occupancyL5[1]`: L5.X.1 averaged on ladders
       *  - ...
       *  - `h_occupancyL5[4]`: L5.X.4 averaged on ladders
       */
      TH1F* h_occupancyL5U[5] = {0}; // Init to all zeros
      /** Hists of the instantaneous occupancy distribution for L6 U side.
       *  - `h_occupancyL6[0]`: averaged on all layer 6
       *  - `h_occupancyL6[1]`: L6.X.1 averaged on ladders
       *  - ...
       *  - `h_occupancyL6[5]`: L6.X.5 averaged on ladders
       */
      TH1F* h_occupancyL6U[6] = {0}; // Init to all zeros
      /** Hists of the instantaneous occupancy distribution for L3 V side.
       *  - `h_occupancyL3[0]`: averaged on all layer 3
       *  - `h_occupancyL3[1]`: L3.X.1 averaged on ladders
       *  - `h_occupancyL3[2]`: L3.X.2 averaged on ladders
       */
      TH1F* h_occupancyL3V[3] = {0}; // Init to all zeros
      /** Hists of the instantaneous occupancy distribution for L4 V side.
       *  - `h_occupancyL4[0]`: averaged on all layer 4
       *  - `h_occupancyL4[1]`: L4.X.1 averaged on ladders
       *  - ...
       *  - `h_occupancyL4[3]`: L4.X.3 averaged on ladders
       */
      TH1F* h_occupancyL4V[4] = {0}; // Init to all zeros
      /** Hists of the instantaneous occupancy distribution for L5 V side.
       *  - `h_occupancyL4[0]`: averaged on all layer 5
       *  - `h_occupancyL5[1]`: L5.X.1 averaged on ladders
       *  - ...
       *  - `h_occupancyL5[4]`: L5.X.4 averaged on ladders
       */
      TH1F* h_occupancyL5V[5] = {0}; // Init to all zeros
      /** Hists of the instantaneous occupancy distribution for L6 V side.
       *  - `h_occupancyL6[0]`: averaged on all layer 6
       *  - `h_occupancyL6[1]`: L6.X.1 averaged on ladders
       *  - ...
       *  - `h_occupancyL6[5]`: L6.X.5 averaged on ladders
       */
      TH1F* h_occupancyL6V[6] = {0}; // Init to all zeros

      /** L3 mid plane U-side instant. occupancy average (L3.1.X and L3.2.X). */
      TH1F* h_occupancyL3Umid = nullptr;
      /** L4 mid plane U-side instant. occupancy average (L4.6.1 and L4.6.2). */
      TH1F* h_occupancyL4Umid = nullptr;
      /** L5 mid plane U-side instant. occupancy average (L5.8.1 and L5.8.2). */
      TH1F* h_occupancyL5Umid = nullptr;
      /** L6 mid plane U-side instant. occupancy average (L6.10.1 and L6.10.2). */
      TH1F* h_occupancyL6Umid = nullptr;
      /** L3 mid plane V-side instant. occupancy average (L3.1.X and L3.2.X). */
      TH1F* h_occupancyL3Vmid = nullptr;
      /** L4 mid plane V-side instant. occupancy average (L4.6.1 and L4.6.2). */
      TH1F* h_occupancyL4Vmid = nullptr;
      /** L5 mid plane V-side instant. occupancy average (L5.8.1 and L5.8.2). */
      TH1F* h_occupancyL5Vmid = nullptr;
      /** L6 mid plane V-side instant. occupancy average (L6.10.1 and L6.10.2). */
      TH1F* h_occupancyL6Vmid = nullptr;

      const std::vector<VxdID> c_L3midSensors {VxdID(3, 1, 1), VxdID(3, 1, 2), VxdID(3, 2, 1), VxdID(3, 2, 2)};
      const std::vector<VxdID> c_L4midSensors {VxdID(4, 6, 1), VxdID(4, 6, 2)};
      const std::vector<VxdID> c_L5midSensors {VxdID(5, 8, 1), VxdID(5, 8, 2)};
      const std::vector<VxdID> c_L6midSensors {VxdID(6, 10, 1), VxdID(6, 10, 2)};
    };
  }
}
