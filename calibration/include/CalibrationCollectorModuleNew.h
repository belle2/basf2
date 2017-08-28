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

#include <TDirectory.h>
#include <TNamed.h>

#include <framework/core/HistoModule.h>
#include <calibration/dataobjects/CalibRootObjBase.h>
#include <calibration/dataobjects/CalibRootObjNew.h>

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

    void defineHisto();

    /// Register object with a name, takes ownership, do not access the pointer beyond prepare()
    template <class T>
    void registerObject(std::string name, T* obj)
    {
      m_object = static_cast<CalibRootObjBase*>(new CalibRootObjNew<T>(obj));
      m_object->SetName(name.c_str());
      m_dir->Add(m_object);
    }

    template<class T>
    T* getObject()
    {
      CalibRootObjNew<T>* calObj = static_cast<CalibRootObjNew<T>*>(m_object);
      return static_cast<T*>(calObj->getObject());
    }

  protected:
    /// Replacement for initialize(). Register calibration dataobjects here as well
    virtual void prepare() {}
    /// Replacement for event(). Fill you calibration data objects here
    virtual void collect() {}
    /// Replacement for beginRun(). Do anything you would normally do in beginRun here
    virtual void startRun() {}

    virtual void inDefineHisto() {}

    TDirectory* m_dir;

    CalibRootObjBase* m_object;
  };
} // Belle2 namespace
