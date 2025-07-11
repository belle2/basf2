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
   * DBObject containing parameters used in HLTprefilter module.
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

    /**
    * Set the minimum threshold of timeSinceLastInjection for LER injection for an event.
    * @param[in] nLERtimeSinceLastInjectionMin for an event.
    */
    void setLERtimeSinceLastInjectionMin(double nLERtimeSinceLastInjectionMin)
    {
      m_LERtimeSinceLastInjectionMin = nLERtimeSinceLastInjectionMin;
    }

    /**
    * Set the maximum threshold of timeSinceLastInjection for LER injection for an event.
    * @param[in] nLERtimeSinceLastInjectionMax for an event.
    */
    void setLERtimeSinceLastInjectionMax(double nLERtimeSinceLastInjectionMax)
    {
      m_LERtimeSinceLastInjectionMax = nLERtimeSinceLastInjectionMax;
    }

    /**
    * Set the minimum threshold of timeSinceLastInjection for HER injection for an event.
    * @param[in] nHERtimeSinceLastInjectionMin for an event.
    */
    void setHERtimeSinceLastInjectionMin(double nHERtimeSinceLastInjectionMin)
    {
      m_HERtimeSinceLastInjectionMin = nHERtimeSinceLastInjectionMin;
    }

    /**
    * Set the maximum threshold of timeSinceLastInjection for HER injection for an event.
    * @param[in] nHERtimeSinceLastInjectionMax for an event.
    */
    void setHERtimeSinceLastInjectionMax(double nHERtimeSinceLastInjectionMax)
    {
      m_HERtimeSinceLastInjectionMax = nHERtimeSinceLastInjectionMax;
    }


    /**
    * Set the minimum threshold of timeInBeamCycle for LER injection for an event.
    * @param[in] nLERtimeInBeamCycleMin for an event.
    */
    void setLERtimeInBeamCycleMin(double nLERtimeInBeamCycleMin)
    {
      m_LERtimeInBeamCycleMin = nLERtimeInBeamCycleMin;
    }

    /**
    * Set the maximum threshold of timeInBeamCycle for LER injection for an event.
    * @param[in] ntimeInBeamCycleMax for an event.
    */
    void setLERtimeInBeamCycleMax(double nLERtimeInBeamCycleMax)
    {
      m_LERtimeInBeamCycleMax = nLERtimeInBeamCycleMax;
    }

    /**
    * Set the minimum threshold of timeInBeamCycle for HER injection for an event.
    * @param[in] nHERtimeInBeamCycleMin for an event.
    */
    void setHERtimeInBeamCycleMin(double nHERtimeInBeamCycleMin)
    {
      m_HERtimeInBeamCycleMin = nHERtimeInBeamCycleMin;
    }

    /**
    * Set the maximum threshold of timeInBeamCycle for HER injection for an event.
    * @param[in] ntimeInBeamCycleMax for an event.
    */
    void setHERtimeInBeamCycleMax(double nHERtimeInBeamCycleMax)
    {
      m_HERtimeInBeamCycleMax = nHERtimeInBeamCycleMax;
    }

    /**
    * Get the minimum threshold of timeSinceLastInjection for LER injection for an event.
    */
    double getLERtimeSinceLastInjectionMin() const
    {
      return m_LERtimeSinceLastInjectionMin;
    }

    /**
    * Get the maximum threshold of timeSinceLastInjection for LER injection for an event.
    */
    double getLERtimeSinceLastInjectionMax() const
    {
      return m_LERtimeSinceLastInjectionMax;
    }

    /**
    * Get the minimum threshold of timeSinceLastInjection for HER injection for an event.
    */
    double getHERtimeSinceLastInjectionMin() const
    {
      return m_HERtimeSinceLastInjectionMin;
    }

    /**
    * Get the maximum threshold of timeSinceLastInjection for HER injection for an event.
    */
    double getHERtimeSinceLastInjectionMax() const
    {
      return m_HERtimeSinceLastInjectionMax;
    }

    /**
    * Get the minimum threshold of timeInBeamCycle for LER injection for an event.
    */
    double getLERtimeInBeamCycleMin() const
    {
      return m_LERtimeInBeamCycleMin;
    }

    /**
    * Get the maximum threshold of timeInBeamCycle for LER injection for an event.
    */
    double getLERtimeInBeamCycleMax() const
    {
      return m_LERtimeInBeamCycleMax;
    }

    /**
    * Get the minimum threshold of timeInBeamCycle for HER injection for an event.
    */
    double getHERtimeInBeamCycleMin() const
    {
      return m_HERtimeInBeamCycleMin;
    }

    /**
    * Get the maximum threshold of timeInBeamCycle for HER injection for an event.
    */
    double getHERtimeInBeamCycleMax() const
    {
      return m_HERtimeInBeamCycleMax;
    }

  private:

    /**
     * Minimum threshold of timeSinceLastInjection for LER injection
     */
    double m_LERtimeSinceLastInjectionMin;
    /**
     * Maximum threshold of timeSinceLastInjection for LER injection
     */
    double m_LERtimeSinceLastInjectionMax;
    /**
     * Minimum threshold of timeSinceLastInjection for HER injection
     */
    double m_HERtimeSinceLastInjectionMin;
    /**
     * Maximum threshold of timeSinceLastInjection for HER injection
     */
    double m_HERtimeSinceLastInjectionMax;
    /**
     * Minimum threshold of timeInBeamCycle for LER injection
     */
    double m_LERtimeInBeamCycleMin;
    /**
     * Maximum threshold of timeInBeamCycle for LER injection
     */
    double m_LERtimeInBeamCycleMax;
    /**
     * Minimum threshold of timeInBeamCycle for HER injection
     */
    double m_HERtimeInBeamCycleMin;
    /**
     * Maximum threshold of timeInBeamCycle for HER injection
     */
    double m_HERtimeInBeamCycleMax;


    /**
     * Class version.
     */
    ClassDef(HLTprefilterParameters, 1);

  };

}
