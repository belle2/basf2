/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Local run Digit Calibration.                                           *
 *                                                                        *
 * This class wraps Boost accumulator to provide mean and standard        *
 * deviation for repeated measurements (either amplitude or time          *
 * measurements) concerning single specific crystal in ECL.               *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su),               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
// STL
#include <vector>
// BOOST
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
namespace bacc = boost::accumulators;

namespace Belle2 {
  /**
   * Class to aggregate data from test pulse events, and
   * calculate integral parameters for further analysis.
   */
  class ECLLocalRunCalibrationData {
  public:
    /**
     * Constructor.
     * @param minValue the lower limit
     * @param maxValue the upper limit
     * @param nOfInitEvs the number of events
     used to initialize value limits
     * @param maxNOfDevs the number of standard
     deviations used to calculate value limits
     */
    ECLLocalRunCalibrationData(
      const float& minValue,
      const float& maxValue,
      const int& nOfInitEvs,
      const int& maxNOfDevs);
    /**
     * Destructor
     */
    ~ECLLocalRunCalibrationData();
    /**
     * Returns number of
     * accepted inputs
     */
    int getNOfAcceptedInputs() const;
    /**
     * Returns number of
     * rejected inputs
     */
    int getNOfRejectedInputs() const;
    /**
     * Returns mean value
     */
    float getMean() const;
    /**
     * Returns standard deviation
     */
    float getStdDev() const;
    /**
     * Accamulates a value,
     * if it is within lower
     * and upper limits.
     * @param value a value
     */
    void add(const float& value);
    /**
     * Stop accumulate
     */
    void stop();
    /**
     * Enable offset flag
     */
    void enableOffsetFlag();
    /**
     * Get offset flag
     */
    bool getOffsetFlag() const;

  private:
    /**
     * Represents the status of median
     * calculation
     */
    bool m_isMedianNotCalculated;
    /**
     * Number of all inputs
     */
    int m_nTotalInputs;
    /**
     * Offset counter
     */
    bool m_isOffset;
    /**
     * Accumulator
     */
    bacc::accumulator_set <
    float,
    bacc::features <
    bacc::tag::count,
    bacc::tag::mean,
    bacc::tag::variance >> m_acc;
    /**
     * Lower value limit
     */
    float m_leftLimit;
    /**
     * Upper value limit
     */
    float m_rightLimit;
    /**
     * Number of events
     * used to initialize
     * value limits
     */
    const int& c_nOfInitEvs;
    /**
     * Number of standard
     * deviations used to
     * calculate value limits
     */
    const int& c_maxNOfDevs;
    /**
     * Initial values to calculate
     * lower and upper value limits
     */
    std::vector<float> m_initValues;
    /**
     * Calculates medain value
     */
    void calcMedian();
    /**
     * Updates mean value and
     * sum of deviation squares
     */
    void update(const float&);
    /**
     * Returns true, if value
     * is between lower and upper
     * limits, and false otherwise
     * @param value a value
     */
    bool isValueInRange(const float& value) const;
    /**
     * Process
     */
    void processInitialEvents();
  };
}  // namespace Belle2

