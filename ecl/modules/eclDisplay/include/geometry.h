/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Milkail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 ***************************************************************************/

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
