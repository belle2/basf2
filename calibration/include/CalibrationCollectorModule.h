/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka (tadeas.bilka@gmail.com)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>
#include <vector>
#include <TFile.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <calibration/dataobjects/CalibRootObj.h>

#include <TH1.h>
#include <TTree.h>
#include <alignment/dataobjects/MilleData.h>

namespace Belle2 {
  namespace calibration {
    /**
     * Calibration collector module base class
     */
    class CalibrationCollectorModule: public Module {

    public:
      /// Constructor. Sets the default prefix for calibration dataobjects
      CalibrationCollectorModule();
      /// Virtual destructor (base class)
      virtual ~CalibrationCollectorModule() {}

      /// Set up a default RunRange object in datastore and call prepare()
      void initialize() final;
      /// Check current experiment and run and update if needed, fill into RunRange and collect()
      void event() final;

    protected:
      /// Replacement for initialize(). Register calibration dataobjects here as well
      virtual void prepare() {}
      /// Replacement for event(). Fill you calibration data objects here
      virtual void collect() {}

      /// Register object with name, takes ownership, do not access the pointer beyond prepare()
      template <class T>
      void registerObject(string name, T* obj)
      {
        string fullName = m_prefix + "_" + name;

        StoreObjPtr<CalibRootObj<T>> storeobj(fullName, DataStore::c_Persistent);
        storeobj.registerInDataStore();

        if (storeobj.isValid()) {
          B2WARNING("Replacing existing calibration data object internal template. Potentially dangerous...");
          storeobj->replaceObject(obj);
        } else
          storeobj.construct(obj);
      }

      /// Get object valid for current experiment and run by its name
      template <class T>
      T& getObject(string name)
      {
        std::string strExpRun = std::to_string(m_currentExpRun.first) + "." + std::to_string(m_currentExpRun.second);
        std::string fullName = m_prefix + "_" + name;
        StoreObjPtr<CalibRootObj<T>> storeobj(fullName, DataStore::c_Persistent);
        return storeobj->getObject(strExpRun);
      }

    private:
      /// The prefix for calibration datastore objects
      std::string m_prefix;
      /// Current exp, run for correct object retrieval/creation
      std::pair<int, int> m_currentExpRun = { -999, -999};

      /// Granularity of data collection = run|all(= no granularity, exp,run=-1,-1)
      std::string m_granularity;
    };
  } // calibration namespace
} // Belle2 namespace

