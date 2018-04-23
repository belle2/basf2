/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Tadeas Bilka                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  /**
   * Test class implementing calibration algorithm which accesses the Database Instance manually
   * It provides a few member functions that show examples of how you *could* decide to update the
   * Database instance and use the constants to construct your new ones. This is not the only way,
   * or necessarily a good idea, just some examples.
   */
  class DBAccessCalibrationAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to TestCalibration
    DBAccessCalibrationAlgorithm();

    /// Destructor
    virtual ~DBAccessCalibrationAlgorithm() {}

  protected:

    /// Run algo on data
    virtual EResult calibrate();

  private:
    void saveNewT0ForEachRunFromTTree(std::shared_ptr<TTree> ttree, StoreObjPtr<EventMetaData>& evtPtr);
    void saveNewT0ForEachRunFromRunRange(StoreObjPtr<EventMetaData>& evtPtr);
    void saveNewT0FromAverageT0(StoreObjPtr<EventMetaData>& evtPtr);

  };
} // namespace Belle2
