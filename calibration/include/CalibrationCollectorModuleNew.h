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

#include <framework/core/HistoModule.h>

#include <TDirectory.h>

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

  protected:
    /// Replacement for initialize(). Register calibration dataobjects here as well
    virtual void prepare() {}
    /// Replacement for event(). Fill you calibration data objects here
    virtual void collect() {}
    /// Replacement for beginRun(). Do anything you would normally do in beginRun here
    virtual void startRun() {}

    virtual void inDefineHisto() {}

    TDirectory* m_dir;
  };
} // Belle2 namespace
