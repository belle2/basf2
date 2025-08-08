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
    namespace ONNX {

      /**
       * Interface class to put Tensor instances of potentially different types into a map
       * (needed for Session::run)
       */
      class BaseTensor {
      public:
        virtual ~BaseTensor() {}
        virtual Ort::Value createOrtTensor() = 0;
      };

      /**
       * Holds information for an input tensor to a ONNX model
       * A map of shared pointers to Tensor can be passed to Session::run
       */
      template <typename T>
      class Tensor : public BaseTensor {
        /**
         * The flat data buffer
         */
        std::vector<T> m_values;

        /**
         * The dimensions of the tensor
         */
        std::vector<int64_t> m_shape;

        Ort::MemoryInfo m_memoryInfo;

      public:
        /**
         * Construct from size and shape
         */
        Tensor(size_t size, std::vector<int64_t> shape)
          : m_values(size), m_shape(std::move(shape)),
            m_memoryInfo(
              Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU)) {}

        /**
         * Construct from values of another vector (will be copied) and shape
         */
        Tensor(std::vector<T> values, std::vector<int64_t> shape)
          : m_values(std::move(values)), m_shape(std::move(shape)),
            m_memoryInfo(
              Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU)) {}

        /**
         * Construct as shared pointer from size and shape
         *
         * Exists mainly for convenience to construct a shared pointer to
         * Tensor from initializer lists, e.g.
         *
         * @code
         * auto input = Tensor<float>::make_shared(batchSize * nFeatures,
         *                                  {batchSize, nFeatures});
         * @endcode
         */
        static auto make_shared(size_t size, std::vector<int64_t> shape)
        {
          return std::make_shared<Tensor>(size, std::move(shape));
        }

        /**
         * Construct as shared pointer from another vector (will be copied) and shape
         *
         * Exists mainly for convenience to construct a shared pointer to
         * Tensor from initializer lists, e.g. to create a Tensor with 3
         * values and shape (1, 3):
         *
         * @code
         * auto input = Tensor<float>::make_shared({v1, v2, v3}, {1, 3});
         * @endcode
         */
        static auto make_shared(std::vector<T> values,
                                std::vector<int64_t> shape)
        {
          return std::make_shared<Tensor>(std::move(values), std::move(shape));
        }

        auto& at(size_t index) { return m_values.at(index); }
        auto& at(std::vector<size_t> index)
        {
          size_t flat_index = 0;
          size_t stride = 1;
          for (int64_t i = m_shape.size() - 1; i >= 0; --i) {
            flat_index += index[i] * stride;
            stride *= m_shape[i];
          }
          return at(flat_index);
        }

        void setValues(const std::vector<T>& values)
        {
          if (m_values.size() != values.size()) {
            throw std::out_of_range("Size of new values vector differs from internal size");
          }
          m_values = values;
        }
        Ort::Value createOrtTensor()
        {
          return Ort::Value::CreateTensor(m_memoryInfo, m_values.data(),
                                          m_values.size(), m_shape.data(),
                                          m_shape.size());
        }
      };


      /**
       * Wrapper around Ort::Session. The purpose is to set up the default
       * settings (e.g. single threaded execution) and provide a run method that
       * works with a custom Tensor class that is somewhat easier to handle
       * compared to Ort::Value instances.
       *
       * Example usage running a hypothetical ONNX model with 2 input tensors
       * "a", "b" with types float, int64_t, shapes (1, 3) and (1, 8, 5) and a
       * float output tensor named "output" with shape (1, 5):
       *
       * @code
       * #include <mva/methods/ONNX.h>
       * using Belle2::MVA::ONNX::Tensor;
       *
       * Belle2::MVA::ONNX::Session session("my_model.onnx");
       *
       * auto input_a = Tensor<float>::make_shared(3, {1, 3});
       * auto input_b = Tensor<int64_t>::make_shared(8 * 5, {1, 8, 5});
       * auto output = Tensor<float>::make_shared(5, {1, 5});
       *
       * input_b->at({0, 2, 4}) = 42; // example for filling data using multi dimensional indexing
       *
       * session.run({{"a", input_a}, {"b", input_b}}, {"output", output}); // will fill output values
       *
       * int output_3 = output->at(3) // get 3rd output value - example for 1-dimensional indexing
       * @endcode
       *
       * Note: This method will not work with Tensor<bool> since the underlying
       * std::vector<bool> does not support getting a pointer to an array. If
       * you have a model with boolean inputs, either convert it to accept a
       * different type (e.g. uint8_t) or use Session::run with Ort::Value
       * instances.
       */
      class Session {
      public:
        Session(const char* filename);
        void run(const std::map<std::string, std::shared_ptr<BaseTensor>>& inputMap,
                 const std::map<std::string, std::shared_ptr<BaseTensor>>& outputMap);
        void run(const std::vector<const char*>& inputNames,
                 std::vector<Ort::Value>& inputs,
                 const std::vector<const char*>& outputNames,
                 std::vector<Ort::Value>& outputs);

      private:
        /**
         * Environment object for ONNX session
         */
        Ort::Env m_env;

        /**
         * ONNX session configuration
         */
        Ort::SessionOptions m_sessionOptions;

        /**
         * The ONNX inference session
         */
        std::unique_ptr<Ort::Session> m_session;

        /**
         * Options to be passed to Ort::Session::Run
         */
        Ort::RunOptions m_runOptions;
      };
    } // namespace ONNX

    /**
     * Options for the ONNX MVA method.
     */
    class ONNXOptions : public SpecificOptions {

    public:
      /**
       * Load mechanism to load Options from a xml tree
       */
      virtual void load(const boost::property_tree::ptree&) override {}

      /**
       * Save mechanism to store Options in a xml tree
       */
      virtual void save(boost::property_tree::ptree&) const override {}

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
       */
      ONNXTeacher(const GeneralOptions& general_options,
                  const ONNXOptions&) : Teacher(general_options) {}

      /**
       * Just returns a default-initialized weightfile
       */
      virtual Weightfile train(Dataset&) const override
      {
        return Weightfile();
      }
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
       * The ONNX inference session wrapper
       */
      std::unique_ptr<ONNX::Session> m_session;
    };
  } // namespace MVA
} // namespace Belle2
