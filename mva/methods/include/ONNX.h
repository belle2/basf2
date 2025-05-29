/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <mva/interface/Expert.h>
#include <mva/interface/Teacher.h>
#include <mva/interface/Options.h>

#include <onnxruntime/onnxruntime_cxx_api.h>

namespace Belle2 {
  namespace MVA {
    /**
     * Options for the ONNX MVA method.
     */
    class ONNXOptions : public SpecificOptions {

    public:
      /**
       * Load mechanism to load Options from a xml tree
       * @param pt xml tree
       */
      virtual void load(const boost::property_tree::ptree& pt) override {}

      /**
       * Save mechanism to store Options in a xml tree
       * @param pt xml tree
       */
      virtual void save(boost::property_tree::ptree& pt) const override {}

      /**
       * Returns a program options description for all available options
       */
      virtual po::options_description getDescription() override
      {
        return po::options_description("ONNX options");
      }

      /**
       * Return method name
       */
      virtual std::string getMethod() const override { return "ONNX"; }
    };

    /**
     * Teacher for the ONNX MVA method.
     * Just there to satisfy the interface - doesn't do anything
     */
    class ONNXTeacher : public Teacher {

    public:
      /**
       * Constructs a new teacher using the GeneralOptions and specific options of
       * this training
       * @param general_options defining all shared options
       * @param specific_options defining all method specific options
       */
      ONNXTeacher(const GeneralOptions& general_options,
                  const ONNXOptions& specific_options) : Teacher(general_options) {}

      /**
       * Just returns a default-initialized weightfile
       * @param training_data used to train the method (ignored)
       */
      virtual Weightfile train(Dataset& training_data) const override
      {
        return Weightfile();
      }
    };

    /**
     * View a Dataset's m_input as ONNX Tensor
     * and also set up output buffers/Tensors
     */
    class ONNXTensorView {
    public:
      ONNXTensorView(Dataset& dataset, int nOutputs)
        : m_inputShape{1, dataset.getNumberOfFeatures()}, m_outputData(nOutputs),
          m_outputShape{1, nOutputs}, m_memoryInfo(Ort::MemoryInfo::CreateCpu(
                                                     OrtDeviceAllocator, OrtMemTypeCPU)),
          m_inputTensor(Ort::Value::CreateTensor<float>(
                          m_memoryInfo, dataset.m_input.data(), dataset.m_input.size(),
                          m_inputShape.data(), m_inputShape.size())),
          m_outputTensor(Ort::Value::CreateTensor<float>(
                           m_memoryInfo, m_outputData.data(), m_outputData.size(),
                           m_outputShape.data(), m_outputShape.size())) {}
      Ort::Value* inputTensor() { return &m_inputTensor; }
      Ort::Value* outputTensor() { return &m_outputTensor; }
      std::vector<float> outputData() { return m_outputData; }
    private:
      std::vector<int64_t> m_inputShape;
      std::vector<float> m_outputData;
      std::vector<int64_t> m_outputShape;
      Ort::MemoryInfo m_memoryInfo;
      Ort::Value m_inputTensor;
      Ort::Value m_outputTensor;
    };

    /**
     * Expert for the ONNX MVA method
     */
    class ONNXExpert : public Expert {
    public:
      /**
       * Load the expert from a Weightfile
       * @param weightfile containing all information necessary to build the expert
       */
      virtual void load(Weightfile& weightfile) override;

      /**
       * Apply this expert onto a dataset
       * @param testData dataset
       */
      virtual std::vector<float> apply(Dataset& testData) const override;

      /**
       * Apply this expert onto a dataset and return multiple outputs
       * @param test_data dataset
       */
      virtual std::vector<std::vector<float>> applyMulticlass(Dataset& test_data) const override;

    private:
      /**
       * Run the current inputs through the onnx model
       * Will retrieve and fill the buffers from the view
       */
      void run(ONNXTensorView& view) const;

      std::unique_ptr<Ort::Session> m_session;
      Ort::RunOptions m_runOptions;
      const char* m_inputNames[1] = {"input"};
      const char* m_outputNames[1] = {"output"};
    };
  } // namespace MVA
} // namespace Belle2
