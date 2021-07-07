/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
