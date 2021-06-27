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

  /** contains the clustering parameters*/
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


    ClassDef(SVDClusterCuts, 2); /**< needed by root*/

  };

}
