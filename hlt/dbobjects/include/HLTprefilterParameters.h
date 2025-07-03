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

/* C++ headers. */
#include <cstdint>

namespace Belle2 {

  /**
   * DBObject containing parameters used in EventsOfDoomBuster module.
   */
  class HLTprefilterParameters : public TObject {

  public:

    /**
     * Default constructor
     */
    HLTprefilterParameters()
    {
      m_LERtimeSinceLastInjectionMin = 0;
      m_LERtimeSinceLastInjectionMax = 0;
      m_HERtimeSinceLastInjectionMin = 0;
      m_HERtimeSinceLastInjectionMax = 0;
      m_LERtimeInBeamCycleMin = 0;
      m_LERtimeInBeamCycleMax = 0;
      m_HERtimeInBeamCycleMin = 0;
      m_HERtimeInBeamCycleMax = 0;
    }

    /**
     * Constructor
     * @param[in] m_LERtimeSinceLastInjectionMin   Minimum threshold of timeSinceLastInjection for LER injection.
     * @param[in] m_LERtimeSinceLastInjectionMax   Maximum threshold of timeSinceLastInjection for LER injection.
     * @param[in] m_HERtimeSinceLastInjectionMin   Minimum threshold of timeSinceLastInjection for HER injection.
     * @param[in] m_HERtimeSinceLastInjectionMax   Maximum threshold of timeSinceLastInjection for HER injection.
     * @param[in] m_LERtimeInBeamCycleMin          Minimum threshold of timeInBeamCycle for LER injection.
     * @param[in] m_LERtimeInBeamCycleMax          Maximum threshold of timeInBeamCycle for LER injection.
     * @param[in] m_HERtimeInBeamCycleMin          Minimum threshold of timeInBeamCycle for HER injection.
     * @param[in] m_HERtimeInBeamCycleMax          Maximum threshold of timeInBeamCycle for HER injection.
     */
    HLTprefilterParameters(double LERtimeSinceLastInjectionMin, double LERtimeSinceLastInjectionMax, double LERtimeInBeamCycleMin,
                           double LERtimeInBeamCycleMax, double HERtimeSinceLastInjectionMin, double HERtimeSinceLastInjectionMax,
                           double HERtimeInBeamCycleMin, double HERtimeInBeamCycleMax)
    {
      m_LERtimeSinceLastInjectionMin = LERtimeSinceLastInjectionMin;
      m_LERtimeSinceLastInjectionMax = LERtimeSinceLastInjectionMax;
      m_HERtimeSinceLastInjectionMin = HERtimeSinceLastInjectionMin;
      m_HERtimeSinceLastInjectionMax = HERtimeSinceLastInjectionMax;
      m_LERtimeInBeamCycleMin = LERtimeInBeamCycleMin;
      m_LERtimeInBeamCycleMax = LERtimeInBeamCycleMax;
      m_HERtimeInBeamCycleMin = HERtimeInBeamCycleMin;
      m_HERtimeInBeamCycleMax = HERtimeInBeamCycleMax;

    }

    /**
     * Destructor
     */
    ~HLTprefilterParameters()
    {
    }

  private:

    /**
     * Minimum threshold of timeSinceLastInjection for LER injection
     */
    double m_LERtimeSinceLastInjectionMin = 5000;
    /**
     * Maximum threshold of timeSinceLastInjection for LER injection
     */
    double m_LERtimeSinceLastInjectionMax = 20000;
    /**
     * Minimum threshold of timeSinceLastInjection for HER injection
     */
    double m_HERtimeSinceLastInjectionMin = 600;
    /**
     * Maximum threshold of timeSinceLastInjection for HER injection
     */
    double m_HERtimeSinceLastInjectionMax = 20000;
    /**
     * Minimum threshold of timeInBeamCycle for LER injection
     */
    double m_LERtimeInBeamCycleMin = 1.25;
    /**
     * Maximum threshold of timeInBeamCycle for LER injection
     */
    double m_LERtimeInBeamCycleMax = 1.55;
    /**
     * Minimum threshold of timeInBeamCycle for HER injection
     */
    double m_HERtimeInBeamCycleMin = 2.2;
    /**
     * Maximum threshold of timeInBeamCycle for HER injection
     */
    double m_HERtimeInBeamCycleMax = 2.33;


    /**
     * Class version.
     */
    ClassDef(HLTprefilterParameters, 1);

  };

}
