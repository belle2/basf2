/*************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2016 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Madlener                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/filterTools/FBDTClassifierHelper.h>
#include <tracking/trackFindingVXD/filterTools/FBDTClassifier.h>

#include <framework/core/Module.h>

#include <string>
#include <vector>
#include <map>

namespace Belle2 {

  class FastBDTClassifierAnalyzerModule : public Module {

  public:

    FastBDTClassifierAnalyzerModule();

    void initialize() override;

    void terminate() override;

  private:

    using TrainSample = Belle2::FBDTTrainSample<9>;

    std::string m_PARAMfbdtFileName;

    std::string m_PARAMtrainSampleFileName;

    std::string m_PARAMtestSampleFileName;

    std::string m_PARAMrootOutFileName;

    std::vector<TrainSample> m_trainSample;

    std::vector<TrainSample> m_testSample;

    Belle2::FBDTClassifier<9> m_classifier;

    std::multimap<int, double> m_trainOutput;

    std::multimap<int, double> m_testOutput;
  };
}
