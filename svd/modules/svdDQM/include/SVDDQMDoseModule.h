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
        /**< The name will be "SVDInstOccupancy_@nameSuffix@side". */
        const char* nameSuffix;
        /**< The title will be "SVD Instantaneous Occupancy @titleSuffix @side;Occupancy [%];Count / bin". */
        const char* titleSuffix;
        int nBins;
        double xMin;
        double xMax;
        /** Function that says if a sensor is in this group. */
        std::function<bool(const VxdID&)> contains;
        /** Total number of U-side strips in this group. Set to zero, will be computed in initialize(). */
        mutable int nStripsU = 0;
        /** Total number of V-side strips in this group. Set to zero, will be computed in initialize(). */
        mutable int nStripsV = 0;
      } SensorGroup;

      // Steerable data members (parameters)
      unsigned int m_eventFilter; /**< Bitmask for event type selection, see EEventType . */
      std::string m_histogramDirectoryName; /**< Name of the histograms' directory in the ROOT file. */
      std::string m_SVDShaperDigitsName; /**< Name of the StoreArray of SVDShaperDigit to use. */
      double m_noInjectionTime; /**< After this time (in microseconds) from last injection the event falls in the "No Injection" category. */
      double m_revolutionTime; /**< Beam revolution time in microseconds. */

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
      /** Hists of the instantaneous occupancy per sensor group (see c_sensorGroups), U-side. */
      std::vector<TH1F*> m_groupOccupanciesU;
      /** Hists of the instantaneous occupancy per sensor group (see c_sensorGroups), V-side. */
      std::vector<TH1F*> m_groupOccupanciesV;

      // Other stuff
      /** List of interesting groups of sensors to average over. */
      const std::vector<SensorGroup> c_sensorGroups{
        {"L3XX", "L3 all", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 3; }},
        {"L3X1", "L3.X.1", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 3 && s.getSensorNumber() == 1; }},
        {"L3X2", "L3.X.2", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 3 && s.getSensorNumber() == 2; }},
        {"L4XX", "L4 all", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 4; }},
        {"L4X1", "L4.X.1", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 4 && s.getSensorNumber() == 1; }},
        {"L4X2", "L4.X.2", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 4 && s.getSensorNumber() == 2; }},
        {"L4X3", "L4.X.3", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 4 && s.getSensorNumber() == 3; }},
        {"L5XX", "L5 all", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 5; }},
        {"L5X1", "L5.X.1", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 5 && s.getSensorNumber() == 1; }},
        {"L5X2", "L5.X.2", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 5 && s.getSensorNumber() == 2; }},
        {"L5X3", "L5.X.3", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 5 && s.getSensorNumber() == 3; }},
        {"L5X4", "L5.X.4", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 5 && s.getSensorNumber() == 4; }},
        {"L6XX", "L6 all", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 6; }},
        {"L6X1", "L6.X.1", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 6 && s.getSensorNumber() == 1; }},
        {"L6X2", "L6.X.2", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 6 && s.getSensorNumber() == 2; }},
        {"L6X3", "L6.X.3", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 6 && s.getSensorNumber() == 3; }},
        {"L6X4", "L6.X.4", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 6 && s.getSensorNumber() == 4; }},
        {"L6X5", "L6.X.5", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 6 && s.getSensorNumber() == 5; }},
        {"L3mid", "L3 mid plane (L3.1.X and L3.2.X)", 90, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 3 && s.getLadderNumber() < 3; }},
        {"L4mid", "L4 mid plane (L4.6.1 and L4.6.2)", 30, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 4 && s.getLadderNumber() == 6 && s.getSensorNumber() < 3; }},
        {"L5mid", "L5 mid plane (L5.8.1 and L5.8.2)", 30, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 5 && s.getLadderNumber() == 8 && s.getSensorNumber() < 3; }},
        {"L6mid", "L6 mid plane (L6.10.1 and L6.10.2)", 30, 0.0, 5.859375, [](const VxdID & s) { return s.getLayerNumber() == 6 && s.getLadderNumber() == 10 && s.getSensorNumber() < 3; }}};
    };
  }
}
