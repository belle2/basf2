/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
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
   * Class to store EKLM simulation in the database.
   */
  class EKLMSimulationParameters : public TObject {

  public:

    /**
     * Constructor.
     */
    EKLMSimulationParameters()
    {
    }

    /**
     * Destructor.
     */
    ~EKLMSimulationParameters()
    {
    }

    /**
     * Get hit time threshold.
     */
    float getHitTimeThreshold() const
    {
      return m_HitTimeThreshold;
    }

    /**
     * Set hit time threshold.
     */
    void setHitTimeThreshold(float threshold)
    {
      m_HitTimeThreshold = threshold;
    }

  private:

    /** Maximal hit time. */
    float m_HitTimeThreshold = 0;

    /** Class version. */
    ClassDef(Belle2::EKLMSimulationParameters, 1);

  };

}
