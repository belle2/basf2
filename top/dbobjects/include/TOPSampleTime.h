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
   * Calibration constants of a singe ASIC channel: time axis
   */
  class TOPSampleTime: public TObject {
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
    TOPSampleTime()
    {
      for (unsigned i = 0; i <= c_TimeAxisSize; i++) m_timeAxis[i] = 0;
    }

    /**
     * Constructor with module ID, hardware channel number
     * @param moduleID module ID
     * @param channel hardware channel number
     * @param syncTimeBase sinchronization time base (width of 2 ASIC windows)
     */
    TOPSampleTime(int moduleID, unsigned channel, double syncTimeBase):
      m_moduleID(moduleID), m_channel(channel)
    {
      setTimeAxis(syncTimeBase);
    }

    /**
     * Destructor
     */
    ~TOPSampleTime()
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
     * Return module ID
     * @return module ID
     */
    int getModuleID() const {return m_moduleID;}

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
     */
    float getTime(unsigned window, float sample) const;

    /**
     * Return time in respect to sample 0 of the specified ASIC window.
     *
     * Note: sample is float - digits that follow the decimal point are used to
     * interpolate the time  btw. two samples
     * @param window ASIC window number
     * @param sample sample counted from the first one in the specified ASIC window
     */
    float getTimeDifference(unsigned window, float sample) const
    {
      return getTime(window, sample) - getTime(window, 0);
    }


  private:

    int m_moduleID = 0;               /**< module ID */
    unsigned m_channel = 0;           /**< hardware channel number */
    float m_timeAxis[c_TimeAxisSize + 1];  /**< time axis + right border point */

    ClassDef(TOPSampleTime, 1); /**< ClassDef */

  };

} // end namespace Belle2

