/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dbobjects/KLMTimeConversion.h>

/* Basf2 headers. */
#include <framework/database/DBObjPtr.h>
#include <framework/dbobjects/HardwareClockSettings.h>

namespace Belle2 {

  /**
   * KLM time conversion.
   */
  class KLMTime {

  public:

    /**
     * Instantiation.
     */
    static KLMTime& Instance();

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
     * @param[in] tdc          TDC.
     * @param[in] triggerCTime Trigger CTIME.
     */
    double getScintillatorTime(int ctime, int tdc, int triggerCTime) const;

    /**
     * Get coarse and fine times for RPC.
     * @param[in] ctime        REVO9 time.
     * @param[in] tdc          TDC.
     * @param[in] triggerTime  REVO9 Trigger time.
     */
    std::pair<int, double> getRPCTimes(int ctime, int tdc, int triggerTime) const;

    /**
     * Get FTime for RPC hits.
     * FTime is the timestamp applied in the Data Concentrator when an RPC hit
     * packet arrives. It uses the TTD clock period (~7.861 ns) and the counter
     * resets using the frame9 signal (cycles from 0 to 11520).
     * @param[in] ctime           REVO9 time (16-bit).
     * @param[in] triggerTime     REVO9 Trigger time (to correct for overflow).
     */
    std::pair<int, double> getFTime(int ctime, int triggerTime) const;
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

    /** KLM time conversion. */
    DBObjPtr<KLMTimeConversion> m_KLMTimeConversion;

    /** Hardware clock settings. */
    DBObjPtr<HardwareClockSettings> m_HardwareClockSettings;

    /**
     * TDC period (stored here because getting it from m_HardwareClockSettings
     * requires to search in a map).
     */
    double m_TDCPeriod = 0.;

    /** CTIME period. */
    double m_CTimePeriod = 0.;

  };

}
