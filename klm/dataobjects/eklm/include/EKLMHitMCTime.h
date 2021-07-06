/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ROOT headers. */
#include <Rtypes.h>

namespace Belle2 {

  /**
   * Hit MC time.
   */
  class EKLMHitMCTime {

  public:

    /**
     * Constructor.
     */
    EKLMHitMCTime()
    {
    }

    /**
     * Destructor.
     */
    virtual ~EKLMHitMCTime()
    {
    }

    /**
     * Set MC time.
     * @param[in] t TIme.
     */
    void setMCTime(float t)
    {
      m_MCTime = t;
    }

    /**
     * Get MC time.
     * @return Time.
     */
    float getMCTime() const
    {
      return m_MCTime;
    }

  protected:

    /** Time. */
    float m_MCTime = -1;

  private:

    /** Class version. */
    ClassDef(Belle2::EKLMHitMCTime, 1);

  };

}
