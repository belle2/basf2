/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//! Definition of bit positions/masks for elements of detector-module ID.
//! Order must be (high bit) END-SECTOR-LAYER-PLANE-STRIP (low bit) for
//! proper sorting in bklmReconstructor module.
//! Also, definition of each status bit/mask.

//! bit position for MC-generated hit that represents a particle-decay point
#define BKLM_DECAYED_BIT 23

//! bit position for out-of-time-window flag (from BKLM hit reconstruction)
#define BKLM_OUTOFTIME_BIT 24

//! bit position for above-threshold flag (scintillators only, from BKLM hit reconstruction)
#define BKLM_ABOVETHRESHOLD_BIT 26

//! bit position for used-on-track flag (from tracking muid reconstruction)
#define BKLM_ONTRACK_BIT 27

//! bit position for ghost-hit flag (from tracking muid reconstruction)
#define BKLM_GHOST_BIT 28

//! bit position for used-on-BKLM-stand-alone-track flag (from BKLMTracking reconstruction)
#define BKLM_ONSTATRACK_BIT 29

//! bit mask for MC-generated hit that represents a particle-decay point
#define BKLM_DECAYED_MASK (1 << BKLM_DECAYED_BIT)

//! bit mask for out-of-time-window flag (from BKLM hit reconstruction)
#define BKLM_OUTOFTIME_MASK (1 << BKLM_OUTOFTIME_BIT)

//! bit mask for above-threshold flag (scintillators only, from BKLM hit reconstruction)
#define BKLM_ABOVETHRESHOLD_MASK (1 << BKLM_ABOVETHRESHOLD_BIT)

//! bit mask for used-on-track flag (from tracking muid reconstruction)
#define BKLM_ONTRACK_MASK (1 << BKLM_ONTRACK_BIT)

//! bit mask for ghost-hit flag (from tracking muid reconstruction)
#define BKLM_GHOST_MASK (1 << BKLM_GHOST_BIT)

//! bit mask for used-on-BKLM-stand-alone-track flag (from BKLMTracking reconstruction)
#define BKLM_ONSTATRACK_MASK (1 << BKLM_ONSTATRACK_BIT)

//! bit mask for status bits
#define BKLM_STATUS_MASK (BKLM_DECAYED_MASK | BKLM_OUTOFTIME_MASK | BKLM_ONTRACK_MASK | BKLM_ABOVETHRESHOLD_MASK | BKLM_ONSTATRACK_MASK)

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
