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

/* ROOT headers. */
#include <TObject.h>

namespace Belle2 {

  /**
   * KLM time conversion.
   */
  class KLMTimeConversion : public TObject {

  public:

    /**
     * Constructor.
     */
    KLMTimeConversion();

    /**
     * Destructor.
     */
    ~KLMTimeConversion();

    /**
     * Get TDC period.
     */
    double getTDCPeriod() const
    {
      return m_TDCPeriod;
    }

    /**
     * Set TDC frequency.
     */
    void setTDCFrequency(double frequency)
    {
      m_TDCPeriod = 1.0 / frequency;
    }

    /**
     * Set time offset.
     * @param[in] offset Offset in ns.
     */
    void setTimeOffset(double offset)
    {
      m_TimeOffset = offset;
    }

    /**
     * Set CTIME shift.
     * @param[in] shift Shift in bits.
     */
    void setCTimeShift(int shift)
    {
      m_CTimeShift = shift;
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
    uint16_t getTDCByTime(double time) const
    {
      return (time - m_TimeOffset) / m_TDCPeriod;
    }

  private:

    /** TDC period in ns. */
    double m_TDCPeriod;

    /** Time offset. */
    double m_TimeOffset;

    /** CTIME shift in bits. */
    int m_CTimeShift;

    /** Class version, */
    ClassDef(KLMTimeConversion, 1);

  };

}
