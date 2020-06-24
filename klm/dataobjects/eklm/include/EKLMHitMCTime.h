/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
