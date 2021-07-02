/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
