/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationCollectorModuleNew.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
//#include <calibration/dataobjects/CalibRootObj.h>
#include <TTree.h>
//#include <TDirectory.h>

namespace Belle2 {
  /// Testing module for collection of calibration data
  class TestHistoModule : public CalibrationCollectorModuleNew {

  public:

    /// Constructor: Sets the description, the properties and the parameters of the module.
    TestHistoModule();

    virtual void prepare();

    virtual void collect();

  private:

    StoreObjPtr<EventMetaData> m_emd;

    TTree* m_tree;

    /** Current event id */
    int m_evt = -1;
    /** Current run id */
    int m_run = -1;
    /** Current experiment id */
    int m_exp = -1;
    /** Fake coordinates of a hit */
    double m_hitX = 0.0;
    double m_hitY = 0.0;
    double m_hitZ = 0.0;

    double m_trackX = 0.0;
    double m_trackY = 0.0;
    double m_trackZ = 0.0;
    double m_chisq = 0.0;
    double m_pvalue = 0.0;
    double m_dof = 0.0;

    /** Number of entries created in the saved tree per event. Will affect the total size of objects stored */
    int m_entriesPerEvent;

  };
}
