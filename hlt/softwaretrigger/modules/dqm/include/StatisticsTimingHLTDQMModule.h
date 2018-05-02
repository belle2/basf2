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
    class StatisticsTimingHLTDQMModule : public HistoModule {
    public:
      /// Constructor for module parameters
      StatisticsTimingHLTDQMModule();

      /// Define the histograms
      void initialize();

      /// Full the histograms
      void event();

      /// Histogram definition
      void defineHisto();

    private:
      /// Mean time of certain modules
      TH1F* m_meanTimeHistogram;
      /// Mean memory of certain modules
      TH1F* m_meanMemoryHistogram;
      /// Time of the full event
      TH1F* m_fullTimeHistogram;

      /// Storage for the last full time sum
      double m_lastFullTimeSum = 0;

      /// Parameter: Directory to put the generated histograms
      std::string m_param_histogramDirectoryName = "timing_statistics";
      /// Parameter: which modules should be shown in the overview list
      std::vector<std::string> m_param_overviewModuleList = {"Sum_Tracking", "Sum_PID", "Sum_Clustering"};
    };

  }
}
