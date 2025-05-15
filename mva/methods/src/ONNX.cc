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
  Ort::Env env;
  m_session = std::make_unique<Ort::Session>(env, onnxModelFileName.c_str(),
                                             Ort::SessionOptions{nullptr});
}

std::vector<float> ONNXExpert::apply(Dataset& testData) const
{
  std::vector<float>& input_data = testData.m_input;
  std::vector<int64_t> input_shape{1, testData.getNumberOfFeatures()};
  std::vector<float> output_data(1);
  std::vector<int64_t> output_shape{1, 1};

  auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
  auto input_tensor = Ort::Value::CreateTensor<float>(
                        memory_info,
                        input_data.data(), input_data.size(),
                        input_shape.data(), input_shape.size());
  auto output_tensor = Ort::Value::CreateTensor<float>(
                         memory_info,
                         output_data.data(), output_data.size(),
                         output_shape.data(), output_shape.size());

  std::vector<float> result;
  Ort::RunOptions run_options;
  const char* input_names[] = {"input"};
  const char* output_names[] = {"output"};
  for (unsigned int iEvent = 0; iEvent < testData.getNumberOfEvents(); ++iEvent) {
    testData.loadEvent(iEvent);
    m_session->Run(run_options, input_names, &input_tensor, 1, output_names, &output_tensor, 1);
    result.push_back(output_data[0]);
  }
  return result;
}
