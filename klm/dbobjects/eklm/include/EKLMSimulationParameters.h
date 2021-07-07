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
