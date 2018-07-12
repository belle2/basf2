/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 *                                                                        *
 * ECLLocalRunCalibQualityChecker                                         *
 *                                                                        *
 * This class has been designed in order to check quality of local run    *
 * calibration.                                                           *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su) (BINP)         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
// ECL
#include <ecl/dbobjects/ECLCrystalLocalRunCalib.h>
namespace Belle2 {
  /**
   * The ECLLocalRunCalibQualityChecker
   * class has been developed in order
   * to check quality of local runs.
   */
  class ECLLocalRunCalibQualityChecker {
  public:
    /**
     * Constructor.
     * @param isTime is true in the case of
     time calibration quality check and is
     false in the case of amplitude calibration
     check.
     */
    ECLLocalRunCalibQualityChecker(bool isTime);
    /**
     * Destructor.
     */
    ~ECLLocalRunCalibQualityChecker();
    /**
     * Used to check calibration
     * quality.
     * @param payload is database
     object for amplitude or time calibration
     results.
     * @param ref_payload is database
     object for amplitude or time calibration
     results from the reference run.
     */
    bool checkQuality(
      const ECLCrystalLocalRunCalib* const payload,
      const ECLCrystalLocalRunCalib* const ref_payload);
    /**
     * m_minOfEvs setter.
     * @param minNOfEvs is the reference
     to input value.
     */
    void setMinNOfEvs(const int& minNOfEvs);
    /**
     * m_badCountRatio setter.
     * @param badCountRatio is the
     reference to input value.
     */
    void setBadCountRatio(const float& badCountRatio);
    /**
     * m_largeOffsetValue setter.
     * @param largeOffsetValue is the
     reference to input value.
     */
    void setBadOffsetValue(const float& largeOffsetValue);
    /**
     * Maimum and minimum
     * allowed mean value limits
     * setter.
     * @param minMeanValue is the minimum allowed
     value.
     * @param maxMeanValue is the maximum allowed
     value.
     */
    void setMeanValueLimits(const float& minMeanValue,
                            const float& maxMeanValue);
    /**
     * Reset check values.
     */
    void reset();
    /**
     * Getter for m_badCountRatio.
     */
    int getNBadCountCh() const;
    /**
     * Getter for m_largeOffsetValue.
     */
    int getNLargeOffsetCh() const;
    /**
     * Getter for m_nChOutsideOfLimits
     */
    int getNChOutsideLimits() const;
    /**
     * Getter for m_tooSmallNOfEvs
     */
    bool isTooSmallNOfEvents() const;
    /**
     * Getter for m_isNegAmpl
     */
    bool isNegAmpl() const;
  private:
    /**
     * True in the case, when the
     * time calibration quality is
     * checking, and false otherwise.
     */
    bool m_isTime;
    /**
     * Minimum allowed number of events in
     * the calibration run.
     */
    int m_minNOfEvs;
    /**
     * Minimum allowed ratio
     * between number of accepted
     * events and total number of
     * events in the considered run.
     */
    float m_badCountRatio;
    /**
     * Maximum offset value
     */
    float m_largeOffsetValue;
    /**
     * Minimum allowed mean value.
     */
    float m_minMeanValue;
    /**
     * Maximum allowed mean value.
     */
    float m_maxMeanValue;
    /**
     * Number of channels, where
     * the ratio between number of
     * accepted events and total
     * number of events in the considered
     * run is smaller than m_badCountRatio.
     */
    int m_nBadCountCh;
    /**
     * Number of channels, where
     * offset is larger than m_nBadCountCh.
     */
    int m_nLargeOffsetCh;
    /**
     * Number of channels with
     * mean values outside of
     * the allowed limits (m_minMeanValue,
     * m_maxMeanValue).
     */
    int m_nChOutsideOfLimits;
    /**
     * True if the number of events
     * is too small, i.e. number of
     * events is smaller than m_minNOfEvs.
     */
    bool m_tooSmallNOfEvs;
    /**
     * True if there are
     * negative amplitudes.
     */
    bool m_isNegAmpl;
    /**
     * Check number of events.
     * @param payload is a locacal run
     calibration database object.
     */
    bool checkNOfEvs(const ECLCrystalLocalRunCalib* const payload);
    /**
     * Check ratio between number of accepted events and
     * total number of events in the considered run.
     * @param payload is a local run calibration
     database object.
     */
    bool checkCount(const ECLCrystalLocalRunCalib* const payload);
    /**
     * Check mean value offset.
     * @param payload is a local run calibration
     database object.
     * @param ref_payload is a local run calibration
     database object corresponding to a reference run.
     */
    bool checkOffset(
      const ECLCrystalLocalRunCalib* const payload,
      const ECLCrystalLocalRunCalib* const ref_payload);
    /**
     * Check value limits.
     * @param payload is a local run calibration database object.
     */
    bool checkValueLimits(const ECLCrystalLocalRunCalib* const payload);
    /**
     * Check if there are negative amplitudes.
     * @param payload is a local run calibration database object,
     which contains amplitude calibration results.
     */
    bool checkNegAmpls(const ECLCrystalLocalRunCalib* const payload);
  };
}
