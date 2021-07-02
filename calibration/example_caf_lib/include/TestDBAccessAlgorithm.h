/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    virtual EResult calibrate() override;

  private:
    float getAverageDistanceFromAnswer();
    void generateNewPayloads();
    void reduceDistancesAndSave();
    bool m_generatePayloads = true;
    DBObjPtr<TestCalibMean> m_dbMean;
  };
} // namespace Belle2
