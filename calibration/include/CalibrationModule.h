/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka (tadeas.bilka@gmail.com)                    *
 *               Sergey Yashchenko (sergey.yaschenko@desy.de)             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>
#include <vector>
#include <TFile.h>

namespace Belle2 {
  namespace calibration {
    /**
     * Calibration module base class
     */
    class CalibrationModule: public Module {

    public:
      /**
       * @brief State of the CalibrationModule
       *
       */
      enum ECalibrationModuleState {
        c_Waiting,
        c_Running,
        c_Monitoring,
        c_Done,
        c_Failed
      };
      /**
       * @brief Result of Calibrate()
       *
       */
      enum ECalibrationModuleResult {
        c_Success,
        c_NotEnoughData,
        c_NoChange,
        c_Failure
      };
      /**
       * @brief Result of Monitor()
       *
       */
      enum ECalibrationModuleMonitoringResult {
        c_MonitoringSuccess,
        c_MonitoringIterateCalibration,
        c_MonitoringIterateMonitor
      };

      /**
       * Constructor.
       */
      CalibrationModule();

      /**
       * Destructor.
       */
      virtual ~CalibrationModule();

      /**
       * Initialize the Module.
       * This method is called only once before the actual event processing starts.
       */
      void initialize() final;

      /**
       * This method is the core of the module.
       * This method is called for each event.
       * All processing of the event has to take place in this method.
       */
      void event() final;

      /**
       * This method is called if the run begins.
       */
      void beginRun() final;

      /**
       * This method is called if the current run ends.
       */
      void endRun() final;

      /**
       * This method is called at the end of the event processing.
       */
      void terminate() final;

      /**
       * This function is called in order to book ntuples, histograms and initialize data members.
       * Should be defined in the individual calibration module class derived from this base class.
       */
      virtual void Prepare();

      /**
       * This function is called in order to fill ntuples and/or histograms.
       * Should be defined in the individual calibration module class derived from this base class.
       */
      virtual void CollectData();

      /**
       * This function is called when data are accumulated and ready for calibration.
       * Should be defined in the individual calibration module class derived from this base class.
       */
      virtual ECalibrationModuleResult Calibrate();

      /**
       * This function is called in order to monitor the obtained calibration.
       * Should be defined in the individual calibration module class derived from this base class.
       */
      virtual ECalibrationModuleMonitoringResult Monitor();

      /**
       * This function is called in order to write calibration data to data base.
       * Should be defined in the individual calibration module class derived from this base class.
       */
      virtual bool StoreInDataBase();

      /**
       * @brief Typedef for module dependencies pairs (module_name, module_mode).
       *
       */
      typedef std::pair<std::string, ECalibrationModuleState> ModuleDependency;

      /**
       * @brief Get the dependencies of the module.
       *
       * @return const std::vector< Belle2::calibration::CalibrationModule::ModuleDependency, std::allocator< void > >&
       */
      const std::vector<ModuleDependency>& getDependencies() const
      {
        return m_dependencies;
      }

      /**
       * @brief Get state of the module.
       *
       * @return Belle2::calibration::CalibrationModule::ECalibrationModuleState
       */
      ECalibrationModuleState getState()
      {
        return m_state;
      }

      /**
       * @brief Set the state of the module
       *
       * @return void
       */
      void setState(ECalibrationModuleState state)
      {
        m_state = state;
      }

      /**
       * @brief Convert state from string to enum
       *
       * @param state ...
       * @return Belle2::calibration::CalibrationModule::ECalibrationModuleState
       */
      static ECalibrationModuleState stringToState(std::string state);


    protected:

      /**
       * @brief Add default list of module dependencies as parameter to the module
       * Parameter name, describtion and binding to variable is done automatically.
       *
       * @return void
       */
      void addDefaultDependencyList(std::string list = "");

      TFile* getCalibrationFile();

      virtual void closeParallelFiles() {};

      std::string getCalibrationFileName() const
      {
        return m_calibrationFileName;
      }

    private:
      bool tryStartCalibration();
      /**
       * Calibration mode: online or offline.
       */
      std::string m_mode;

      /**
       * Calibration state: waiting, running, monitoring, done, failed.
       */
      ECalibrationModuleState m_state;

      /**
       * Iteration counter.
       */
      int m_iterationNumber;

      /**
       * Calibration parameter: category of dataset.
       */
      std::string m_datasetCategory;

      /**
       * Calibration parameter: granularity of calibration (run, experiment, or other).
       */
      std::string m_granularityOfCalibration;

      /**
       * Calibration parameter: maximal number of iterations.
       */
      int m_numberOfIterations;
      /**
       * Calibration parameter: name of calibration file
       */
      std::string m_calibrationFileName;
      /**
       * True if Calibration Module is already initialized
       */
      //bool m_calibrationModuleInitialized;

      /**
       * True if Calibration Manager is already initialized
       */
      //bool m_calibrationManagerInitialized;
      /**
       * @brief Dependencies of the module to be set on initialization
       * from module parameter "Dependencies"
       */
      std::vector< ModuleDependency > m_dependencies;


      /**
       * Calibration parameter: dependencies on other calibration modules.
       */
      std::string m_dependencyList;

      TFile* m_calibrationFile;

    };
  }
} //Belle2 namespace

