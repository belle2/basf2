/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giacomo De Pietro                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
  class EventsOfDoomParameters : public TObject {

  public:

    /**
     * Default constructor
     */
    EventsOfDoomParameters()
    {
      m_nCDCHitsMax = 0;
      m_nSVDShaperDigitsMax = 0;
    }

    /**
     * Constructor
     * @param[in] nCDCHitsMax         Max number of CDC hits for an event.
     * @param[in] nSVDShaperDigitsMax Max number of SVD shaper digits for an event.
     */
    EventsOfDoomParameters(uint32_t nCDCHitsMax, uint32_t nSVDShaperDigitsMax)
    {
      m_nCDCHitsMax = nCDCHitsMax;
      m_nSVDShaperDigitsMax = nSVDShaperDigitsMax;
    }

    /**
     * Destructor
     */
    ~EventsOfDoomParameters()
    {
    }

    /**
     * Set the max number of CDC hits for an event.
     * @param[in] nCDCHitsMax Max number of CDC hits for an event.
     */
    void setNCDCHitsMax(uint32_t nCDCHitsMax)
    {
      m_nCDCHitsMax = nCDCHitsMax;
    }

    /**
     * Set the max number of SVD shaper digits for an event.
     * @param[in] nSVDShaperDigitsMax Max number of SVD shaper digits for an event.
     */
    void setNSVDShaperDigitsMax(uint32_t nSVDShaperDigitsMax)
    {
      m_nSVDShaperDigitsMax = nSVDShaperDigitsMax;
    }

    /**
     * Get the max number of CDC hits for an event.
     */
    uint32_t getNCDCHitsMax() const
    {
      return m_nCDCHitsMax;
    }

    /**
     * Get the max number of SVD shaper digits for an event.
     */
    uint32_t getNSVDShaperDigitsMax() const
    {
      return m_nSVDShaperDigitsMax;
    }

  private:

    /**
     * Max number of CDC hits for an event to be kept for reconstruction.
     */
    uint32_t m_nCDCHitsMax;

    /**
     * Max number of SVD shaper digits for an event to be kept for reconstruction.
     */
    uint32_t m_nSVDShaperDigitsMax;

    /**
     * Class version.
     */
    ClassDef(EventsOfDoomParameters, 1);

  };

}
