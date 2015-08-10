/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka (tadeas.bilka@gmail.com)                    *
 *               Sergey Yashchenko (sergey.yaschenko@desy.de)             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CALIBRATIONMANAGER_H
#define CALIBRATIONMANAGER_H

#include <framework/pcore/ProcHandler.h>
#include <calibration/CalibrationModule.h>

#include <framework/database/IntervalOfValidity.h>

namespace Belle2 {
  namespace calibration {
    typedef IntervalOfValidity ExpRunRange;
    /**
     * A base (singleton) class for the calibration framework.
     * Should communicate with individual calibration modules.
     *
     * Calibration manager provides an interface for individual calibration/alignment modules
     * in the common calibration framework. It is designed as a singleton.
     */
    class CalibrationManager {
      //---------------------------------------------------------------------------------------
      // Singleton implementation
      //---------------------------------------------------------------------------------------

      /**
       * Static function to get reference to the instance of the singleton
       *
       * Should be thread-safe: theInstance initialized at the first call
       * and for sure only once (other threads should wait only for
       * the first time Instance is called)
       * Destroyed at program termination with other static variables (LIFO).
       *
       * After destruction, the CalibrationManager can NOT be "resurrected"
       *
       * NEVER try to call CalibrationManager::Instance in some destructor!
       * If your destructor would be called upon program exit, the singleton
       * might already be destroyed (if the singleton was created after your object).
       *
       * It must also be initialized from the main process, otherwise I am not
       * sure what could happen if some other thread (which would initialize it)
       * terminates, while other ones would still like to access it.
       *
       * I prefer to keep the singleton implementation rather simple
       * and only make it more complex if really needed.
       *
       * This implementation uses the Meyers' way of static class instance inside
       * the getInstance function.
       * In addition there is a check for destroyed singleton.
       * See A. Alexandrescu's Modern C++ ...
       *
       * @return A reference to an instance of this class.
       */
    public:
      static CalibrationManager& getInstance();
    private:
      /** The constructor is hidden to avoid that someone creates an instance of this class. */
      CalibrationManager();
      /** Delete the copy constructor. */
      CalibrationManager(const CalibrationManager&) = delete;
      /** Delete the copy assignment operator. */
      CalibrationManager& operator=(const CalibrationManager&) = delete;
      /** The destructor of the CalibrationManager class. */
      ~CalibrationManager();
      /** Flag to check if the singleton was already destroyed */
      static bool m_calibrationManagerDestroyed;
      //---------------------------------------------------------------------------------------

    public:

      /**
       * @brief Register CalibrationModule in the CalibrationManager
       * Has to be called from module initialize() method
       *
       * @param module The calling module pointer (this)
       * @return void
       */
      void register_module(CalibrationModule* module);
      /**
       * @brief Check that dependencies for a module are OK
       *
       * @param module The module for which dependencies are checked (this)
       * @param ignoreStates Check states of modules for dependencies? True: states are checked. False: Only dependence on particular is checked (no matter its state)
       * @return bool True: dependencies OK. False: not OK.
       */
      bool checkDependencies(CalibrationModule* module, const ExpRunRange& iov);

      /**
       * @brief Get current number of iterations
       *
       * @return int Number of iterations performed so far
       */
      int getNumberOfIterations() const;

      /**
       * @brief Set number of iterations
       * TODO: Make private?
       * @param numberOfIterations Number of iterations
       * @return void
       */
      void setNumberOfIterations(int numberOfIterations);

      void loadCachedState(std::string moduleName, ExpRunRange iov, CalibrationModule::ECalibrationModuleState& state, int& iteration);
      void saveCachedStates();

      bool done();

      void setRange(ExpRunRange iov) {m_calibration_range = iov;}
      ExpRunRange getFullRange() {return m_calibration_range;}
      void setStateFileName(std::string filename) {m_stateFileName = filename;}

      void setJobType(bool collection, bool calibration)
      {
        m_doCollection = collection;
        m_doCalibration = calibration;
      }
      bool doCollection() { return m_doCollection; }
      bool doCalibration() { return m_doCalibration; }
    private:

      ExpRunRange m_calibration_range{0, 0, -1, -1};
      /** Number of iterations for calibration or alignment procedures */
      int m_numberOfIterations;
      /** Registered calibration modules */
      std::vector<CalibrationModule*> m_calibrationModules;

      std::string m_stateFileName = "calibration_state.txt";

      bool m_doCollection = true;
      bool m_doCalibration = true;

    };
  } //end of namespace calibration
} //end of namespace Belle2
#endif /* CALIBRATIONMANAGER_H */
