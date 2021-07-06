/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/HistoModule.h>
#include <vector>
#include <string>

class TH1F;

namespace Belle2 {
  namespace SoftwareTrigger {
    /// Module for Event Time Statistics DQM plots.
    class StatisticsTimingHLTDQMModule final : public HistoModule {
    public:
      /// Constructor for module parameters
      StatisticsTimingHLTDQMModule();

      /// Define the histograms
      void initialize() override;

      /// Full the histograms
      void event() override;

      /// Histogram definition
      void defineHisto() override;

      /// Reset histogram entries
      void beginRun() override;

    private:
      /// Mean time of certain modules
      TH1F* m_meanTimeHistogram;

      /// Mean memory of certain modules
      TH1F* m_meanMemoryHistogram;

      /// Time distribution of certain modules
      std::map<std::string, TH1F*> m_moduleTimeHistograms;

      /// Memory distribution of certain modules
      std::map<std::string, TH1F*> m_moduleMemoryHistograms;

      /// Budget time distribution of all events
      TH1F* m_fullTimeHistogram;

      /// Processing time distribution of all events
      TH1F* m_processingTimeHistogram;

      /// Total memory usage distribution of all events
      TH1F* m_fullMemoryHistogram;

      /// Mean budget time of events per unit
      TH1F* m_fullTimeMeanPerUnitHistogram;

      /// Mean processing time of events per unit
      TH1F* m_processingTimeMeanPerUnitHistogram;

      /// Budget time distribution of events per unit
      std::map<unsigned int, TH1F*> m_fullTimePerUnitHistograms;

      /// Processing time distribution of events per unit
      std::map<unsigned int, TH1F*> m_processingTimePerUnitHistograms;

      /// Total memory distribution of events per unit
      std::map<unsigned int, TH1F*> m_fullMemoryPerUnitHistograms;

      /// Number of processes per unit
      TH1F* m_processesPerUnitHistogram;

      /// Storage for the last full time sum
      double m_lastFullTimeSum = 0;

      /// Storage for the last full time sum per unit
      std::map<unsigned int, double> m_lastFullTimeSumPerUnit;

      /// Storage for the last processing time sum
      double m_lastProcessingTimeSum = 0;

      /// Storage for the last processing time sum per unit
      std::map<unsigned int, double> m_lastProcessingTimeSumPerUnit;

      /// Storage for the last time sum of certain modules
      std::map<std::string, double> m_lastModuleTimeSum;

      /// Parameter: Create HLT unit number histograms?
      bool m_param_create_hlt_unit_histograms;

      /// Parameter: Directory to put the generated histograms
      std::string m_param_histogramDirectoryName = "timing_statistics";

      /// Parameter: which modules should be shown in the overview list
      std::vector<std::string> m_param_overviewModuleList = {"Sum_Wait", "Sum_Initialization", "Sum_Unpackers", "Sum_EventsofDoomBuster", "Sum_Clustering", "Sum_Prefilter_Tracking", "Sum_Posttracking_Reconstruction", "Sum_HLT_Filter_Calculation", "Sum_HLT_DQM_before_filter", "Sum_HLT_Discard", "Sum_Postfilter_Reconstruction", "Sum_HLT_Skim_Calculation", "Sum_ROI_Finder", "Sum_HLT_DQM_filtered", "Sum_ROI_Payload_Assembler", "Sum_HLT_DQM_all_events", "Sum_Close_Event"};

      /// Summary modules of the actual processing
      std::vector<std::string> m_summaryModuleList = {"Sum_Initialization", "Sum_Unpackers", "Sum_EventsofDoomBuster", "Sum_Clustering", "Sum_Prefilter_Tracking", "Sum_Posttracking_Reconstruction", "Sum_HLT_Filter_Calculation", "Sum_HLT_DQM_before_filter", "Sum_HLT_Discard", "Sum_Postfilter_Reconstruction", "Sum_HLT_Skim_Calculation", "Sum_ROI_Finder", "Sum_HLT_DQM_filtered", "Sum_ROI_Payload_Assembler", "Sum_HLT_DQM_all_events", "Sum_Close_Event"};

      /// Store HLT unit number on initialization
      int m_hlt_unit = 0;

      /// Maximum for the histograms of fullTime
      const double m_fullTimeMax = 10000;

      /// Number of bins for the histograms of fullTime
      const double m_fullTimeNBins = 250;

      /// Maximum for the histograms of processingTime
      const double m_processingTimeMax = 10000;

      /// Number of bins for the histograms of processingTime
      const double m_processingTimeNBins = 250;

      /// Maximum for the histograms of fullMemory
      const double m_fullMemoryMax = 4000;

      /// Number of bins for the histograms of fullMemory
      const double m_fullMemoryNBins = 100;
    };

  }
}
