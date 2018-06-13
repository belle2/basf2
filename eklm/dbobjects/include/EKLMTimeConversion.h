/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMTIMECONVERSION_H
#define EKLMTIMECONVERSION_H

/* External headers. */
#include <TObject.h>

namespace Belle2 {

  /**
   * EKLM data concentrator lane identifier.
   */
  class EKLMTimeConversion : public TObject {

  public:

    /**
     * Constructor.
     */
    EKLMTimeConversion();

    /**
     * Destructor.
     */
    ~EKLMTimeConversion();

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
    double getTimeByTDC(uint16_t tdc) const;

    /**
     * Get TDC by time.
     */
    uint16_t getTDCByTime(double time) const;

  private:

    /** TDC period in ns. */
    double m_TDCPeriod;

    /** Time offset. */
    double m_TimeOffset;

    /** Needed to make objects storable, */
    ClassDef(EKLMTimeConversion, 1);

  };

}

#endif

