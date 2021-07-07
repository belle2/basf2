/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    KLMTimeConversion()
    {
    }

    /**
     * Destructor.
     */
    ~KLMTimeConversion()
    {
    }

    /**
     * Get time offset.
     */
    double getTimeOffset() const
    {
      return m_TimeOffset;
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
     * Get CTIME shift.
     */
    int getCTimeShift() const
    {
      return m_CTimeShift;
    }

    /**
     * Set CTIME shift.
     * @param[in] shift Shift in bits.
     */
    void setCTimeShift(int shift)
    {
      m_CTimeShift = shift;
    }

  private:

    /** Time offset. */
    double m_TimeOffset = 0.0;

    /** CTIME shift in bits. */
    int m_CTimeShift = 0;

    /** Class version, */
    ClassDef(KLMTimeConversion, 2);

  };

}
