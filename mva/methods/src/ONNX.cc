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
using namespace Belle2::MVA::ONNX;

Session::Session(const std::string filename)
{
  // Ensure single-threaded execution, see
  // https://onnxruntime.ai/docs/performance/tune-performance/threading.html
  //
  // InterOpNumThreads is probably optional (not used in ORT_SEQUENTIAL mode)
  // Also, with batch size 1 and ORT_SEQUENTIAL mode, MLP-like models will
  // always run single threaded, but maybe not e.g. graph networks which can
  // run in parallel on nodes. Here, setting IntraOpNumThreads to 1 is
  // important to ensure single-threaded execution.
  m_sessionOptions.SetIntraOpNumThreads(1);
  m_sessionOptions.SetInterOpNumThreads(1);
  m_sessionOptions.SetExecutionMode(ORT_SEQUENTIAL); // default, but make it explicit

  m_session = std::make_unique<Ort::Session>(m_env, filename.c_str(), m_sessionOptions);
}

void Session::run(const std::map<std::string, std::shared_ptr<BaseTensor>>& inputMap,
                  const std::map<std::string, std::shared_ptr<BaseTensor>>& outputMap)
{
  std::vector<Ort::Value> inputs;
  std::vector<Ort::Value> outputs;
  std::vector<const char*> inputNames;
  std::vector<const char*> outputNames;
  for (auto& x : inputMap) {
    inputNames.push_back(x.first.c_str());
    inputs.push_back(x.second->createOrtTensor());
  }
  for (auto& x : outputMap) {
    outputNames.push_back(x.first.c_str());
    outputs.push_back(x.second->createOrtTensor());
  }
  run(inputNames, inputs, outputNames, outputs);
}

void Session::run(const std::vector<const char*>& inputNames,
                  std::vector<Ort::Value>& inputs,
                  const std::vector<const char*>& outputNames,
                  std::vector<Ort::Value>& outputs)
{
  m_session->Run(m_runOptions, inputNames.data(), inputs.data(), inputs.size(),
                 outputNames.data(), outputs.data(), outputs.size());
}

void ONNXExpert::load(Weightfile& weightfile)
{
  std::string onnxModelFileName = weightfile.generateFileName();
  weightfile.getFile("ONNX_Modelfile", onnxModelFileName);
  weightfile.getOptions(m_general_options);
  m_session = std::make_unique<Session>(onnxModelFileName.c_str());
}

std::vector<float> ONNXExpert::apply(Dataset& testData) const
{
  auto nFeatures = testData.getNumberOfFeatures();
  auto nEvents = testData.getNumberOfEvents();
  auto input = Tensor<float>::make_shared({1, nFeatures});
  auto output = Tensor<float>::make_shared({1, 1});
  std::vector<float> result;
  result.reserve(nEvents);
  for (unsigned int iEvent = 0; iEvent < nEvents; ++iEvent) {
    testData.loadEvent(iEvent);
    input->setValues(testData.m_input);
    m_session->run({{"input", input}}, {{"output", output}});
    result.push_back(output->at(0));
  }
  return result;
}

std::vector<std::vector<float>> ONNXExpert::applyMulticlass(Dataset& testData) const
{
  unsigned int nClasses = m_general_options.m_nClasses;
  auto nFeatures = testData.getNumberOfFeatures();
  auto nEvents = testData.getNumberOfEvents();
  auto input = Tensor<float>::make_shared({1, nFeatures});
  auto output = Tensor<float>::make_shared({1, nClasses});
  std::vector<std::vector<float>> result(nEvents, std::vector<float>(nClasses));
  for (unsigned int iEvent = 0; iEvent < nEvents; ++iEvent) {
    testData.loadEvent(iEvent);
    input->setValues(testData.m_input);
    m_session->run({{"input", input}}, {{"output", output}});
    for (unsigned int iClass = 0; iClass < nClasses; ++iClass) {
      result[iEvent][iClass] = output->at(iClass);
    }
  }
  return result;
}
