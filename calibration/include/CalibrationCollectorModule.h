/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka (tadeas.bilka@gmail.com)                    *
 *               David Dossett (david.dossett@unimelb.edu.au)             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <memory>
#include <utility>

#include <TDirectory.h>
#include <TFile.h>
#include <TRandom.h>

#include <framework/core/HistoModule.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>

#include <calibration/dataobjects/RunRange.h>
#include <calibration/CalibObjManager.h>
#include <calibration/Utilities.h>

namespace Belle2 {
  /**
   * Calibration collector module base class
   */
  class CalibrationCollectorModule: public HistoModule {

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
    /// Write the current collector objects to a file and clear their memory
    void endRun() final;
    /// Write the final objects to the file
    void terminate() final;
    /// Runs due to HistoManager, allows us to discover the correct file
    void defineHisto() final;

    /// Register object with a name, takes ownership, do not access the pointer beyond prepare()
    template <class T>
    void registerObject(std::string name, T* obj)
    {
      std::shared_ptr<T> calObj(obj);
      calObj->SetName(name.c_str());
      m_manager.addObject(name, calObj);
    }

    /// Calls the CalibObjManager to get the requested stored collector data
    template<class T>
    T* getObjectPtr(std::string name)
    {
      return m_manager.getObject<T>(name, m_expRun);
    }

  protected:
    /// Replacement for initialize(). Register calibration dataobjects here as well
    virtual void prepare() {}
    /// Replacement for event(). Fill you calibration data objects here
    virtual void collect() {}
    /// Replacement for beginRun(). Do anything you would normally do in beginRun here
    virtual void startRun() {}
    /// Replacement for endRun(). Do anything you would normally do in endRun here.
    virtual void closeRun() {}
    /// Replacement for terminate(). Do anything you would normally do in terminate here.
    virtual void finish() {}
    /// Replacement for defineHisto(). Do anything you would normally do in defineHisto here.
    virtual void inDefineHisto() {}

    /// The top TDirectory that collector objects for this collector will be stored beneath
    TDirectory* m_dir;

    /// Controls the creation, collection and access to calibration objects
    CalibObjManager m_manager;

    /// Overall list of runs processed
    RunRange* m_runRange;

    /// Current ExpRun for object retrieval (becomes -1,-1 for granularity=all)
    Calibration::ExpRun m_expRun;

    /// Current EventMetaData
    StoreObjPtr<EventMetaData> m_emd;

  private:
    /****** Module Parameters *******/
    /// Granularity of data collection = run|all(= no granularity, exp,run=-1,-1)
    std::string m_granularity;
    /// Maximum number of events to be collected at the start of each run (-1 = no maximum)
    int m_maxEventsPerRun;
    /// Prescale module parameter, this fraction of events will have collect() run on them [0.0 -> 1.0]
    float m_preScale;
    /********************************/

    StoreObjPtr<EventMetaData> m_evtMetaData; /**< Required input for EventMetaData */

    /// Whether or not we will run the collect() at all this run, basically skips the event() function if false
    bool m_runCollectOnRun = true;
    /** How many events processed for each ExpRun so far, stops counting up once max is hit
      * Only used/incremented if m_maxEventsPerRun > -1
      */
    std::map<Calibration::ExpRun, int> m_expRunEvents;
    /// Will point at correct value in m_expRunEvents
    int* m_eventsCollectedInRun;
    /** I'm a little worried about floating point precision when comparing to 0.0 and 1.0 as special values.
      * But since a user will have set them (or left them as default) as exactly equal to 0.0 or 1.0 rather
      * than calculating them in almost every case, I think we can assume that the equalities hold.
      */
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
