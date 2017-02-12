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
#include <top/dbobjects/TOPSampleTimes.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  /**
   * Sample time calibration constants for all channels.
   */
  class TOPCalTimebase: public TObject {
  public:

    /**
     * Default constructor
     */
    TOPCalTimebase()
    {}

    /**
     * Useful constructor
     * @param syncTimeBase sinchronization time base (width of 2 ASIC windows)
     */
    explicit TOPCalTimebase(double syncTimeBase): m_syncTimeBase(syncTimeBase)
    {}

    /**
     * Copy constructor (since class includes pointers in its cache)
     */
    TOPCalTimebase(const TOPCalTimebase& timeBase): TObject()
    {
      *this = timeBase;
      m_map.clear();
      m_sampleTime = 0;
    }

    /**
     * Destructor
     */
    ~TOPCalTimebase()
    {
      if (m_sampleTime) delete m_sampleTime;
    }

    /**
     * Appends calibration of a single ASIC channel - sets equidistant time axis
     * @param scrodID SCROD ID
     * @param channel channel number within SCROD (0 - 127)
     */
    void append(unsigned scrodID, unsigned channel)
    {
      m_sampleTimes.push_back(TOPSampleTimes(scrodID, channel, m_syncTimeBase));
    }

    /**
     * Appends calibration of a single ASIC channel
     * @param scrodID SCROD ID
     * @param channel channel number within SCROD (0 - 127)
     * @param sampleTimes vector of 256 elements defining time axis
     */
    void append(unsigned scrodID, unsigned channel,
                const std::vector<double>& sampleTimes)
    {
      m_sampleTimes.push_back(TOPSampleTimes(scrodID, channel, m_syncTimeBase));
      m_sampleTimes.back().setTimeAxis(sampleTimes, m_syncTimeBase);
    }

    /**
     * Returns sinchronization time base (width of 2 ASIC windows)
     * @return sinchronization time base
     */
    double getSyncTimeBase() const {return m_syncTimeBase;}

    /**
     * Returns all calibration constants
     * @return vector of calibration constants
     */
    const std::vector<TOPSampleTimes>& getSampleTimes() const {return m_sampleTimes;}

    /**
     * Returns calibration constants for given SCROD and channel.
     * If calibration doesn't exist, return equidistant sample times.
     * @return calibration constants (always valid pointer)
     */
    const TOPSampleTimes* getSampleTimes(unsigned scrodID, unsigned channel) const;

  private:

    /**
     * Creates a map in cache
     */
    void createMap() const;

    double m_syncTimeBase = 0; /**< sinchronization time base (width of 2 ASIC windows) */
    std::vector<TOPSampleTimes> m_sampleTimes; /**< calibration constants */

    /** cache for a map */
    mutable std::map<unsigned, const TOPSampleTimes*> m_map; //!

    /** cache for default (equidistant) sample times */
    mutable TOPSampleTimes* m_sampleTime = 0; //!

    ClassDef(TOPCalTimebase, 1); /**< ClassDef */

  };

} // end namespace Belle2

