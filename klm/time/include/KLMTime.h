/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dbobjects/KLMTimeConversion.h>

/* Belle 2 headers. */
#include <framework/database/DBObjPtr.h>
#include <framework/dbobjects/HardwareClockSettings.h>

namespace Belle2 {

  /**
   * KLM time conversion.
   */
  class KLMTime {

  public:

    /**
     * Constructor.
     */
    KLMTime()
    {
    }

    /**
     * Destructor.
     */
    ~KLMTime()
    {
    }

    /**
     * Update constants from database objects. This function needs to be called
     * when database objects change, for example, at the beginning of the run.
     */
    void updateConstants();

    /**
     * Get TDC period.
     */
    double getTDCPeriod() const
    {
      return m_TDCPeriod;
    }

    /**
     * Get CTIME period.
     */
    double getCTimePeriod() const
    {
      return m_CTimePeriod;
    }

    /**
     * Get time for scintillator.
     * @param[in] ctime        CTIME.
     * @param[in] triggerCTime Trigger CTIME.
     */
    double getScintillatorTime(int ctime, int triggerCTime) const;

    /**
     * Get coarse and fine times for RPC.
     * @param[in] ctime        REVO9 time.
     * @param[in] tdc          TDC.
     * @param[in] triggerTime  REVO9 Trigger time.
     */
    std::pair<int, double> getRPCTimes(int ctime, int tdc, int triggerTime) const;

    /**
     * Get time for simulation.
     * @param[in] tdc          TDC.
     * @param[in] scintillator Scintillator (true) or RPC (false).
     */
    double getTimeSimulation(int tdc, bool scintillator) const;

    /**
     * Get TDC by time.
     */
    uint16_t getTDCByTime(double time) const;

  private:

    /** KLM time conversion. */
    DBObjPtr<KLMTimeConversion> m_KLMTimeConversion;

    /** Hardware clock settings. */
    DBObjPtr<HardwareClockSettings> m_HardwareClockSettings;

    /**
     * TDC period (stored here because getting it from m_HardwareClockSettings
     * requires to search in a map).
     */
    double m_TDCPeriod;

    /** CTIME period. */
    double m_CTimePeriod;

  };

}
