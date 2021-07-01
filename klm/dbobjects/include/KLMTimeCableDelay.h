/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jincheng Mei                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* ROOT headers. */
#include <TObject.h>

/* C++ headers. */
#include <cstdint>
#include <map>

namespace Belle2 {

  /**
   * Class to store BKLM delay time coused by cable in the database.
   */
  class KLMTimeCableDelay : public TObject {

  public:

    /**
     * Constructor.
     */
    KLMTimeCableDelay();

    /**
     * Destructor.
     */
    ~KLMTimeCableDelay();

    /**
     * Set the time calibration constant.
     * @param[in] channel   Channel number.
     * @param[in] timeDelay Time calibration constant.
     */
    void setTimeDelay(uint16_t channel, float timeDelay);

    /**
     * Get the time calibration constant.
     * @param[in] channel Channel number.
     */
    float getTimeDelay(uint16_t channel) const;

    /**
     * Clear the time calibration constants.
     */
    void clearTimeDelay();

  private:

    /** Map of the time calibration constants. */
    std::map<uint16_t, float> m_timeDelay;

    /** Class version. */
    ClassDef(KLMTimeCableDelay, 1);

  };

}
