/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * ECLLocalRunCalibAcc                                                    *
 *                                                                        *
 * Feature (mean value, standard deviation and number of accepted         *
 * events) accumulator.                                                   *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su),               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef _ECLLocalRunCalibAcc_H_
#define  _ECLLocalRunCalibAcc_H_
// STL
#include <vector>
// BOOST
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
namespace bacc = boost::accumulators;
namespace Belle2 {
  /**
   * ECLLocalRunCalibAcc is the
   * class designed to accumulate
   * mean values, standard deviation
   * and number of accepted events.
   */
  class ECLLocalRunCalibAcc {
  public:
    /**
     * Constructor.
     * @param min_value is the lower value limit.
     * @param max_value is the upper value limit
     * @param ndevs is the number of standard deviations.
     used to update value limits.
     */
    ECLLocalRunCalibAcc(
      const float& min_value,
      const float& max_value,
      const int* const ndevs);
    /**
     * Destructor.
     */
    ~ECLLocalRunCalibAcc();
    /**
     * Get total number of events.
     */
    int getNOfEvents() const;
    /**
     * Get number of accepted
     * events.
     */
    int getCount() const;
    /**
     * Get mean value.
     */
    float getMean() const;
    /**
     * Get standard deviation.
     */
    float getStdDev() const;
    /**
     * Add value.
     */
    void add(const float& value);
    /**
     * Calculate mean value, standard
     * deviation and number of accepted
     * events.
     */
    void calc();
  private:
    /**
     * Check value.
     * @param value is the amplitude or
     time value
     */
    bool isValueInRange(const float& value) const;
    /**
     * Calculate standard deviation
     * using variance and number of
     * accepted events.
     * @param variance is the variance.
     * @param count is the number of
     accepted events.
     */
    float calcStdDev(const float& variance,
                     const int& count) const;
    /**
     * Update value limits.
     * @param mean is the mean value.
     * @param stddev is the standard
     deviation.
     */
    void updateLimits(const float& mean,
                      const float& stddev);
    /**
     * Lower value limit.
     */
    float m_leftLimit;
    /**
     * Upper value limit.
     */
    float m_rightLimit;
    /**
     * Number of standard
     * deviations used to
     * update value limits.
     */
    const int* const c_ndevs;
    /**
     * Total number of events.
     */
    int m_nevents;
    /**
     * Number of accepted
     * events
     */
    int m_count;
    /**
     * Mean value.
     */
    float m_mean;
    /**
     * Standard deviation.
     */
    float m_stddev;
    /**
     * Vector of accepted values.
     */
    std::vector<int> m_data;
  };
}
#endif
