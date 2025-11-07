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
  class HLTPrefilterParameters : public TObject {

  public:
    /// Enumeration for HLT prefilter state
    enum HLTPrefilterState { TimingCut = 0, CDCECLCut = 1 };

    /**
     * Default constructor
     */
    HLTPrefilterParameters()
    {
      m_LERtimeSinceLastInjectionMin = 0;
      m_LERtimeSinceLastInjectionMax = 0;
      m_HERtimeSinceLastInjectionMin = 0;
      m_HERtimeSinceLastInjectionMax = 0;
      m_LERtimeInBeamCycleMin = 0;
      m_LERtimeInBeamCycleMax = 0;
      m_HERtimeInBeamCycleMin = 0;
      m_HERtimeInBeamCycleMax = 0;
      m_cdcHitsMax = 1e9;
      m_eclDigitsMax = 1e9;
      m_HLTPrefilterState = static_cast<HLTPrefilterState>(0);
      m_HLTPrefilterPrescale = 1000;
    }

    /**
     * Constructor
     * @param[in] LERtimeSinceLastInjectionMin   Minimum threshold of timeSinceLastInjection for LER injection.
     * @param[in] LERtimeSinceLastInjectionMax   Maximum threshold of timeSinceLastInjection for LER injection.
     * @param[in] HERtimeSinceLastInjectionMin   Minimum threshold of timeSinceLastInjection for HER injection.
     * @param[in] HERtimeSinceLastInjectionMax   Maximum threshold of timeSinceLastInjection for HER injection.
     * @param[in] LERtimeInBeamCycleMin          Minimum threshold of timeInBeamCycle for LER injection.
     * @param[in] LERtimeInBeamCycleMax          Maximum threshold of timeInBeamCycle for LER injection.
     * @param[in] HERtimeInBeamCycleMin          Minimum threshold of timeInBeamCycle for HER injection.
     * @param[in] HERtimeInBeamCycleMax          Maximum threshold of timeInBeamCycle for HER injection.
     * @param[in] State                          State of HLTprefilter
     * @param[in] HLTPrefilterPrescale           Prescale for accepting events rejected by the HLTprefilter result
     */
    HLTPrefilterParameters(double LERtimeSinceLastInjectionMin, double LERtimeSinceLastInjectionMax, double LERtimeInBeamCycleMin,
                           double LERtimeInBeamCycleMax, double HERtimeSinceLastInjectionMin, double HERtimeSinceLastInjectionMax,
                           double HERtimeInBeamCycleMin, double HERtimeInBeamCycleMax, unsigned short State,
                           unsigned int HLTPrefilterPrescale)
    {
      m_LERtimeSinceLastInjectionMin = LERtimeSinceLastInjectionMin;
      m_LERtimeSinceLastInjectionMax = LERtimeSinceLastInjectionMax;
      m_HERtimeSinceLastInjectionMin = HERtimeSinceLastInjectionMin;
      m_HERtimeSinceLastInjectionMax = HERtimeSinceLastInjectionMax;
      m_LERtimeInBeamCycleMin = LERtimeInBeamCycleMin;
      m_LERtimeInBeamCycleMax = LERtimeInBeamCycleMax;
      m_HERtimeInBeamCycleMin = HERtimeInBeamCycleMin;
      m_HERtimeInBeamCycleMax = HERtimeInBeamCycleMax;
      m_HLTPrefilterState = static_cast<HLTPrefilterState>(State);
      m_HLTPrefilterPrescale = HLTPrefilterPrescale;
    }

    /**
     * Constructor
     * @param[in] cdcHitsMax                     Maximum threshold of CDCHits
     * @param[in] eclDigitsMax                   Maximum threshold of ECLDigits
     * @param[in] State                          State of HLTprefilter
     * @param[in] HLTPrefilterPrescale           Prescale for accepting events rejected by the HLTprefilter result
     */
    HLTPrefilterParameters(uint32_t cdcHitsMax, uint32_t eclDigitsMax, unsigned short State, unsigned int HLTPrefilterPrescale)
    {
      m_cdcHitsMax = cdcHitsMax;
      m_eclDigitsMax = eclDigitsMax;
      m_HLTPrefilterState = static_cast<HLTPrefilterState>(State);
      m_HLTPrefilterPrescale = HLTPrefilterPrescale;
    }

    /**
     * Destructor
     */
    ~HLTPrefilterParameters()
    {
    }

    /**
    * Set the minimum threshold of timeSinceLastInjection for LER injection for an event.
    * @param[in] LERtimeSinceLastInjectionMin for an event.
    */
    void setLERtimeSinceLastInjectionMin(double LERtimeSinceLastInjectionMin)
    {
      m_LERtimeSinceLastInjectionMin = LERtimeSinceLastInjectionMin;
    }

    /**
    * Set the maximum threshold of timeSinceLastInjection for LER injection for an event.
    * @param[in] LERtimeSinceLastInjectionMax for an event.
    */
    void setLERtimeSinceLastInjectionMax(double LERtimeSinceLastInjectionMax)
    {
      m_LERtimeSinceLastInjectionMax = LERtimeSinceLastInjectionMax;
    }

    /**
    * Set the minimum threshold of timeSinceLastInjection for HER injection for an event.
    * @param[in] HERtimeSinceLastInjectionMin for an event.
    */
    void setHERtimeSinceLastInjectionMin(double HERtimeSinceLastInjectionMin)
    {
      m_HERtimeSinceLastInjectionMin = HERtimeSinceLastInjectionMin;
    }

    /**
    * Set the maximum threshold of timeSinceLastInjection for HER injection for an event.
    * @param[in] HERtimeSinceLastInjectionMax for an event.
    */
    void setHERtimeSinceLastInjectionMax(double HERtimeSinceLastInjectionMax)
    {
      m_HERtimeSinceLastInjectionMax = HERtimeSinceLastInjectionMax;
    }


    /**
    * Set the minimum threshold of timeInBeamCycle for LER injection for an event.
    * @param[in] LERtimeInBeamCycleMin for an event.
    */
    void setLERtimeInBeamCycleMin(double LERtimeInBeamCycleMin)
    {
      m_LERtimeInBeamCycleMin = LERtimeInBeamCycleMin;
    }

    /**
    * Set the maximum threshold of timeInBeamCycle for LER injection for an event.
    * @param[in] LERtimeInBeamCycleMax for an event.
    */
    void setLERtimeInBeamCycleMax(double LERtimeInBeamCycleMax)
    {
      m_LERtimeInBeamCycleMax = LERtimeInBeamCycleMax;
    }

    /**
    * Set the minimum threshold of timeInBeamCycle for HER injection for an event.
    * @param[in] HERtimeInBeamCycleMin for an event.
    */
    void setHERtimeInBeamCycleMin(double HERtimeInBeamCycleMin)
    {
      m_HERtimeInBeamCycleMin = HERtimeInBeamCycleMin;
    }

    /**
    * Set the maximum threshold of timeInBeamCycle for HER injection for an event.
    * @param[in] HERtimeInBeamCycleMax for an event.
    */
    void setHERtimeInBeamCycleMax(double HERtimeInBeamCycleMax)
    {
      m_HERtimeInBeamCycleMax = HERtimeInBeamCycleMax;
    }

    /**
    * Set the maximum threshold of CDCHits for an event.
    * @param[in] cdcHitsMax for an event.
    */
    void setCDCHitsMax(uint32_t cdcHitsMax)
    {
      m_cdcHitsMax = cdcHitsMax;
    }

    /**
    * Set the maximum threshold of CDCHits for an event.
    * @param[in] eclDigitsMax for an event.
    */
    void setECLDigitsMax(uint32_t eclDigitsMax)
    {
      m_eclDigitsMax = eclDigitsMax;
    }

    /**
    * Set the HLTPrefilter state
    * @param[in] State
    */
    void setHLTPrefilterState(unsigned short State)
    {
      m_HLTPrefilterState = static_cast<HLTPrefilterState>(State);
    }

    /**
    * Set the prescale for HLTPrefilter result
    * @param[in] HLTPrefilterPrescale
    */
    void setHLTPrefilterPrescale(unsigned int HLTPrefilterPrescale)
    {
      m_HLTPrefilterPrescale = HLTPrefilterPrescale;
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

    /**
    * Get the maximum threshold of CDCHits for an event.
    */
    uint32_t getCDCHitsMax() const
    {
      return m_cdcHitsMax;
    }

    /**
    * Get the maximum threshold of CDCHits for an event.
    */
    uint32_t getECLDigitsMax() const
    {
      return m_eclDigitsMax;
    }

    /**
    * Get the HLTPrefilter state
    */
    unsigned short getHLTPrefilterState() const
    {
      return static_cast<unsigned short>(m_HLTPrefilterState);
    }

    /**
    * Get the prescale for HLTPrefilter result
    */
    unsigned int getHLTPrefilterPrescale() const
    {
      return m_HLTPrefilterPrescale;
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
     * Maximum threshold of CDCHits
     */
    uint32_t m_cdcHitsMax;
    /**
     * Maximum threshold of ECLDigits
     */
    uint32_t m_eclDigitsMax;

    /**
     * HLTPrefilter State
     */
    HLTPrefilterState m_HLTPrefilterState;

    /**
     * HLTPrefilter prescale
     */
    unsigned int m_HLTPrefilterPrescale;

    /**
     * Class version.
     */
    ClassDef(HLTPrefilterParameters, 1);

  };

}
