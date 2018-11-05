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
#include <framework/database/DBObjPtr.h>
#include <calibration/dbobjects/TestCalibMean.h>

namespace Belle2 {
  /**
   * Test class implementing calibration algorithm
   */
  class TestDBAccessAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to TestCalibration
    TestDBAccessAlgorithm();

    /// Destructor
    virtual ~TestDBAccessAlgorithm() {}
    void setGeneratePayloads(const bool& value) {m_generatePayloads = value;}
    bool getGeneratePayloads() const {return m_generatePayloads;}
    void saveSameMeans();

  protected:

    /// Run algo on data
    virtual EResult calibrate();

  private:
    float getAverageDistanceFromAnswer();
    void generateNewPayloads();
    void reduceDistancesAndSave();
    bool m_generatePayloads = true;
    DBObjPtr<TestCalibMean> m_dbMean;
  };
} // namespace Belle2
