/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

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

#ifdef __clang_analyzer__
      // unused junk assignment purely to calm the nerves of the clang analyzer
      // (references the two arguments to avoid nattering about unused variables)
      module = module + (strip - 1);
#else
      // the clang analyzer falsely thinks that this bitwise operation produces garbage
      module = (module & (~BKLM_MAXSTRIP_MASK)) | ((strip - 1) << BKLM_MAXSTRIP_BIT);
#endif
    }

  protected:

    /** Bit position for maxStrip-1 [0..47]. */
    static constexpr int BKLM_MAXSTRIP_BIT = 15;

    /** Bit mask for maxStrip-1 [0..47]. */
    static constexpr int BKLM_MAXSTRIP_MASK = (63 << BKLM_MAXSTRIP_BIT);

  };

}
