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
   * ECL element numbers.
   */
  namespace ECLElementNumbers {

    /** Number of crystals. */
    const int c_NCrystals = 8736;

    /** Number of crystals in the forward ECL. */
    const int c_NCrystalsForward = 1152;

    /** Number of crystals in the barrel ECL. */
    const int c_NCrystalsBarrel = 6624;

    /** Number of crystals in the backward ECL. */
    const int c_NCrystalsBackward = 960;

    /** Number of crystals in the forward and barrel ECL. */
    const int c_NCrystalsForwardBarrel = c_NCrystalsForward + c_NCrystalsBarrel;

    /**
     * Check whether the crystal is in forward ECL.
     * @param[in] cellId Cell identifier (1-based).
     */
    inline bool isForward(int cellId)
    {
      return cellId <= c_NCrystalsForward;
    }

    /**
     * Check whether the crystal is in barrel ECL.
     * @param[in] cellId Cell identifier (1-based).
     */
    inline bool isBarrel(int cellId)
    {
      return (c_NCrystalsForward < cellId) &&
             (cellId <= c_NCrystalsForwardBarrel);
    }

    /**
     * Check whether the crystal is in backward ECL.
     * @param[in] cellId Cell identifier (1-based).
     */
    inline bool isBackward(int cellId)
    {
      return c_NCrystalsForwardBarrel < cellId;
    }

  }

}
