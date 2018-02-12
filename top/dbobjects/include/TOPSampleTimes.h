/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <vector>
#include <top/dbobjects/TOPASICPedestals.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  /**
   * Calibration constants of a singe ASIC channel: time axis (sample times)
   */
  class TOPSampleTimes: public TObject {
  public:

    /**
     * Calibration status values
     */
    enum EStatus {
      c_Default = 0,    /**< uncalibrated default value */
      c_Calibrated = 1, /**< good calibrated value */
      c_Unusable = 2    /**< bad calibrated value */
    };

    /**
     * time axis size (time axis for 4 ASIC windows)
     */
    enum {c_WindowSize = TOPASICPedestals::c_WindowSize,
          c_TimeAxisSize = c_WindowSize * 4
         };

    /**
     * Default constructor
     */
    TOPSampleTimes()
    {}

    /**
     * Useful constructor, sets equidistant time base (uncalibrated)
     * @param scrodID scrod ID
     * @param channel hardware channel number
     * @param syncTimeBase sinchronization time base (width of 2 ASIC windows)
     */
    TOPSampleTimes(unsigned scrodID, unsigned channel, double syncTimeBase):
      m_scrodID(scrodID), m_channel(channel)
    {
      setTimeAxis(syncTimeBase);
    }

    /**
     * Destructor
     */
    ~TOPSampleTimes()
    {
    }

    /**
     * Sets equidistant time axis (uncalibrated).
     * @param syncTimeBase sinchronization time base (width of 2 ASIC windows)
     */
    void setTimeAxis(double syncTimeBase);

    /**
     * Sets time axis from calibration data and switches status to calibrated
     * @param sampleTimes vector of 256 elements of sample times
     * @param syncTimeBase sinchronization time base (width of 2 ASIC windows)
     */
    void setTimeAxis(const std::vector<double>& sampleTimes, double syncTimeBase);

    /**
     * Sets uncertainty on time base calibration incl. systematics
     * @param error estimated uncertainty of time axis points
     */
    void setTimeError(double error) {m_timeError = error;}

    /**
     * Switches calibration status to unusable to flag badly calibrated constant
     */
    void setUnusable() {m_calibrated = c_Unusable;}

    /**
     * Returns scrod ID
     * @return scrod ID
     */
    unsigned getScrodID() const {return m_scrodID;}

    /**
     * Returns hardware channel number
     * @return channel number
     */
    unsigned getChannel() const {return m_channel;}

    /**
     * Returns time axis range (time interval corresponding to 4 asic windows)
     * @return time axis range
     */
    double getTimeRange() const {return m_timeAxis[c_TimeAxisSize];}

    /**
     * Returns time axis (sample times)
     * @return vector of sample times
     */
    std::vector<double> getTimeAxis() const;

    /**
     * Returns uncertainty on time base calibration incl. systematics
     * @return uncertainty of time axis points
     */
    double getTimeError() const {return m_timeError;}

    /**
     * Returns time in respect to sample 0 of window 0
     *
     * Note: sample is float - digits that follow the decimal point are used to
     * interpolate the time  btw. two samples
     * @param window ASIC window number
     * @param sample sample counted from the first one in the specified ASIC window
     * @return time in [ns]
     */
    double getFullTime(int window, double sample) const;

    /**
     * Returns time in respect to sample 0 of the specified ASIC window.
     *
     * Note: sample is float - digits that follow the decimal point are used to
     * interpolate the time  btw. two samples
     * @param window ASIC window number
     * @param sample sample counted from the first one in the specified ASIC window
     * @return time in [ns]
     */
    double getTime(int window, double sample) const
    {
      return getFullTime(window, sample) - getFullTime(window, 0);
    }

    /**
     * Returns time difference between sample2 and sample1
     *
     * Note: sample is float - digits that follow the decimal point are used to
     * interpolate the time  btw. two samples
     * @param window ASIC window number
     * @param sample2 sample counted from the first sample of the specified ASIC window
     * @param sample1 sample counted from the first sample of the specified ASIC window
     * @return time difference in [ns]
     */
    double getDeltaTime(int window, double sample2, double sample1) const
    {
      return getFullTime(window, sample2) - getFullTime(window, sample1);
    }

    /**
     * Returns time bin of a given sample number and window
     * (e.g. time interval to the next sample)
     * @param window ASIC window number
     * @param sampleNumber sample number counted from begin of the specified ASIC window
     * @return time bin in [ns]
     */
    double getTimeBin(int window, int sampleNumber) const;

    /**
     * Returns sample in respect to sample 0 of the specified ASIC window
     * (inverse of getTime).
     *
     * Note: sample is float - digits that follow the decimal point are used to
     * interpolate the time  btw. two samples
     * @param window ASIC window number
     * @param time time in respect to time of sample 0 of the specified ASIC window
     * @return sample
     */
    double getSample(int window, double time) const;

    /**
     * Returns calibration status
     * @return true, if good calibrated
     */
    bool isCalibrated() const {return m_calibrated == c_Calibrated;}

    /**
     * Returns calibration status
     * @return true, if default (= equidistant, not calibrated)
     */
    bool isDefault() const {return m_calibrated == c_Default;}

    /**
     * Returns calibration status
     * @return true, if bad calibrated
     */
    bool isUnusable() const {return m_calibrated == c_Unusable;}

  private:

    unsigned short m_scrodID = 0;          /**< scrod ID */
    unsigned short m_channel = 0;          /**< hardware channel number within SCROD */
    float m_timeAxis[c_TimeAxisSize + 1] = {0};  /**< time axis + right border point */
    float m_timeError = 0;                 /**< uncertainty of time axis points incl. systematics */
    EStatus m_calibrated = c_Default;      /**< calibration status */

    ClassDef(TOPSampleTimes, 2); /**< ClassDef */

  };

} // end namespace Belle2

