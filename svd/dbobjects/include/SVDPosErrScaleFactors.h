/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {

  /** contains the scaling factors for the cluster position error*/
  struct SVDPosErrScaleFactors final {

    /** position error scale factor for cluster size = 1
     */
    float scaleError_clSize1 = 1;

    /** position error scale factor for cluster size = 2
     */
    float scaleError_clSize2 = 1;

    /** position error scale factor for cluster size = 3
     */
    float scaleError_clSize3 = 1;

    /** position error scale factor for cluster size = 4
     */
    float scaleError_clSize4 = 1;

    /** position error scale factor for cluster size > 4
     */
    float scaleError_clSize5 = 1;

    /** returns the corrected cluster position error */
    float getCorrectedValue(float raw_error, int size) const
    {

      if (size == 1)
        return raw_error * scaleError_clSize1;

      if (size == 2)
        return raw_error * scaleError_clSize2;

      if (size == 3)
        return raw_error * scaleError_clSize3;

      if (size == 4)
        return raw_error * scaleError_clSize4;

      return raw_error * scaleError_clSize5;
    }

    ClassDef(SVDPosErrScaleFactors, 2); /**< needed by root*/

  };

}
