/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua Li, Thomas Hauth, Nils Braun                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
      /// Budget time of the event
      TH1F* m_fullTimeHistogram;
      /// Processing time of the event
      TH1F* m_processingTimeHistogram;
      /// Mean budget time of the event per unit
      TH1F* m_fullTimePerUnitHistogram;
      /// Mean processing time of the event per unit
      TH1F* m_processingTimePerUnitHistogram;

      /// Storage for the last full time sum
      double m_lastFullTimeSum = 0;
      /// Storage for the last processing time sum
      double m_lastProcessingTimeSum = 0;

      /// Parameter: Create HLT unit number histograms?
      bool m_param_create_hlt_unit_histograms;
      /// Parameter: Directory to put the generated histograms
      std::string m_param_histogramDirectoryName = "timing_statistics";
      /// Parameter: which modules should be shown in the overview list
      std::vector<std::string> m_param_overviewModuleList = {"Sum_Start_ZMQ", "Sum_Initialization", "Sum_Unpackers", "Sum_EventsofDoomBuster", "Sum_Clustering", "Sum_Tracking", "Sum_Posttracking_Reconstruction", "Sum_HLT_Filter_Calculation", "Sum_HLT_DQM_before_filter", "Sum_HLT_Discard", "Sum_HLT_Skim_Calculation", "Sum_ROI_Finder", "Sum_HLT_DQM_filtered", "Sum_ROI_Assembler", "Sum_HLT_DQM_all_events", "Sum_Close_Event", "Sum_Finalize_ZMQ"};
      /// Parameter: Summary modules of the actual processing
      std::vector<std::string> m_summaryModuleList = {"Sum_Initialization", "Sum_Unpackers", "Sum_EventsofDoomBuster", "Sum_Clustering", "Sum_Tracking", "Sum_Posttracking_Reconstruction", "Sum_HLT_Filter_Calculation", "Sum_HLT_DQM_before_filter", "Sum_HLT_Discard", "Sum_HLT_Skim_Calculation", "Sum_ROI_Finder", "Sum_HLT_DQM_filtered", "Sum_ROI_Assembler", "Sum_HLT_DQM_all_events", "Sum_Close_Event"};

      /// Store HLT unit number on initialization
      int m_hlt_unit = 0;
      /// Maximum number of HLT units used during the experiment
      const unsigned int m_max_hlt_units = 10;
    };

  }
}
