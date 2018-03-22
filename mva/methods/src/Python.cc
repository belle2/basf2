/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *               Jochen Gemmler                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/methods/Python.h>

#include <boost/filesystem/convenience.hpp>
#include <numpy/npy_common.h>
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <numeric>
#include <fstream>


namespace Belle2 {
  namespace MVA {

    void PythonOptions::load(const boost::property_tree::ptree& pt)
    {
      int version = pt.get<int>("Python_version");
      if (version < 1 or version > 2) {
        B2ERROR("Unknown weightfile version " << std::to_string(version));
        throw std::runtime_error("Unknown weightfile version " + std::to_string(version));
      }
      m_framework = pt.get<std::string>("Python_framework");
      m_steering_file = pt.get<std::string>("Python_steering_file");
      m_mini_batch_size = pt.get<unsigned int>("Python_mini_batch_size");
      m_nIterations = pt.get<unsigned int>("Python_n_iterations");
      m_config = pt.get<std::string>("Python_config");
      m_training_fraction = pt.get<double>("Python_training_fraction");
      if (version == 2) {
        m_normalize = pt.get<bool>("Python_normalize");
      } else {
        m_normalize = false;
      }

    }

    void PythonOptions::save(boost::property_tree::ptree& pt) const
    {
      pt.put("Python_version", 2);
      pt.put("Python_framework", m_framework);
      pt.put("Python_steering_file", m_steering_file);
      pt.put("Python_mini_batch_size", m_mini_batch_size);
      pt.put("Python_n_iterations", m_nIterations);
      pt.put("Python_config", m_config);
      pt.put("Python_training_fraction", m_training_fraction);
      pt.put("Python_normalize", m_normalize);
    }

    po::options_description PythonOptions::getDescription()
    {
      po::options_description description("Python options");
      description.add_options()
      ("framework", po::value<std::string>(&m_framework),
       "Framework which should be used. Currently supported are sklearn, tensorflow and theano")
      ("steering_file", po::value<std::string>(&m_steering_file), "Steering file which describes")
      ("mini_batch_size", po::value<unsigned int>(&m_mini_batch_size), "Size of the mini batch given to partial_fit function")
      ("nIterations", po::value<unsigned int>(&m_nIterations), "Number of iterations")
      ("normalize", po::value<bool>(&m_normalize), "Normalize input data (shift mean to 0 and std to 1)")
      ("training_fraction", po::value<double>(&m_training_fraction),
       "Training fraction used to split up dataset in training and validation sample.")
      ("config", po::value<std::string>(&m_config), "Json encoded python object passed to begin_fit function");
      return description;
    }

    /**
    Singleton class which handles the initialization and finalization of Python
        and numpy
    */
    class PythonInitializerSingleton {

    public:
      /**
        Return static instance of PythonInitializerSingleton
      */
      static PythonInitializerSingleton& GetInstance();

    private:
      /**
        Constructor of PythonInitializerSingleton
      */
      PythonInitializerSingleton()
      {
        if (not Py_IsInitialized()) {
          Py_Initialize();
          // wchar_t* bla[] = {L""};
          wchar_t** bla = nullptr;
          PySys_SetArgvEx(0, bla, 0);
          m_initialized_python = true;
        }

        if (PyArray_API == NULL) {
          init_numpy();
        }
      }

      /**
        Destructor of PythonInitializerSingleton
      */
      ~PythonInitializerSingleton()
      {
        if (m_initialized_python) {
          if (Py_IsInitialized()) {
            // We don't finalize Python because this call only frees some memory,
            // but can cause crashes in loaded python-modules like Theano
            // https://docs.python.org/3/c-api/init.html
            // Py_Finalize();
          }
        }
      }

      /**
        Forbid copy constructor of PythonInitializerSingleton
      */
      PythonInitializerSingleton(const PythonInitializerSingleton&) = delete;

      /**
        Helper funtion which initializes array system of numpy.
        Since import_array is a weird macro we need this wrapper function
        to protect us from the return statement in this macro
      */
      void* init_numpy()
      {
        // Import array is a macro which returns NUMPY_IMPORT_ARRAY_RETVAL
        import_array();
        return NULL;
      }

      bool m_initialized_python = false; /**< Member which keeps indicate if this class initialized python */
    };

    PythonInitializerSingleton& PythonInitializerSingleton::GetInstance()
    {
      static PythonInitializerSingleton singleton;
      return singleton;
    }


    PythonTeacher::PythonTeacher(const GeneralOptions& general_options,
                                 const PythonOptions& specific_options) : Teacher(general_options),
      m_specific_options(specific_options)
    {
      PythonInitializerSingleton::GetInstance();
    }


    Weightfile PythonTeacher::train(Dataset& training_data) const
    {

      Weightfile weightfile;
      std::string custom_weightfile = weightfile.generateFileName();
      std::string custom_steeringfile = weightfile.generateFileName();

      uint64_t numberOfFeatures = training_data.getNumberOfFeatures();
      uint64_t numberOfSpectators = training_data.getNumberOfSpectators();
      uint64_t numberOfEvents = training_data.getNumberOfEvents();

      auto numberOfValidationEvents = static_cast<uint64_t>(numberOfEvents * (1 - m_specific_options.m_training_fraction));
      auto numberOfTrainingEvents = static_cast<uint64_t>(numberOfEvents * m_specific_options.m_training_fraction);

      uint64_t batch_size = m_specific_options.m_mini_batch_size;
      if (batch_size == 0) {
        batch_size = numberOfTrainingEvents;
      }

      if (m_specific_options.m_training_fraction <= 0.0 or m_specific_options.m_training_fraction > 1.0) {
        B2ERROR("Please provide a positive training fraction");
        throw std::runtime_error("Please provide a training fraction between (0.0,1.0]");
      }

      auto X = std::unique_ptr<float[]>(new float[batch_size * numberOfFeatures]);
      auto S = std::unique_ptr<float[]>(new float[batch_size * numberOfSpectators]);
      auto y = std::unique_ptr<float[]>(new float[batch_size]);
      auto w = std::unique_ptr<float[]>(new float[batch_size]);
      npy_intp dimensions_X[2] = {static_cast<npy_intp>(batch_size), static_cast<npy_intp>(numberOfFeatures)};
      npy_intp dimensions_S[2] = {static_cast<npy_intp>(batch_size), static_cast<npy_intp>(numberOfSpectators)};
      npy_intp dimensions_y[2] = {static_cast<npy_intp>(batch_size), 1};
      npy_intp dimensions_w[2] = {static_cast<npy_intp>(batch_size), 1};

      auto X_v = std::unique_ptr<float[]>(new float[numberOfValidationEvents * numberOfFeatures]);
      auto S_v = std::unique_ptr<float[]>(new float[numberOfValidationEvents * numberOfSpectators]);
      auto y_v = std::unique_ptr<float[]>(new float[numberOfValidationEvents]);
      auto w_v = std::unique_ptr<float[]>(new float[numberOfValidationEvents]);
      npy_intp dimensions_X_v[2] = {static_cast<npy_intp>(numberOfValidationEvents), static_cast<npy_intp>(numberOfFeatures)};
      npy_intp dimensions_S_v[2] = {static_cast<npy_intp>(numberOfValidationEvents), static_cast<npy_intp>(numberOfSpectators)};
      npy_intp dimensions_y_v[2] = {static_cast<npy_intp>(numberOfValidationEvents), 1};
      npy_intp dimensions_w_v[2] = {static_cast<npy_intp>(numberOfValidationEvents), 1};

      std::string steering_file_source_code;
      if (m_specific_options.m_steering_file != "") {
        std::string filename = FileSystem::findFile(m_specific_options.m_steering_file);
        std::ifstream steering_file(filename);
        if (not steering_file) {
          throw std::runtime_error(std::string("Couldn't open file ") + filename);
        }
        steering_file.seekg(0, std::ios::end);
        steering_file_source_code.resize(steering_file.tellg());
        steering_file.seekg(0, std::ios::beg);
        steering_file.read(&steering_file_source_code[0], steering_file_source_code.size());
      }

      std::vector<float> means(numberOfFeatures, 0.0);
      std::vector<float> stds(numberOfFeatures, 0.0);

      if (m_specific_options.m_normalize) {
        // Stable calculation of mean and variance with weights
        // see https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
        auto weights = training_data.getWeights();
        for (uint64_t iFeature = 0; iFeature < numberOfFeatures; ++iFeature) {
          double wSum = 0.0;
          double wSum2 = 0.0;
          double mean = 0.0;
          double running_std = 0.0;
          auto feature = training_data.getFeature(iFeature);
          for (uint64_t i = 0; i < weights.size(); ++i) {
            wSum += weights[i];
            wSum2 += weights[i] * weights[i];
            double meanOld = mean;
            mean += (weights[i] / wSum) * (feature[i] - meanOld);
            running_std += weights[i] * (feature[i] - meanOld) * (feature[i] - mean);
          }
          means[iFeature] = mean;
          stds[iFeature] = std::sqrt(running_std / (wSum - 1));
        }
      }

      try {
        // Load python modules
        auto json = boost::python::import("json");
        auto builtins = boost::python::import("builtins");
        auto inspect = boost::python::import("inspect");

        // Load framework
        auto framework = boost::python::import((std::string("basf2_mva_python_interface.") + m_specific_options.m_framework).c_str());
        // Overwrite framework with user-defined code from the steering file
        builtins.attr("exec")(steering_file_source_code.c_str(), boost::python::object(framework.attr("__dict__")));

        // Call get_model with the parameters provided by the user
        auto parameters = json.attr("loads")(m_specific_options.m_config.c_str());
        auto model = framework.attr("get_model")(numberOfFeatures, numberOfSpectators,
                                                 numberOfEvents,  m_specific_options.m_training_fraction, parameters);

        // Call begin_fit with validation sample
        for (uint64_t iEvent = 0; iEvent < numberOfValidationEvents; ++iEvent) {
          training_data.loadEvent(iEvent);
          if (m_specific_options.m_normalize) {
            for (uint64_t iFeature = 0; iFeature < numberOfFeatures; ++iFeature)
              X_v[iEvent * numberOfFeatures + iFeature] = (training_data.m_input[iFeature] - means[iFeature]) / stds[iFeature];
          } else {
            for (uint64_t iFeature = 0; iFeature < numberOfFeatures; ++iFeature)
              X_v[iEvent * numberOfFeatures + iFeature] = training_data.m_input[iFeature];
          }
          for (uint64_t iSpectator = 0; iSpectator < numberOfSpectators; ++iSpectator)
            S_v[iEvent * numberOfSpectators + iSpectator] = training_data.m_spectators[iSpectator];
          y_v[iEvent] = training_data.m_target;
          w_v[iEvent] = training_data.m_weight;
        }

        auto ndarray_X_v = boost::python::handle<>(PyArray_SimpleNewFromData(2, dimensions_X_v, NPY_FLOAT32, X_v.get()));
        auto ndarray_S_v = boost::python::handle<>(PyArray_SimpleNewFromData(2, dimensions_S_v, NPY_FLOAT32, S_v.get()));
        auto ndarray_y_v = boost::python::handle<>(PyArray_SimpleNewFromData(2, dimensions_y_v, NPY_FLOAT32, y_v.get()));
        auto ndarray_w_v = boost::python::handle<>(PyArray_SimpleNewFromData(2, dimensions_w_v, NPY_FLOAT32, w_v.get()));

        auto state = framework.attr("begin_fit")(model, ndarray_X_v, ndarray_S_v, ndarray_y_v, ndarray_w_v);

        uint64_t nBatches = std::floor(numberOfTrainingEvents / batch_size);
        bool continue_loop = true;
        for (uint64_t iIteration = 0; (iIteration < m_specific_options.m_nIterations or m_specific_options.m_nIterations == 0)
             and continue_loop; ++iIteration) {
          for (uint64_t iBatch = 0; iBatch < nBatches and continue_loop; ++iBatch) {

            // Release Global Interpreter Lock in python to allow multithreading while reading root files
            // also see: https://docs.python.org/3.5/c-api/init.html
            PyThreadState* m_thread_state =  PyEval_SaveThread();
            for (uint64_t iEvent = 0; iEvent < batch_size; ++iEvent) {
              training_data.loadEvent(iEvent + iBatch * batch_size + numberOfValidationEvents);
              if (m_specific_options.m_normalize) {
                for (uint64_t iFeature = 0; iFeature < numberOfFeatures; ++iFeature)
                  X[iEvent * numberOfFeatures + iFeature] = (training_data.m_input[iFeature] - means[iFeature]) / stds[iFeature];
              } else {
                for (uint64_t iFeature = 0; iFeature < numberOfFeatures; ++iFeature)
                  X[iEvent * numberOfFeatures + iFeature] = training_data.m_input[iFeature];
              }
              for (uint64_t iSpectator = 0; iSpectator < numberOfSpectators; ++iSpectator)
                S[iEvent * numberOfSpectators + iSpectator] = training_data.m_spectators[iSpectator];
              y[iEvent] = training_data.m_target;
              w[iEvent] = training_data.m_weight;
            }

            // Maybe slow, create ndarrays outside of loop?
            auto ndarray_X = boost::python::handle<>(PyArray_SimpleNewFromData(2, dimensions_X, NPY_FLOAT32, X.get()));
            auto ndarray_S = boost::python::handle<>(PyArray_SimpleNewFromData(2, dimensions_S, NPY_FLOAT32, S.get()));
            auto ndarray_y = boost::python::handle<>(PyArray_SimpleNewFromData(2, dimensions_y, NPY_FLOAT32, y.get()));
            auto ndarray_w = boost::python::handle<>(PyArray_SimpleNewFromData(2, dimensions_w, NPY_FLOAT32, w.get()));

            // Reactivate Global Interpreter Lock to safely execute python code
            PyEval_RestoreThread(m_thread_state);
            auto r = framework.attr("partial_fit")(state, ndarray_X, ndarray_S, ndarray_y,
                                                   ndarray_w, iIteration * nBatches + iBatch);
            boost::python::extract<bool> proxy(r);
            if (proxy.check())
              continue_loop = static_cast<bool>(proxy);
          }
        }

        auto result = framework.attr("end_fit")(state);

        auto pickle = boost::python::import("pickle");
        auto file = builtins.attr("open")(custom_weightfile.c_str(), "wb");
        pickle.attr("dump")(result, file);

        auto steeringfile = builtins.attr("open")(custom_steeringfile.c_str(), "wb");
        pickle.attr("dump")(steering_file_source_code.c_str(), steeringfile);

        auto importances = framework.attr("feature_importance")(state);
        if (len(importances) == 0) {
          B2INFO("Python method returned empty feature importance. There won't be any information about the feature importance in the weightfile.");
        } else if (numberOfFeatures != static_cast<uint64_t>(len(importances))) {
          B2WARNING("Python method didn't return the correct number of importance value. I ignore the importances");
        } else {
          std::map<std::string, float> feature_importances;
          for (uint64_t iFeature = 0; iFeature < numberOfFeatures; ++iFeature) {
            boost::python::extract<float> proxy(importances[iFeature]);
            if (proxy.check()) {
              feature_importances[m_general_options.m_variables[iFeature]] = static_cast<float>(proxy);
            } else {
              B2WARNING("Failed to convert importance output of the method to a float, using 0 instead");
              feature_importances[m_general_options.m_variables[iFeature]] = 0.0;
            }
          }
          weightfile.addFeatureImportance(feature_importances);
        }

      } catch (...) {
        PyErr_Print();
        PyErr_Clear();
        B2ERROR("Failed calling train in PythonTeacher");
        throw std::runtime_error(std::string("Failed calling train in PythonTeacher"));
      }

      weightfile.addOptions(m_general_options);
      weightfile.addOptions(m_specific_options);
      weightfile.addFile("Python_Weightfile", custom_weightfile);
      weightfile.addFile("Python_Steeringfile", custom_steeringfile);
      weightfile.addSignalFraction(training_data.getSignalFraction());
      if (m_specific_options.m_normalize) {
        weightfile.addVector("Python_Means", means);
        weightfile.addVector("Python_Stds", stds);
      }

      return weightfile;

    }

    PythonExpert::PythonExpert()
    {
      PythonInitializerSingleton::GetInstance();
    }


    void PythonExpert::load(Weightfile& weightfile)
    {

      std::string custom_weightfile = weightfile.generateFileName();
      weightfile.getFile("Python_Weightfile", custom_weightfile);
      weightfile.getOptions(m_general_options);
      weightfile.getOptions(m_specific_options);

      if (m_specific_options.m_normalize) {
        m_means = weightfile.getVector<float>("Python_Means");
        m_stds = weightfile.getVector<float>("Python_Stds");
      }

      try {
        auto pickle = boost::python::import("pickle");
        auto builtins = boost::python::import("builtins");
        m_framework = boost::python::import((std::string("basf2_mva_python_interface.") + m_specific_options.m_framework).c_str());

        if (weightfile.containsElement("Python_Steeringfile")) {
          std::string custom_steeringfile = weightfile.generateFileName();
          weightfile.getFile("Python_Steeringfile", custom_steeringfile);
          auto steeringfile = builtins.attr("open")(custom_steeringfile.c_str(), "rb");
          auto source_code = pickle.attr("load")(steeringfile);
          builtins.attr("exec")(boost::python::object(source_code), boost::python::object(m_framework.attr("__dict__")));
        }

        auto file = builtins.attr("open")(custom_weightfile.c_str(), "rb");
        auto unpickled_fit_object = pickle.attr("load")(file);
        m_state = m_framework.attr("load")(unpickled_fit_object);
      } catch (...) {
        PyErr_Print();
        PyErr_Clear();
        B2ERROR("Failed calling load in PythonExpert");
        throw std::runtime_error("Failed calling load in PythonExpert");
      }

    }

    std::vector<float> PythonExpert::apply(Dataset& test_data) const
    {

      uint64_t numberOfFeatures = test_data.getNumberOfFeatures();
      uint64_t numberOfEvents = test_data.getNumberOfEvents();

      auto X = std::unique_ptr<float[]>(new float[numberOfEvents * numberOfFeatures]);
      npy_intp dimensions_X[2] = {static_cast<npy_intp>(numberOfEvents), static_cast<npy_intp>(numberOfFeatures)};

      for (uint64_t iEvent = 0; iEvent < numberOfEvents; ++iEvent) {
        test_data.loadEvent(iEvent);
        if (m_specific_options.m_normalize) {
          for (uint64_t iFeature = 0; iFeature < numberOfFeatures; ++iFeature)
            X[iEvent * numberOfFeatures + iFeature] = (test_data.m_input[iFeature] - m_means[iFeature]) / m_stds[iFeature];
        } else {
          for (uint64_t iFeature = 0; iFeature < numberOfFeatures; ++iFeature)
            X[iEvent * numberOfFeatures + iFeature] = test_data.m_input[iFeature];
        }
      }

      std::vector<float> probabilities(test_data.getNumberOfEvents());

      try {
        auto ndarray_X = boost::python::handle<>(PyArray_SimpleNewFromData(2, dimensions_X, NPY_FLOAT32, X.get()));
        auto result = m_framework.attr("apply")(m_state, ndarray_X);
        for (uint64_t iEvent = 0; iEvent < numberOfEvents; ++iEvent) {
          // We have to do some nasty casting here, because the Python C-Api uses structs which are binary compatible
          // to a PyObject but do not inherit from it!
          probabilities[iEvent] = static_cast<float>(*static_cast<float*>(PyArray_GETPTR1(reinterpret_cast<PyArrayObject*>(result.ptr()),
                                                     iEvent)));
        }
      } catch (...) {
        PyErr_Print();
        PyErr_Clear();
        B2ERROR("Failed calling applying PythonExpert");
        throw std::runtime_error("Failed calling applying PythonExpert");
      }

      return probabilities;
    }
  }
}
