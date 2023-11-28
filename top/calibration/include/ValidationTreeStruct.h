/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <cmath>

namespace Belle2 {
  namespace TOP {

    /**
     * Calibration validation tree structure
     */
    struct ValidationTreeStruct {
      enum {c_numModules = 16}; /**< number of modules */
      int expNo = 0; /**< experiment number */
      int runNo = 0; /**< run number */
      int numTracks = 0; /**< number of selected tracks */
      float commonT0 = 0;  /**< common T0 residual */
      float commonT0Err = 0; /**< common T0 uncertainty (not scaled) */
      float moduleT0[c_numModules] = {0};  /**< module T0 residuals, index = slot - 1 */
      float moduleT0Err[c_numModules] = {0}; /**< module T0 uncertainties (not scaled), index = slot - 1 */
      int numTBCalibrated[c_numModules] = {0}; /**< number of timebase calibrated channels, index = slot - 1 */
      int numT0Calibrated[c_numModules] = {0}; /**< number of channel T0 calibrated channels, index = slot - 1 */
      int numActive[c_numModules] = {0}; /**< number of active channels, index = slot - 1 */
      int numActiveCalibrated[c_numModules] = {0}; /**< number of active calibrated channels, index = slot - 1 */
      float thrEffi[c_numModules] = {0}; /**< threshold efficiency: average over active calibrated channels, index = slot - 1 */
      float asicShifts[4] = {0}; /**< carrier shifts of BS13d, index = carrier number */
      float svdOffset = 0; /**< SVD event T0 offset */
      float svdSigma = 0; /**< SVD event T0 resolution */
      float cdcOffset = 0; /**< CDC event T0 offset */
      float cdcSigma = 0; /**< CDC event T0 resolution */
      float fillPatternOffset = 0; /**< fill pattern offset */
      float fillPatternFraction = 0; /**< fraction of reconstructed buckets matched with filled ones */
      int numMerged = 0; /**< number of merged entries */

      /**
       * Clear the structure
       */
      void clear();

      /**
       * Clear the number of merged entries
       */
      void clearNumMerged() {numMerged = 0;}

      /**
       * Merge two structures
       * @param other structure to be merged with this one
       */
      void merge(const ValidationTreeStruct& other);

      /**
       * Rescale errors
       * @param scaleFactor scale factor
       */
      void rescaleErrors(double scaleFactor);

      /**
       * Add (x1, e1) to (x, e) using weighting by errors (e.g. least square fit of a constant)
       * @param x input/output value
       * @param e input/output error
       * @param x1 value to be added
       * @param e1 error to be added
       */
      void add(float& x, float& e, float x1, float e1);

    };


    inline void ValidationTreeStruct::add(float& x, float& e, float x1, float e1)
    {
      if (e1 == 0) return;
      if (e == 0) {
        x = x1;
        e = e1;
        return;
      }
      e *= e;
      e1 *= e1;
      float s = 1 / e + 1 / e1;
      x = (x / e + x1 / e1) / s;
      e = sqrt(1 / s);
    }

  } // TOP
} // Belle2
