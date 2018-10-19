/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {

  struct SVDClusterCuts {

    /** minimum value of SNR of the seed strip
     */
    float minSeedSNR;

    /** minimum value of SNR of the adjacent strip
     */
    float minAdjSNR;

    /** minimum value of SNR of the cluster
     */
    float minClusterSNR;

    /** scale factor for cluster size = 1
     */
    float scaleError_clSize1;

    /** scale factor for cluster size = 2
     */
    float scaleError_clSize2;

    /** scale factor for cluster size > 2
     */
    float scaleError_clSize3;

    void set_minClusterSNR(float a)
    {
      minClusterSNR = a;
    }
    float getCorrectedValue(float raw_error, int size)
    {

      if (size == 1)
        return raw_error * scaleError_clSize1;

      if (size == 2)
        return raw_error * scaleError_clSize2;

      return raw_error * scaleError_clSize3;
    }


    ClassDef(SVDClusterCuts, 1);

  };

}
