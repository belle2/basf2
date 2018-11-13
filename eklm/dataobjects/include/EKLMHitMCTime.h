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

/* External headers. */
#include <TObject.h>

namespace Belle2 {

  /**
   * Hit MC time.
   */
  class EKLMHitMCTime {

  public:

    /**
     * Constructor.
     */
    EKLMHitMCTime();

    /**
     * Destructor.
     */
    virtual ~EKLMHitMCTime();

    /**
     * Set MC time.
     * @param[in] t TIme.
     */
    void setMCTime(float t);

    /**
     * Get MC time.
     * @return Time.
     */
    float getMCTime() const;

  protected:

    /** Time. */
    float m_MCTime;

  private:

    /** Class version. */
    ClassDef(Belle2::EKLMHitMCTime, 1);

  };

}
