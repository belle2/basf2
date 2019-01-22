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

/* External headers. */
#include <TObject.h>

namespace Belle2 {

  /**
   * EKLM data concentrator lane identifier.
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
     * Set TDC frequency.
     */
    void setTDCFrequency(double frequency);

    /**
     * Set time offset.
     */
    void setTimeOffset(double offset);

    /**
     * Get time by TDC.
     */
    double getTime(int ctime, int tdc, int triggerCTime) const;

    /**
     * Get TDC by time.
     */
    uint16_t getTDCByTime(double time) const;

  private:

    /** TDC period in ns. */
    double m_TDCPeriod;

    /** Time offset. */
    double m_TimeOffset;

    /** Class version, */
    ClassDef(KLMTimeConversion, 1);

  };

}
