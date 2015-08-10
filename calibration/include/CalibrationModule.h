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

#include <framework/database/IntervalOfValidity.h>

namespace Belle2 {
  namespace calibration {
    typedef IntervalOfValidity ExpRunRange;
    /**
     * Calibration module base class
     */
    class CalibrationModule: public Module {

    public:
      /** State of the CalibrationModule */
      enum ECalibrationModuleState {
        c_Waiting,
        c_Running,
        c_Monitoring,
        c_Done,
        c_Failed,
        c_Blocked
      };
      /** Result of Calibrate() */
      enum ECalibrationModuleResult {
        c_Success,
        c_NotEnoughData,
        c_NoChange,
        c_Failure
      };
      /** Result of Monitor() */
      enum ECalibrationModuleMonitoringResult {
        c_MonitoringSuccess,
        c_MonitoringIterateCalibration,
        c_MonitoringIterateMonitor
      };

      /**  Typedef for module dependencies pairs (module_name, module_mode). */
      typedef std::pair<std::string, ECalibrationModuleState> ModuleDependency;

      /**  Constructor. */
      CalibrationModule();
      /** Destructor. */
      virtual ~CalibrationModule();
      /** Initialize the Module. */
      void initialize() final;
      /** This method is called for each event. */
      void event() final;
      /** This method is called if the run begins. */
      void beginRun() final;
      /** This method is called if the current run ends. */
      void endRun() final;
      /** This method is called at the end of the event processing. */
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
      /** A function to close files in parallel processing before calibration. */
      virtual void closeParallelFiles() {};
      virtual void storeData() {};
      virtual void resetData() {};
      virtual void loadData() {};
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
       * Get the dependencies of the module.
       * @return Vector of module dependencies
       */
      const std::vector<ModuleDependency>& getDependencies() const { return m_dependencies; }

      /**
       * Get current state of the module.
       * @return Current module state if active, c_Blocked else
       */
      ECalibrationModuleState getState();
      /**
       * Get current number of iterations (for active IOV).
       * @return Number of iterations performed so far if active,
       * 0 otherwise
       */
      int getNumberOfIterations();
      /** Increase number of iterations for active module state */
      void increaseIterations();

      /**
       * Get state of the module for its
       * particular calibration IOV.
       * @return Module state for given IOV, c_Blocked if not found
       */
      ECalibrationModuleState getState(const ExpRunRange& iov);
      /**
       * Get number of iterations for give calibration IOV.
       * @return Number of iterations performed so far in IOV,
       * 0 otherwise (not found or no iterations so far finished)
       */
      int getNumberOfIterations(const ExpRunRange& iov);

      /**
       * @brief Set the state of the module.
       *
       * @return void
       */
      void setState(ECalibrationModuleState state);

      /**
       * @brief Convert state from string to enum.
       *
       * @param state Module state
       * @return Belle2::calibration::CalibrationModule::ECalibrationModuleState
       */
      static ECalibrationModuleState stringToState(std::string state);

      static std::string stateToString(ECalibrationModuleState state);

      void setState(ExpRunRange iov, ECalibrationModuleState state, int iteration);

      std::vector<ExpRunRange> getCalibrationIOVs();

      bool isCalibrator() const { return m_isCalibrator; }
      bool isCollector() const { return m_isCollector; }



      static const std::map<ECalibrationModuleState, std::string> m_stateNames;
    protected:

      /**
       * Add default list of module dependencies as parameter to the module
       * Parameter name, description and binding to variable is done automatically.
       */
      void addDefaultDependencyList(std::string list = "");
      /** Get the calibration file. */
      TFile* getCalibrationFile();


      /** Get the calibration file name. */
      std::string getCalibrationFileName() const
      {
        return m_calibrationFileName;
      }

      ExpRunRange m_calibration_iov;

      std::string getBaseName() const { return m_baseName; }

    private:


      /** Try to start calibration. */
      bool tryStartCalibration();

      /** Calibration mode: online or offline. */
      std::string m_mode;

      /** Calibration parameter: category of dataset. */
      std::string m_datasetCategory;

      /** Calibration parameter: granularity of calibration (run, experiment, or other). */
      std::string m_granularityOfCalibration;

      /** Calibration parameter: maximal number of iterations. */
      int m_numberOfIterations;

      /** Calibration parameter: name of calibration file. */
      std::string m_calibrationFileName;

      /** Dependencies of the module to be set on initialization */
      std::vector< ModuleDependency > m_dependencies;

      /** Calibration parameter: dependencies on other calibration modules. */
      std::string m_dependencyList;

      /** File for the calibration. */
      TFile* m_calibrationFile = nullptr;
      std::vector< std::tuple< ExpRunRange, ECalibrationModuleState, int> > m_states = {};
      int m_currentStateIndex = -1;
      bool m_isCollector = true;
      bool m_isCalibrator = true;

      std::string m_baseName = "";

      long int m_lastRun = -1;
    };
  }
} //Belle2 namespace

