/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <TH1.h>
#include <TH2.h>
#include <string>
#include <vector>
#include <functional>

namespace Belle2 {
  // Forward declarations to avoid the need of #includes here in the header
  class RawFTSW;
  class SVDShaperDigit;
  class TRGSummary;
  class VxdID;
  // class SVDFADCMaskedStrips;

  namespace SVD {
    /** The SVD dose-monitoring DQM module.
     *
     * Fills histograms of the SVD's instantaneous occupancy and of SVD
     * occupancy vs time since last injection and time in beam
     * revolution cycle. The primary purpose is to monitor the
     * correlation coefficients between the dose rate in the diamonds
     * and that in the layers of SVD.
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
        /** The number of bins for the instantaneous occupancy histo */
        int nBins;
        /** The lower limit for the instantaneous occupancy histo */
        double xMin;
        /** The upper limit for the instantaneous occupancy histo */
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
      std::vector<int> m_trgTypes; /**< Trigger types to accept (all if the vector is empty). */

      // Inputs
      StoreArray<RawFTSW> m_rawTTD; /**< Input: DAQ status. Has timing and injection info. */
      StoreArray<SVDShaperDigit> m_digits; /**< Input: raw hits. */
      StoreObjPtr<TRGSummary> m_trgSummary; /**< Input: trigger type. */

      // Outputs (histograms)
      /** Hist of the total evts in each time bin (time since inj. and time in cycle). */
      TH2F* h_nEvtsVsTime = nullptr;
      /** Hists of the number of hits in each time bin per sensor group, U-side. */
      std::vector<TH2F*> m_groupNHitsU;
      /** Hists of the instantaneous occupancy per sensor group (see c_sensorGroups), U-side. */
      std::vector<TH1F*> m_groupOccupanciesU;
      /** Hist of the total evts in each time bin (1D, time since inj. only). */
      TH1F* h_nEvtsVsTime1 = nullptr;
      /** Hists of the number of hits in each time bin (1D) per sensor group, U-side. */
      std::vector<TH1F*> m_groupNHits1U;
      /** Hist of bunch number vs number of fired strips (copied from SVDDQMInjection). */
      TH2F* h_bunchNumVsNHits = nullptr;

      // Other stuff
      /** Beam revolution time in microseconds (approximated).
       *
       * The exact time could be obtained as
       * `5120 / HardwareClockSettings::getAcceleratorRF() * 1e3`
       * but this would run after defineHisto() if used in initialize().
       * Since defineHisto() uses this value, using a run-independent
       * approximated constant value is the only way.
       */
      static constexpr double c_revolutionTime = 5120.0 / 508.0;
      /** Approximated global clock frequency in MHz.
       *
       * Used to convert TTD timing to us.
       *
       * The exact frequency could be obtained as
       * `HardwareClockSettings::getGlobalClockFrequency() * 1e3`
       * but this would produce inconsistent histograms since I am
       * forced to use an approximated accelerator RF (508 MHz) for the
       * beam revolution period.
       *
       * @sa c_revolutionTime
       */
      static constexpr double c_globalClock = 127.0;

      /** Default minimum of the instantaneous occupancy histograms. */
      static constexpr double c_defaultOccuMin = 0.0;
      /** Default number of bins of the instantaneous occupancy histograms.
       * Optimized to get a maximum around 5%.
       * @sa c_defaultOccuMax
       */
      static const int c_defaultNBins = 90;
      /** Default maximum of the instantaneous occupancy histograms.
       * This value is optimized for a single ladder of layer 3, which has two sensors
       * with 768 strips each (on both sides). The occupancy resolution on one event
       * in this case is (in %) 100/nStrips = 100/1536 (exactly this value).
       *
       * Using a bin width different from the resolution would result in artefacts
       * (some bins have zero entries because the resolution can never assume a value
       * in that range, as it is a discrete measure); using bins much larger than the
       * resolution would not resolve anything, as they would be too large to read
       * anything; hence, using the exact resolution as bin width is necessary.
       */
      static constexpr double c_defaultOccuMax = 100.0 / 1536.0 * c_defaultNBins;

      /** List of interesting groups of sensors to average over. Defined in SVDDQMDoseModule.cc . */
      static const std::vector<SensorGroup> c_sensorGroups;
    };
  }
}
