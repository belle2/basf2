/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: David Dossett                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {
  // Test DBObject
  class TestCalibMean: public TObject {

  public:

    // Default constructor
    TestCalibMean(): m_mean(0), m_meanError(0) {};

    // Constructor
    TestCalibMean(float mean, float meanError): m_mean(mean), m_meanError(meanError) {};

    // Could extend this class with more methods to access/calculate things.
    float getMean() const {return m_mean;};
    float getMeanError() const {return m_meanError;};

    void setMean(float mean) {m_mean = mean;};
    void setMeanError(float meanError) {m_meanError = meanError;};
  private:

    float m_mean;
    float m_meanError;

    ClassDef(TestCalibMean, 1); /**< ClassDef */
  };
} // end namespace Belle2
