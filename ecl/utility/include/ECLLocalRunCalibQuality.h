/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 *                                                                        *
 * ECLLocalRunCalibQuality                                                *
 *                                                                        *
 * This class contains information about quality of local run calibration.*
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su) (BINP)         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
namespace Belle2 {
  /**
   * The ECLLocalRunCalibQuality class
   * has been designed in order to contain
   * information about quality of local runs.
   */
  class ECLLocalRunCalibQuality {
  public:
    /**
     * Constructor.
     * @param exp is an experiment number.
     * @param run os a run number.
     * @param runRef is a reference run number.
     */
    ECLLocalRunCalibQuality(const int& exp,
                            const int& run,
                            const int& runRef);
    /**
     * Copy constructor.
     */
    ECLLocalRunCalibQuality(
      const ECLLocalRunCalibQuality& quality);
    /**
     * Destructor.
     */
    ~ECLLocalRunCalibQuality();
    /**
     * Getter for experiment number.
     */
    int getExpNumber() const;
    /**
     * Getter for run number.
     */
    int getRunNumber() const;
    /**
     * Getter for reference run number.
     */
    int getReferenceRunNumber() const;
    /**
     * Getter for number of events.
     */
    int getNOfEvents() const;
    /**
     * Getter for number of channels,
     * where the number of events used
     * to calculate mean time is too small.
     */
    int getNOfBadTimeCountCh() const;
    /**
     * Getter for number of channels,
     * where the number of events used
     * to calculate mean amplitude is too small.
     */
    int getNOfBadAmplCountCh() const;
    /**
     * Getter for number of channels,
     * where the offset of mean time
     * is too large.
     */
    int getNOfLargeTimeOffsetCh() const;
    /**
     * Getter for number of channels,
     * where the offset of mean amplitude
     * is too large.
     */
    int getNOfLargeAmplOffsetCh() const;
    /**
     * Getter for number of channels,
     * where mean time value is outside of
     * allowed limits.
     */
    int getNOfAbsMeanTimeChOutRange() const;
    /**
     * Getter for number of channels,
     * where mean amplitude value is outside of
     * allowed limits.
     */
    int getNOfAbsMeanAmplChOutRange() const;
    /**
     * Check, if the number of events
     * is too small.
     */
    bool isTooSmallNOfEvs() const;
    /**
     * Check is there are negative amplitudes.
     */
    bool isNegAmpl() const;
    /**
     * Setter for number of events.
     * @param nOfEvents is the number of events.
     */
    void setNumberOfEvents(const int& nOfEvents);
    /**
     * m_nOfBadTimeCountCh setter.
     * @param nOfBadTimeCountCh is the reference to
     input value.
     */
    void setNOfBadTimeCountCh(const int& nOfBadTimeCountCh);
    /**
     * m_nOfBadAmplCountCh setter.
     * @param nOfBadAmplCountCh is the reference to input
     value.
     */
    void setNOfBadAmplCountCh(const int& nOfBadAmplCountCh);
    /**
     * m_nOfLargeTimeOffsetCh setter.
     * @param nOfLargeTimeOffsetCh is the reference to
     input value.
     */
    void setNOfLargeTimeOffsetCh(const int& nOfLargeTimeOffsetCh);
    /**
     * m_nOfLargeAmplOffsetCh setter.
     * @param nOfLargeAmplOffsetCh is the reference to
     input value.
     */
    void setNOfLargeAmplOffsetCh(const int& nOfLargeAmplOffsetCh);
    /**
     * m_nOfAbsMeanTimeChOutRange setter.
     * @param nOfAbsMeanTimeChOutRange is the reference
     to input value.
     */
    void setNOfAbsMeanTimeChOutRange(const int& nOfAbsMeanTimeChOutRange);
    /**
     * nOfAbsMeanAmplChOutRange setter.
     * @param nOfAbsMeanAmplChOutRange is the reference
     to input value.
     */
    void setNOfAbsMeanAmplChOutRange(const int& nOfAbsMeanAmplChOutRange);
    /**
     * Call this function, if
     * the number of events is too small.
     */
    void enableTooSmallNOfEvs();
    /**
     * Call this function, if
     * there are negative amplitudes.
     */
    void enableNegAmpl();
  private:
    /**
     * m_exp is an experiment
     * number.
     */
    int m_exp;
    /**
     * m_run is a run number.
     */
    int m_run;
    /**
     * m_runRef is a reference run
     * number.
     */
    int m_runRef;
    /**
     * m_nOfEvents is the number of
     * events.
     */
    int m_nOfEvents;
    /**
     * m_nOfBadTimeCountCh is the
     * number of channels, where
     * the number of events used to
     * calculate mean time is too small.
     */
    int m_nOfBadTimeCountCh;
    /**
     * m_nOfBadAmplCountCh is the
     * number of channels, where
     * the number of events used to
     * calculate mean amplitude is too
     * small.
     */
    int m_nOfBadAmplCountCh;
    /**
     * m_nOfLargeTimeOffsetCh is the
     * number of channels, where the
     * offset of mean time is too large.
     */
    int m_nOfLargeTimeOffsetCh;
    /**
     * m_nOfLargeAmplOffsetCh is the
     * number of channels, where the
     * offset of mean amplitude is too
     * large.
     */
    int m_nOfLargeAmplOffsetCh;
    /**
     * m_nOfAbsMeanTimeChOutRange is
     * the number of channels, where
     * mean time value is outside of
     * allowed limits.
     */
    int m_nOfAbsMeanTimeChOutRange;
    /**
     * m_nOfAbsMeanAmplChOutRange is
     * the number of channels, where
     * mean amplitude value is outside
     * of allowed limits.
     */
    int m_nOfAbsMeanAmplChOutRange;
    /**
     * m_tooSmallNOfEvs is true, when
     * the number of events is too small.
     */
    bool m_tooSmallNOfEvs;
    /**
     * m_isNegAmpl is true, when
     * there are negative amplitude
     * values.
     */
    bool m_isNegAmpl;
  };
};
