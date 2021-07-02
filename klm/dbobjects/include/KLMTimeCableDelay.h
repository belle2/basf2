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

/* KLM headers. */
#include <klm/dataobjects/KLMElementNumberDefinitions.h>

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
    void setTimeDelay(KLMChannelNumber channel, float timeDelay);

    /**
     * Get the time calibration constant.
     * @param[in] channel Channel number.
     */
    float getTimeDelay(KLMChannelNumber channel) const;

    /**
     * Clear the time calibration constants.
     */
    void clearTimeDelay();

  private:

    /** Map of the time calibration constants. */
    std::map<KLMChannelNumber, float> m_timeDelay;

    /** Class version. */
    ClassDef(KLMTimeCableDelay, 1);

  };

}
