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
  struct SVDPosErrScaleFactors {

    /** position error scale factor for cluster size = 1
     */
    float scaleError_clSize1;

    /** position error scale factor for cluster size = 2
     */
    float scaleError_clSize2;

    /** position error scale factor for cluster size > 2
     */
    float scaleError_clSize3;

    /** returns the corrected cluster position error */
    float getCorrectedValue(float raw_error, int size) const
    {

      if (size == 1)
        return raw_error * scaleError_clSize1;

      if (size == 2)
        return raw_error * scaleError_clSize2;

      return raw_error * scaleError_clSize3;
    }


    ClassDef(SVDPosErrScaleFactors, 1); /**< needed by root*/

  };

}
