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

#include <memory>

#include <TDirectory.h>
#include <TNamed.h>

#include <framework/core/HistoModule.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <calibration/dataobjects/CalibRootObjBase.h>
#include <calibration/dataobjects/CalibRootObjNew.h>
#include <calibration/CalibObjManager.h>
#include <calibration/Utilities.h>

namespace Belle2 {
  /**
   * Calibration collector module base class
   */
  class CalibrationCollectorModuleNew: public HistoModule {

  public:
    /// Constructor. Sets the default prefix for calibration dataobjects
    CalibrationCollectorModuleNew();
    /// Virtual destructor (base class)
    virtual ~CalibrationCollectorModuleNew() {}

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

    void defineHisto();

    /// Register object with a name, takes ownership, do not access the pointer beyond prepare()
    template <class T>
    void registerObject(std::string name, T* obj)
    {
      std::unique_ptr<CalibRootObjBase> calObj(new CalibRootObjNew<T>(obj));
      calObj->SetName(name.c_str());
      m_manager.addObject(name, std::move(calObj));
    }

    template<class T>
    T& getObject(std::string name)
    {
      Belle2::Calibration::KeyType expRun = std::make_pair(m_emd->getExperiment(), m_emd->getRun());
      return *(m_manager.getObject<T>(name, expRun));
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

    virtual void inDefineHisto() {}

    TDirectory* m_dir;

    CalibObjManager m_manager;

    StoreObjPtr<EventMetaData> m_emd;
  };
} // Belle2 namespace
