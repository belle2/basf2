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
     * time axis size (time axis for 4 ASIC windows)
     */
    enum {c_WindowSize = TOPASICPedestals::c_WindowSize,
          c_TimeAxisSize = c_WindowSize * 4
         };

    /**
     * Default constructor
     */
    TOPSampleTimes()
    {
      for (unsigned i = 0; i <= c_TimeAxisSize; i++) m_timeAxis[i] = 0;
    }

    /**
     * Useful constructor
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
     * Set equidistant time axis.
     * @param syncTimeBase sinchronization time base (width of 2 ASIC windows)
     */
    void setTimeAxis(double syncTimeBase);

    /**
     * Set time axis from calibration data.
     * @param sampleTimes vector of 256 elements of sample times
     * @param syncTimeBase sinchronization time base (width of 2 ASIC windows)
     */
    void setTimeAxis(const std::vector<double>& sampleTimes, double syncTimeBase);

    /**
     * Return scrod ID
     * @return scrod ID
     */
    unsigned getScrodID() const {return m_scrodID;}

    /**
     * Return hardware channel number
     * @return channel number
     */
    unsigned getChannel() const {return m_channel;}

    /**
     * Return time axis range (time interval corresponding to 4 asic windows)
     * @return time axis range
     */
    double getTimeRange() const {return m_timeAxis[c_TimeAxisSize];}

    /**
     * Return time axis (sample times)
     * @return vector of sample times
     */
    std::vector<double> getTimeAxis() const
    {
      std::vector<double> timeAxis;
      for (unsigned i = 0; i < c_TimeAxisSize; i++) {
        timeAxis.push_back(m_timeAxis[i]);
      }
      return timeAxis;
    }

    /**
     * Return time in respect to sample 0 of window 0
     *
     * Note: sample is float - digits that follow the decimal point are used to
     * interpolate the time  btw. two samples
     * @param window ASIC window number
     * @param sample sample counted from the first one in the specified ASIC window
     * @return time in [ns]
     */
    double getTime(unsigned window, double sample) const;

    /**
     * Return time in respect to sample 0 of the specified ASIC window.
     *
     * Note: sample is float - digits that follow the decimal point are used to
     * interpolate the time  btw. two samples
     * @param window ASIC window number
     * @param sample sample counted from the first one in the specified ASIC window
     * @return time in [ns]
     */
    double getTimeDifference(unsigned window, double sample) const
    {
      return getTime(window, sample) - getTime(window, 0);
    }

    /**
     * Return time difference between sample2 and sample1
     *
     * Note: sample is float - digits that follow the decimal point are used to
     * interpolate the time  btw. two samples
     * @param window ASIC window number
     * @param sample1 sample counted from the first one in the specified ASIC window
     * @param sample2 sample counted from the first one in the specified ASIC window
     * @return time difference in [ns]
     */
    double getDeltaTime(unsigned window, double sample1, double sample2) const
    {
      return getTime(window, sample2) - getTime(window, sample1);
    }

    /**
     * Is time axis calibrated or equidistant
     * @return true, if calibrated
     */
    bool isCalibrated() const {return m_calibrated;}


  private:

    unsigned short m_scrodID = 0;          /**< scrod ID */
    unsigned short m_channel = 0;          /**< hardware channel number within SCROD */
    float m_timeAxis[c_TimeAxisSize + 1];  /**< time axis + right border point */
    bool m_calibrated = false;             /**< flag: calibrated or equidistant */

    ClassDef(TOPSampleTimes, 1); /**< ClassDef */

  };

} // end namespace Belle2

