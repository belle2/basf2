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
#include <framework/datastore/StoreObjPtr.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <svd/dataobjects/SVDShaperDigit.h>
// #include <svd/calibration/SVDFADCMaskedStrips.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <vxd/dataobjects/VxdID.h>
#include <TH1.h>
#include <TH2.h>
#include <string>
#include <vector>
#include <functional>

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
      /** Bits definition for the bitmask that selects the events to put in the histograms. */
      enum EEventType {
        c_HERInjection = 1,
        c_LERInjection = 2,
        c_NoInjection = 4
      };

      SVDDQMDoseModule(); /**< Default constructor, defines parameters. */

      void initialize() override final; /**< Overrides HistoModule::initialize */
      void beginRun() override final; /**< Overrides HistoModule::beginRun */
      void event() override final; /**< Overrides HistoModule::event */
      void defineHisto() override final; /**< Overrides HistoModule::defineHisto */

    private:
      /** A struct to define non-trivial histograms in a human-readable way. */
      typedef struct SensorGroup {
        /** The name will be "SVDInstOccupancy_@nameSuffix@side". */
        const char* nameSuffix;
        /** The title will be "SVD Instantaneous Occupancy @titleSuffix @side;Occupancy [%];Count / bin". */
        const char* titleSuffix;
        /// The number of bins for the instantaneous occupancy histo
        int nBins;
        /// The lower limit for the instantaneous occupancy histo
        double xMin;
        /// The upper limit for the instantaneous occupancy histo
        double xMax;
        /** Function that says if a sensor is in this group. */
        std::function<bool(const VxdID&)> contains;
        /** Total number of U-side strips in this group. Set to zero, will be computed in initialize(). */
        mutable int nStripsU = 0;
      } SensorGroup;

      // Steerable data members (parameters)
      unsigned int m_eventFilter; /**< Bitmask for event type selection, see EEventType . */
      std::string m_histogramDirectoryName; /**< Name of the histograms' directory in the ROOT file. */
      std::string m_SVDShaperDigitsName; /**< Name of the StoreArray of SVDShaperDigit to use. */
      double m_noInjectionTime; /**< After this time (in microseconds) from last injection the event falls in the "No Injection" category. */
      double m_revolutionTime; /**< Beam revolution time in microseconds. */
      std::vector<int> m_trgTypes;

      // Inputs
      StoreArray<RawFTSW> m_rawTTD; /**< Input: DAQ status. Has timing and injection info. */
      StoreArray<SVDShaperDigit> m_digits; /**< Input: raw hits. */
      StoreObjPtr<TRGSummary> m_trgSummary; /**< Input: trigger type. */

      // Outputs (histograms)
      /** Hist of the total evts in each time bin (time since inj. and time in cycle). */
      TH2F* h_nEvtsVsTime = nullptr;
      /// Hists of the number of hits in each time bin per sensor group, U-side.
      std::vector<TH2F*> m_groupNHitsU;
      /** Hists of the instantaneous occupancy per sensor group (see c_sensorGroups), U-side. */
      std::vector<TH1F*> m_groupOccupanciesU;

      // Other stuff
      /** List of interesting groups of sensors to average over. Defined in SVDDQMDoseModule.cc . */
      static const std::vector<SensorGroup> c_sensorGroups;
    };
  }
}
