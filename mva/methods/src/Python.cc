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
#include <numeric>

namespace Belle2 {
  namespace MVA {

    void PythonOptions::load(const boost::property_tree::ptree& pt)
    {
      int version = pt.get<int>("Python_version");
      if (version != 1) {
        B2ERROR("Unkown weightfile version " << std::to_string(version));
        throw std::runtime_error("Unkown weightfile version " + std::to_string(version));
      }
      m_framework = pt.get<std::string>("Python_framework");
      m_steering_file = pt.get<std::string>("Python_steering_file");
      m_mini_batch_size = pt.get<unsigned int>("Python_mini_batch_size");
      m_nIterations = pt.get<unsigned int>("Python_n_iterations");
      m_config = pt.get<std::string>("Python_config");
      m_training_fraction = pt.get<double>("Python_training_fraction");

    }

    void PythonOptions::save(boost::property_tree::ptree& pt) const
    {
      pt.put("Python_version", 1);
      pt.put("Python_framework", m_framework);
      pt.put("Python_steering_file", m_steering_file);
      pt.put("Python_mini_batch_size", m_mini_batch_size);
      pt.put("Python_n_iterations", m_nIterations);
      pt.put("Python_config", m_config);
      pt.put("Python_training_fraction", m_training_fraction);
    }

    po::options_description PythonOptions::getDescription()
    {
      po::options_description description("Python options");
      description.add_options()
      ("framework", po::value<std::string>(&m_framework),
       "Framework which should be used. Currently supported are sklearn, tensorflow and theano")
      ("model", po::value<std::string>(&m_steering_file), "Steering file which describes")
      ("mini_batch_size", po::value<unsigned int>(&m_mini_batch_size), "Size of the mini batch given to partial_fit function")
      ("n_iterations", po::value<unsigned int>(&m_nIterations), "Number of iterations")
      ("training_fraction", po::value<double>(&m_training_fraction),
       "Training fraction used to split up dataset in training and validation sample.")
      ("config", po::value<std::string>(&m_config), "Json encoded python object passed to begin_fit function");
      return description;
    }

    /**
    Singleton class which handles the initalization and finalization of Python
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
        Helper funtion which initalizes array system of numpy.
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

    boost::python::object get_attr_from_module_else_fallback_to_framework(const std::string& attrName, boost::python::object module,
        boost::python::object framework)
    {
      if (PyObject_HasAttrString(module.ptr(), attrName.c_str())) {
        return module.attr(attrName.c_str());
      } else {
        return framework.attr(attrName.c_str());
      }
    }

    Weightfile PythonTeacher::train(Dataset& training_data) const
    {

      Weightfile weightfile;
      std::string custom_weightfile = weightfile.getFileName();

      auto steering_path = boost::filesystem::path(m_specific_options.m_steering_file);

      try {
        auto parent_path = boost::filesystem::canonical(steering_path).parent_path();
        auto sys = boost::python::import("sys");
        sys.attr("path").attr("append")(parent_path.c_str());
      } catch (boost::filesystem::filesystem_error& e) {
        // Filesystem error is fine, this means the given steering_file was not found
        // in the current or absolute directory, however maybe we can still load the module
        // using the usual PYTHON_PATH
      } catch (...) {
        PyErr_Print();
        PyErr_Clear();
        B2ERROR("Failed calling train in PythonTeacher");
        throw std::runtime_error("Failed calling train in PythonTeacher");
      }

      uint64_t numberOfFeatures = training_data.getNumberOfFeatures();
      uint64_t numberOfEvents = training_data.getNumberOfEvents();

      uint64_t batch_size = m_specific_options.m_mini_batch_size;
      if (batch_size == 0) {
        batch_size = numberOfEvents;
      }

      uint64_t training_batch_size = batch_size;
      uint64_t validation_batch_size = 0;

      if (m_specific_options.m_training_fraction <= 0.0) {
        B2ERROR("Please provide a positive training fraction");
        throw std::runtime_error("Please provide a positive training fraction");
      }

      if (m_specific_options.m_training_fraction < 1.0) {
        training_batch_size = static_cast<uint64_t>(batch_size * m_specific_options.m_training_fraction);
        validation_batch_size = static_cast<uint64_t>(batch_size * (1 - m_specific_options.m_training_fraction));
      }

      auto X = std::unique_ptr<float[]>(new float[training_batch_size * numberOfFeatures]);
      auto y = std::unique_ptr<float[]>(new float[training_batch_size]);
      auto w = std::unique_ptr<float[]>(new float[training_batch_size]);
      npy_intp dimensions_X[2] = {static_cast<npy_intp>(training_batch_size), static_cast<npy_intp>(numberOfFeatures)};
      npy_intp dimensions_y[1] = {static_cast<npy_intp>(training_batch_size)};
      npy_intp dimensions_w[1] = {static_cast<npy_intp>(training_batch_size)};

      auto X_v = std::unique_ptr<float[]>(new float[validation_batch_size * numberOfFeatures]);
      auto y_v = std::unique_ptr<float[]>(new float[validation_batch_size]);
      auto w_v = std::unique_ptr<float[]>(new float[validation_batch_size]);
      npy_intp dimensions_X_v[2] = {static_cast<npy_intp>(validation_batch_size), static_cast<npy_intp>(numberOfFeatures)};
      npy_intp dimensions_y_v[1] = {static_cast<npy_intp>(validation_batch_size)};
      npy_intp dimensions_w_v[1] = {static_cast<npy_intp>(validation_batch_size)};


      try {
        auto json = boost::python::import("json");
        auto parameters = json.attr("loads")(m_specific_options.m_config.c_str());

        auto module = boost::python::import(steering_path.stem().c_str());
        auto framework = boost::python::import((std::string("basf2_mva_python_interface.") + m_specific_options.m_framework).c_str());

        auto model = get_attr_from_module_else_fallback_to_framework("get_model", module, framework)(numberOfFeatures, numberOfEvents,
                     parameters);
        auto state = get_attr_from_module_else_fallback_to_framework("begin_fit", module, framework)(model);

        std::vector<uint64_t> data_indices(numberOfEvents);
        std::iota(data_indices.begin(), data_indices.end(), 0);
        // disabled random shuffle since this is very slow (O(days)) on large files.
        // TODO: remove it in a more sophisticated way
        //std::random_shuffle(data_indices.begin(), data_indices.end());

        auto start_train = data_indices.begin();
        auto end_train = data_indices.begin() + static_cast<uint64_t>(numberOfEvents * m_specific_options.m_training_fraction);
        auto end_validation = data_indices.end();
        std::vector<uint64_t> training_indices(start_train, end_train);
        std::vector<uint64_t> validation_indices(end_train, end_validation);

        uint64_t nBatches = std::floor(numberOfEvents / batch_size);
        bool continue_loop = true;
        for (uint64_t iIteration = 0; iIteration < m_specific_options.m_nIterations and continue_loop; ++iIteration) {
          // disabled random shuffle since this is very slow (O(days)) on large files.
          // TODO: remove it in a more sophisticated way
          // std::random_shuffle(training_indices.begin(), training_indices.end());
          // std::random_shuffle(validation_indices.begin(), validation_indices.end());

          for (uint64_t iBatch = 0; iBatch < nBatches and continue_loop; ++iBatch) {
            for (uint64_t iEvent = 0; iEvent < training_batch_size; ++iEvent) {
              training_data.loadEvent(training_indices[iEvent + iBatch * training_batch_size]);
              for (uint64_t iFeature = 0; iFeature < numberOfFeatures; ++iFeature)
                X[iEvent * numberOfFeatures + iFeature] = training_data.m_input[iFeature];
              y[iEvent] = training_data.m_target;
              w[iEvent] = training_data.m_weight;
            }

            for (uint64_t iEvent = 0; iEvent < validation_batch_size; ++iEvent) {
              training_data.loadEvent(validation_indices[iEvent + iBatch * validation_batch_size]);
              for (uint64_t iFeature = 0; iFeature < numberOfFeatures; ++iFeature)
                X_v[iEvent * numberOfFeatures + iFeature] = training_data.m_input[iFeature];
              y_v[iEvent] = training_data.m_target;
              w_v[iEvent] = training_data.m_weight;
            }
            // Maybe slow, create ndarrays outside of loop?
            auto ndarray_X = boost::python::handle<>(PyArray_SimpleNewFromData(2, dimensions_X, NPY_FLOAT32, X.get()));
            auto ndarray_y = boost::python::handle<>(PyArray_SimpleNewFromData(1, dimensions_y, NPY_FLOAT32, y.get()));
            auto ndarray_w = boost::python::handle<>(PyArray_SimpleNewFromData(1, dimensions_w, NPY_FLOAT32, w.get()));

            auto ndarray_X_v = boost::python::handle<>(PyArray_SimpleNewFromData(2, dimensions_X_v, NPY_FLOAT32, X_v.get()));
            auto ndarray_y_v = boost::python::handle<>(PyArray_SimpleNewFromData(1, dimensions_y_v, NPY_FLOAT32, y_v.get()));
            auto ndarray_w_v = boost::python::handle<>(PyArray_SimpleNewFromData(1, dimensions_w_v, NPY_FLOAT32, w_v.get()));

            auto r = get_attr_from_module_else_fallback_to_framework("partial_fit", module, framework)(state, ndarray_X, ndarray_y, ndarray_w,
                     ndarray_X_v, ndarray_y_v, ndarray_w_v, iIteration * nBatches + iBatch);
            boost::python::extract<bool> proxy(r);
            if (proxy.check())
              continue_loop = static_cast<bool>(proxy);
          }
        }

        auto result = get_attr_from_module_else_fallback_to_framework("end_fit", module, framework)(state);

        auto pickle = boost::python::import("pickle");
        auto builtins = boost::python::import("builtins");
        auto file = builtins.attr("open")(custom_weightfile.c_str(), "wb");
        pickle.attr("dump")(result, file);

        auto importances = get_attr_from_module_else_fallback_to_framework("feature_importance", module, framework)(state);
        if (len(importances) == 0) {
          B2INFO("Python method returned empty feature importance. There won't be any information about the feature importance in the weightfile.");
        } else if (numberOfFeatures != len(importances)) {
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
      weightfile.addSignalFraction(training_data.getSignalFraction());

      return weightfile;

    }

    PythonExpert::PythonExpert()
    {
      PythonInitializerSingleton::GetInstance();
    }


    void PythonExpert::load(Weightfile& weightfile)
    {

      std::string custom_weightfile = weightfile.getFileName();
      weightfile.getFile("Python_Weightfile", custom_weightfile);
      weightfile.getOptions(m_general_options);
      weightfile.getOptions(m_specific_options);

      try {
        auto pickle = boost::python::import("pickle");
        auto builtins = boost::python::import("builtins");
        auto file = builtins.attr("open")(custom_weightfile.c_str(), "rb");
        auto unpickled_object = pickle.attr("load")(file);

        m_framework = boost::python::import((std::string("basf2_mva_python_interface.") + m_specific_options.m_framework).c_str());
        //m_framework = boost::python::import((std::string("basf2_mva_") + m_specific_options.m_framework).c_str());
        m_state = m_framework.attr("load")(unpickled_object);
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
        for (uint64_t iFeature = 0; iFeature < numberOfFeatures; ++iFeature)
          X[iEvent * numberOfFeatures + iFeature] = test_data.m_input[iFeature];
      }

      std::vector<float> probabilities(test_data.getNumberOfEvents());

      try {
        auto ndarray_X = boost::python::handle<>(PyArray_SimpleNewFromData(2, dimensions_X, NPY_FLOAT32, X.get()));
        auto result = m_framework.attr("apply")(m_state, ndarray_X);
        for (uint64_t iEvent = 0; iEvent < numberOfEvents; ++iEvent) {
          // We have to do some nasty casting here, because the Python C-Api uses structs which are binary compatible
          // to a PyObject but do not inherit from it!
          probabilities[iEvent] = static_cast<float>(*static_cast<double*>(PyArray_GETPTR1(reinterpret_cast<PyArrayObject*>(result.ptr()),
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
