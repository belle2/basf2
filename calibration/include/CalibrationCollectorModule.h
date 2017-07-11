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
#include <TFile.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <calibration/dataobjects/CalibRootObj.h>

#include <TH1.h>
#include <TTree.h>
#include <TRandom.h>
#include <alignment/dataobjects/MilleData.h>

namespace Belle2 {
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
    /// Reset the m_runCollectOnRun flag, if necessary, to begin collection again
    void beginRun() final;

  protected:
    /// Replacement for initialize(). Register calibration dataobjects here as well
    virtual void prepare() {}
    /// Replacement for event(). Fill you calibration data objects here
    virtual void collect() {}
    /// Replacement for beginRun(). Do anything you would normally do in beginRun here
    virtual void startRun() {}

    /// Register object with name, takes ownership, do not access the pointer beyond prepare()
    template <class T>
    void registerObject(std::string name, T* obj)
    {
      std::string fullName = getName() + "_" + name;

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
    T& getObject(std::string name)
    {
      std::string strExpRun = std::to_string(m_currentExpRun.first) + "." + std::to_string(m_currentExpRun.second);
      std::string fullName = getName() + "_" + name;
      StoreObjPtr<CalibRootObj<T>> storeobj(fullName, DataStore::c_Persistent);
      return storeobj->getObject(strExpRun);
    }

  private:
    /// Current exp, run for correct object retrieval/creation
    std::pair<int, int> m_currentExpRun = { -999, -999};

    /****** Module Parameters *******/
    /// Granularity of data collection = run|all(= no granularity, exp,run=-1,-1)
    std::string m_granularity;
    /// Maximum number of events to be collected at the start of each run (-1 = no maximum)
    int m_maxEventsPerRun;
    /// Prescale module parameter, this fraction of events will have collect() run on them [0.0 -> 1.0]
    float m_preScale;
    /********************************/

    /// Whether or not we will run the collect() at all this run, basically skips the event() function if false
    bool m_runCollectOnRun = true;
    /// How many events processed for each ExpRun so far, stops counting up once max is hit
    /// Only used/incremented if m_maxEventsPerRun > -1
    std::map<std::pair<int, int>, int> m_expRunEvents;
    /// Will point at correct value in m_expRunEvents
    int* m_eventsCollectedInRun;

    /// I'm a little worried about floating point precision when comparing to 0.0 and 1.0 as special values.
    /// But since a user will have set them (or left them as default) as exactly equal to 0.0 or 1.0 rather
    /// than calculating them in almost every case, I think we can assume that the equalities hold.
    bool getPreScaleChoice()
    {
      if (m_preScale == 1.) {
        return true;
      } else if (m_preScale == 0.) {
        return false;
      } else {
        const double randomNumber = gRandom->Uniform();
        return randomNumber < m_preScale;
      }
    }
  };
} // Belle2 namespace

