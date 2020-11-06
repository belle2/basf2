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
     * Set time calibration constant value.
     * @param[in] KLM strip global element number.
     * @param[in] calibration constant value for the strip.
     */
    void setTimeShift(uint16_t, double);

    /**
     * Get time calibration data.
     * @param[in] KLM strip global element number.
     */
    double getTimeShift(uint16_t) const;

    /**
     * Clean time calibration constant.
     */
    void cleanTimeShift();

  private:

    /** Container of time calibration constant value. */
    std::map<uint16_t, double> m_timeShift;

    /** Class version. */
    ClassDef(KLMTimeCableDelay, 1);

  };
}
