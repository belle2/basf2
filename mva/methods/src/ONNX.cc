/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <mva/methods/ONNX.h>

#include <framework/logging/Logger.h>
#include <iostream>
#include <vector>

using namespace Belle2::MVA;

void ONNXExpert::load(Weightfile& weightfile)
{
  std::string onnxModelFileName = weightfile.generateFileName();
  weightfile.getFile("ONNX_Modelfile", onnxModelFileName);

  // Ensure single-threaded execution, see
  // https://onnxruntime.ai/docs/performance/tune-performance/threading.html
  //
  // InterOpNumThreads is probably optional (not used in ORT_SEQUENTIAL mode)
  // Also, with batch size 1 and ORT_SEQUENTIAL mode, MLP-like models will
  // always run single threaded, but maybe not e.g. graph networks which can run
  // in parallel on nodes. Here, setting IntraOpNumThreads to 1 is important to
  // ensure single-threaded execution.
  m_sessionOptions.SetIntraOpNumThreads(1);
  m_sessionOptions.SetInterOpNumThreads(1);
  m_sessionOptions.SetExecutionMode(ORT_SEQUENTIAL); // default, but make it explicit

  m_session = std::make_unique<Ort::Session>(m_env, onnxModelFileName.c_str(), m_sessionOptions);
}

void ONNXExpert::run(ONNXTensorView& view) const
{
  m_session->Run(m_runOptions,
                 m_inputNames, view.inputTensor(), 1,
                 m_outputNames, view.outputTensor(), 1);
}

std::vector<float> ONNXExpert::apply(Dataset& testData) const
{
  auto view = ONNXTensorView(testData, 1);
  std::vector<float> result;
  result.reserve(testData.getNumberOfEvents());
  for (unsigned int iEvent = 0; iEvent < testData.getNumberOfEvents(); ++iEvent) {
    testData.loadEvent(iEvent);
    run(view);
    result.push_back(view.outputData()[0]);
  }
  return result;
}

std::vector<std::vector<float>> ONNXExpert::applyMulticlass(Dataset& testData) const
{
  auto view = ONNXTensorView(testData, m_general_options.m_nClasses);
  std::vector<std::vector<float>> result(testData.getNumberOfEvents(),
                                         std::vector<float>(m_general_options.m_nClasses));
  for (unsigned int iEvent = 0; iEvent < testData.getNumberOfEvents(); ++iEvent) {
    testData.loadEvent(iEvent);
    run(view);
    auto outputs = view.outputData();
    for (unsigned int iClass = 0; iClass < m_general_options.m_nClasses; ++iClass) {
      result[iEvent][iClass] = outputs[iClass];
    }
  }
  return result;
}
