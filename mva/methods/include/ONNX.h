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
       * @brief Interface class for Tensor template instantiations
       *
       * Provides a common base class to store heterogeneous Tensor<T> instances
       * in a map, for use in `Session::run`.
       */
      class BaseTensor {
      public:
        virtual ~BaseTensor() {}

        /**
         * Is implemented by Tensor::createOrtTensor
         */
        virtual Ort::Value createOrtTensor() = 0;
      };

      /**
       * @brief Represents an input or output tensor for an ONNX model.
       *
       * Stores both the data and shape information of a tensor as
       * `std::vector`. Shared pointers to instances of this class can be passed
       * in a map to `Session::run`.
       *
       * @tparam T The data type of the tensor elements (e.g., float, int64_t).
       */
      template <typename T>
      class Tensor : public BaseTensor {
        /**
         * Flat buffer storing tensor data in row-major order.
         */
        std::vector<T> m_values;

        /**
         * The dimensions of the tensor
         */
        std::vector<int64_t> m_shape;

        /**
         * @brief Memory information used for allocating ONNX Runtime tensors.
         *
         * Will be set to CPU allocator in the constructor.
         */
        Ort::MemoryInfo m_memoryInfo;

        /**
         * Calculates the internal vector size from the product of the shape dimensions
         */
        size_t sizeFromShape(const std::vector<int64_t>& shape)
        {
          size_t size = 1;
          for (auto n : shape) size *= n;
          return size;
        }

        /**
         * Checks if all shape dimensions are positive
         *
         * @throws std::invalid_argument if any shape dimension is negative
         */
        void checkShapePositive()
        {
          for (auto n : m_shape) {
            if (n < 0) throw std::invalid_argument("All shape dimensions must be positive");
          }
        }

      public:
        /**
         * @brief Constructs a tensor from shape
         *
         * @param shape Shape of the tensor.
         *
         * @throws std::invalid_argument if any shape dimension is negative
         */
        Tensor(std::vector<int64_t> shape)
          : m_values(sizeFromShape(shape)), m_shape(std::move(shape)),
            m_memoryInfo(
              Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU))
        {
          checkShapePositive();
        }


        /**
         * @brief Constructs a tensor from a data vector and shape.
         *
         * Copies the provided values into the tensor. Ensures that the size of the
         * values vector matches the product of the shape dimensions.
         *
         * @param values Flat vector of tensor values.
         * @param shape Shape of the tensor.
         *
         * @throws std::length_error if the size of values does not match the expected size.
         * @throws std::invalid_argument if any shape dimension is negative
         */
        Tensor(std::vector<T> values, std::vector<int64_t> shape)
          : m_values(std::move(values)), m_shape(std::move(shape)),
            m_memoryInfo(
              Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU))
        {
          if (sizeFromShape(m_shape) != m_values.size()) {
            throw std::length_error(
              "Size of the given values vector (" + std::to_string(m_values.size()) + ") "
              "does not match the product of the shape dimensions (" + std::to_string(sizeFromShape(m_shape)) + ")");
          }
          checkShapePositive();
        }

        /**
         * @brief Convenience method to create a shared pointer to a Tensor from shape.
         *
         * Useful for constructing shared pointers from initializer lists, e.g.
         *
         * @code
         * auto input = Tensor<float>::make_shared({batchSize, nFeatures});
         * @endcode
         *
         * @param shape Shape of the tensor.
         * @return Shared pointer to the created Tensor.
         */
        static auto make_shared(std::vector<int64_t> shape)
        {
          return std::make_shared<Tensor>(std::move(shape));
        }

        /**
         * @brief Convenience method to create a shared pointer to a Tensor from values and shape.
         *
         * Useful for constructing shared pointers from initializer lists, e.g.
         * to create a Tensor with 3 values and shape (1, 3):
         *
         * @code
         * auto input = Tensor<float>::make_shared({v1, v2, v3}, {1, 3});
         * @endcode
         *
         * @param values Flat vector of tensor values.
         * @param shape Shape of the tensor.
         * @return Shared pointer to the created Tensor.
         */
        static auto make_shared(std::vector<T> values,
                                std::vector<int64_t> shape)
        {
          return std::make_shared<Tensor>(std::move(values), std::move(shape));
        }

        /**
         * @brief Accesses the element at the specified flat index.
         *
         * Returns a reference to the element in the tensor's underlying storage
         * at the given 1D (flattened) index. Performs bounds checking.
         *
         * @param index The flat index into the tensor's data.
         * @return Reference to the tensor element at the specified index.
         *
         * @throws std::out_of_range if the index is out of bounds.
         */
        auto& at(size_t index) { return m_values.at(index); }

        /**
         * @brief Accesses the element at the specified multi-dimensional index.
         *
         * Converts the given multi-dimensional index into a flat index based on the
         * tensor's shape, and returns a reference to the corresponding element.
         * Performs bounds checking via the underlying flat index access.
         *
         * Example:
         *   For a tensor with shape {2, 3}, index {1, 2} accesses element at flat index 5.
         *
         * @param index A vector of indices, one for each dimension.
         * @return Reference to the tensor element at the specified index.
         *
         * @throws std::out_of_range if any index is out of bounds.
         */
        auto& at(std::vector<size_t> index)
        {
          size_t flat_index = 0;
          size_t stride = 1;
          for (int64_t i = m_shape.size() - 1; i >= 0; --i) {
            if (index[i] >= static_cast<size_t>(m_shape[i])) {
              throw std::out_of_range(
                "index " + std::to_string(index[i]) + " is out of bounds for axis "
                + std::to_string(i) + " with size " + std::to_string(m_shape[i]));
            }
            flat_index += index[i] * stride;
            stride *= m_shape[i];
          }
          return at(flat_index);
        }

        /**
         * @brief Replaces the internal values with a new vector.
         *
         * @param values A vector of values to be used to update the internal ones
         *
         * @throws std::length_error if the size of the new values vector does not match the internal size.
         */
        void setValues(const std::vector<T>& values)
        {
          if (m_values.size() != values.size()) {
            throw std::length_error(
              "Size of new values vector (" + std::to_string(values.size()) + ") "
              "differs from internal size (" + std::to_string(m_values.size()) + ")");
          }
          m_values = values;
        }

        /**
         * @brief Create an Ort::Value from pointers to the underlying data and shape.
         *
         * Primarily intended for internal use by `Session::run`.
         *
         * @note It is generally recommended to use the `Session::run` overload
         * that accepts maps of `Tensor` shared pointers. However, for
         * performance-critical scenarios, such as small, fast models executed in
         * tight loops, manually reusing `Ort::Value` instances may offer slight
         * performance gains (on the order of microseconds).
         *
         * @warning The returned `Ort::Value` does **not** take ownership of the
         * tensor data. It simply references the existing memory. Therefore, you
         * must ensure that the `Tensor` object remains alive for as long as the
         * `Ort::Value` is in use.
         *
         * @return An `Ort::Value` pointing to the `Tensor`'s internal data.
         */
        Ort::Value createOrtTensor()
        {
          return Ort::Value::CreateTensor(m_memoryInfo, m_values.data(),
                                          m_values.size(), m_shape.data(),
                                          m_shape.size());
        }
      };


      /**
       * @brief A wrapper around Ort::Session providing model execution.
       *
       * This class encapsulates an ONNX Runtime session, pre-configured with default
       * settings such as single-threaded execution. It offers a more user-friendly
       * interface to run inference using a custom `Tensor` class, which is easier to
       * work with than raw `Ort::Value` instances.
       *
       * Example usage:
       *
       * @code
       * #include <mva/methods/ONNX.h>
       *
       * using Belle2::MVA::ONNX::Tensor;
       *
       * // ...
       *
       * // assume my_model.onnx contains a model with inputs
       * // - "a": float Tensor with shape (1, 3)
       * // - "b": int64 Tensor with shape (1, 8, 5)
       * // and one float output called "output" with shape (1, 5)
       * Belle2::MVA::ONNX::Session session("my_model.onnx");
       *
       * auto input_a = Tensor<float>::make_shared(3, {1, 3});
       * auto input_b = Tensor<int64_t>::make_shared(8 * 5, {1, 8, 5});
       * auto output = Tensor<float>::make_shared(5, {1, 5});
       *
       * // example for filling data using multi dimensional indexing
       * input_b->at({0, 2, 4}) = 42;
       *
       * // run model and fill output values
       * session.run({{"a", input_a}, {"b", input_b}}, {{"output", output}});
       *
       * // get 3rd output value - example for 1-dimensional indexing
       * int output_3 = output->at(3);
       * @endcode
       *
       * @note This method will not work with `Tensor<bool>` since the
       * underlying `std::vector<bool>` does not support getting a pointer to an
       * array. If you have a model with boolean inputs, either convert it to
       * accept a different type (e.g. uint8_t) or use the `Session::run`
       * overload thet works directl with `Ort::Value` instances.
       */
      class Session {
      public:

        /**
        * Constructs a new ONNX Runtime Session using the specified model file.
        *
        * @param filename Path to the ONNX model file.
        */
        Session(const std::string filename);

        /**
        * @brief Runs inference on the model using named Tensor maps.
        *
        * This overload accepts maps of input and output tensor names to their corresponding tensor data.
        * It feeds the input tensors into the session and fills the output tensors with inference results.
        *
        * @param inputMap A map of input tensor names to shared pointers of Tensor objects.
        * @param outputMap A map of output tensor names to shared pointers of Tensor objects.
        */
        void run(const std::map<std::string, std::shared_ptr<BaseTensor>>& inputMap,
                 const std::map<std::string, std::shared_ptr<BaseTensor>>& outputMap);

        /**
        * @brief Runs inference on the model using raw ONNX Runtime inputs and outputs.
        *
        * This overload provides works with raw ONNX Runtime `Ort::Value` objects
        * and names. It's a lower level abstraction that should usually not be
        * directly used, except in performance critical applications (meaning
        * situations where saving O(1 microsecond) per call is helpful).
        *
        * @param inputNames A vector of input tensor names.
        * @param inputs A vector of Ort::Value objects representing the input tensors.
        * @param outputNames A vector of output tensor names.
        * @param outputs A vector of Ort::Value objects to be filled with the model's outputs.
        */
        void run(const std::vector<const char*>& inputNames,
                 std::vector<Ort::Value>& inputs,
                 const std::vector<const char*>& outputNames,
                 std::vector<Ort::Value>& outputs);

        /**
         * @brief Get a reference to the raw Ort::Session object
         *
         * Can be used to call methods on it
         */
        const Ort::Session& getOrtSession() { return *m_session; }

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
      virtual void load(const boost::property_tree::ptree&) override;

      /**
       * Save mechanism to store Options in a xml tree
       */
      virtual void save(boost::property_tree::ptree&) const override;

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

      /**
       * @brief Name of the output Tensor that is used to make predictions.
       *
       * Only has to be provided if there are multiple outputs and none of them
       * is called "output". In case of a single output, or if among multiple
       * outputs there is one called "output", it will be used automatically.
       */
      std::string m_outputName;

      /**
       * Filename of the model. Is used in ONNXTeacher::train as a path to the
       * ONNX model to store in the weightfile.
       */
      std::string m_modelFilename;
    };

    /**
     * Teacher for the ONNX MVA method.
     * Just there to satisfy the interface - doesn't do any training
     */
    class ONNXTeacher : public Teacher {

    public:
      /**
       * Constructs a new teacher using the GeneralOptions and specific options of this training
       *
       * @param generalOptions defining all shared options
       * @param onnxOptions defining all method specific options
       */
      ONNXTeacher(const GeneralOptions& generalOptions, const ONNXOptions& onnxOptions) : Teacher(generalOptions),
        m_specific_options(onnxOptions) {};

      /**
       * Won't do any actual training, but will return a valid MVA Weightfile
       *
       * The Dataset parameter is required to adhere to the interface, but ignored.
       */
      virtual Weightfile train(Dataset&) const override;

    private:
      ONNXOptions m_specific_options; /**< Method specific options */
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
       * Set up input and output names and perform consistency checks.
       */
      void configureInputOutputNames();

      /**
       * The ONNX inference session wrapper
       */
      std::unique_ptr<ONNX::Session> m_session;

      /**
       * ONNX specific options loaded from weightfile
       */
      ONNXOptions m_specific_options;

      /**
       * Name of the input tensor (will be determined automatically)
       */
      std::string m_inputName;

      /**
       * Name of the output tensor
       * (will either be determined automatically or loaded from specific options)
       */
      std::string m_outputName;
    };
  } // namespace MVA
} // namespace Belle2
