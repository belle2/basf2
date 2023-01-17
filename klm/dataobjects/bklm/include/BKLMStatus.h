/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//! bit position for MC-generated hit that represents a particle-decay point
#define BKLM_DECAYED_BIT 23

//! bit mask for MC-generated hit that represents a particle-decay point
#define BKLM_DECAYED_MASK (1 << BKLM_DECAYED_BIT)

namespace Belle2 {

  /**
   * BKLM hit status.
   */
  class BKLMStatus {

  public:

    /**
     * Constructor.
     */
    BKLMStatus()
    {
    }

    /**
     * Destructor.
     */
    ~BKLMStatus()
    {
    }

    /**
     * Get maximal strip number.
     */
    static int getMaximalStrip(int module)
    {
      return ((module & BKLM_MAXSTRIP_MASK) >> BKLM_MAXSTRIP_BIT) + 1;
    }

    /**
     * Set maximal strip number.
     */
    static void setMaximalStrip(int& module, int strip)
    {
      module = (module & (~BKLM_MAXSTRIP_MASK)) | ((strip - 1) << BKLM_MAXSTRIP_BIT);
    }

  protected:

    /** Bit position for maxStrip-1 [0..47]. */
    static constexpr int BKLM_MAXSTRIP_BIT = 15;

    /** Bit mask for maxStrip-1 [0..47]. */
    static constexpr int BKLM_MAXSTRIP_MASK = (63 << BKLM_MAXSTRIP_BIT);

  };

}
