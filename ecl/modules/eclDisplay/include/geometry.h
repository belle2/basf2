/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Milkail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 ***************************************************************************/

#ifndef GEOMETRY_H
#define GEOMETRY_H

// This file was meant to contain several (relatively) short auxillary functions
// that depend on geometric properties of the calorimeter and specific
// parameters of COPPER.

// Now it only contains getter/setter for the mode of display.
// I plan to move it either to EclFrame or EclData.

namespace Belle2 {
  namespace ECLDisplayUtility {
    /**
     * @brief Changes between display of events and energy.
     */
    void SetMode(int i);
    /**
     * @brief Returns current displayed mode (0 shows event count, 1 shows total energy)
     */
    int GetMode();
  }
}

#endif // GEOMETRY_H
