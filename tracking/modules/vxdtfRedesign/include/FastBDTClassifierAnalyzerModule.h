/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <map>

#include <framework/core/Module.h>

#include <tracking/trackFindingVXD/filterTools/FBDTClassifierHelper.h>
#include <tracking/trackFindingVXD/filterTools/FBDTClassifier.h>


namespace Belle2 {

  /**
   * Module to evaluate a trained fastBDT
   */
  class FastBDTClassifierAnalyzerModule : public Module {
  public:
    /// Constructor
    FastBDTClassifierAnalyzerModule();

    /// Module initialization
    void initialize() override;

    /// Module termination
    void terminate() override;

  private:
    /// Using type definition for fastBDT training sample type
    using TrainSample = Belle2::FBDTTrainSample<9>;

    std::string m_PARAMfbdtFileName; /**< weight file name */
    std::string m_PARAMtrainSampleFileName; /**< training sample file name */
    std::string m_PARAMtestSampleFileName; /**< test sample file name */
    std::string m_PARAMrootOutFileName; /**< output file name */

    std::vector<TrainSample> m_trainSample; /**< vector for training sample */
    std::vector<TrainSample> m_testSample; /**< vector for test sample */
    Belle2::FBDTClassifier<9> m_classifier; /**< classifier */
    std::multimap<int, double> m_trainOutput; /**< map containing output for each training event */
    std::multimap<int, double> m_testOutput; /**< map containing output for each test event */
  };
}
