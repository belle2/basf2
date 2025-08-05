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

void ONNXExpert::load(Weightfile& weightfile)
{
  std::string onnxModelFileName = weightfile.generateFileName();
  weightfile.getFile("ONNX_Modelfile", onnxModelFileName);
  m_session = std::make_unique<Session>(onnxModelFileName.c_str());
}

std::vector<float> ONNXExpert::apply(Dataset& testData) const
{
  auto nFeatures = testData.getNumberOfFeatures();
  auto nEvents = testData.getNumberOfEvents();
  auto input = Tensor::make_shared(nFeatures, {1, nFeatures});
  auto output = Tensor::make_shared(1, {1, 1});
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
  auto input = Tensor::make_shared(nFeatures, {1, nFeatures});
  auto output = Tensor::make_shared(nClasses, {1, nClasses});
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
