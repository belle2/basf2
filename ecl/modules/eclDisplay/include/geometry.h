/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/** @file ecl/modules/eclDisplay/geometry.h
 * This file was meant to contain several (relatively) short auxillary functions
 * that depend on geometric properties of the calorimeter and specific
 * parameters of COPPER.
 *
 * Now it only contains getter/setter for the mode of display.
 * I plan to move it either to EclFrame or EclData.
 */

namespace Belle2 {
  namespace ECLDisplayUtility {
    /**
     * Changes between display of events (0) and energy (1).
     */
    void SetMode(int i);
    /**
     * Returns current displayed mode (0 shows event count, 1 shows total energy)
     */
    int GetMode();
  }
}
