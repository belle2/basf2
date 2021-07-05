/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <vxd/dataobjects/VxdID.h>

#include <vector>
#include <numeric>

namespace Belle2 {

  /// struct containing a cluster candidate for easier handling
  struct DATCONSVDClusterCandidate {
    VxdID vxdID = 0; /**< VxdID of the cluster */
    std::vector<unsigned short> strips; /**< Vector containing strips (DATCONSVDDigits) that are added */
    std::vector<unsigned short> charges; /**< Vector containing the charges of the corresponding strips that are added */
    float maxSNRinClusterCandidate = 0; /**< Maximum SNR of all the strips in the cluster candidate */

    int charge = 0; /**< Charge of the cluster */
    int seedStripIndex = 0; /**< Index of the seed strip of the cluster (0...m_Size) */
    int seedStrip = 0; /**< SVD strip (i.e. 0...511 or 0...767) that is considered the seed strip of the cluster */
    int seedCharge = 0; /**< Seed Charge of the cluster */
    float clusterPosition = 0; /**< Position of the cluster */

    /// add a new strip to the current cluster candidate if possible
    /// @param nextID VxdID of the digit to be added
    /// @param nextCharge charge (in ADU) of the digit to be added
    /// @param nextCellID strip number of the digit to be added
    /// @param nextStripSNR SNR of the digit to be added
    /// @param maxClusterSize maximum cluster size, if exceeded, start new cluster
    /// @return true if digit was successfully added to current cluster candidate, else return false
    bool add(VxdID nextID, int nextCharge, unsigned short nextCellID, float nextStripSNR, unsigned short maxClusterSize)
    {
      bool added = false;

      // do not add if you are on the wrong sensor or side
      if (vxdID != nextID) return false;

      // add if it's the first strip
      if (strips.size() == 0) added = true;

      // add if it adjacent to the last strip added (we assume that SVDRecoDigits are ordered)
      // and if cluster size would still be <= 4 strips
      if (strips.size() > 0 and nextCellID == strips.back() + 1 and strips.size() < maxClusterSize) {
        added  = true;
      }

      //add it to the vector od strips, update the seed nextCharge and index:
      if (added) {
        strips.push_back(nextCellID);
        charges.push_back(nextCharge);

        if (nextCharge > seedCharge) seedCharge = nextCharge;

        if (nextStripSNR > maxSNRinClusterCandidate) {
          maxSNRinClusterCandidate = nextStripSNR;
        }
      }
      return added;

    };

    /// calculate cluster properties once a cluster is ready to be stored
    /// @param pitch strip pitch of the sensor and sensor side on which the cluster was found
    /// @param stripsInSensor number of strips of the sensor and sensor side on which the cluster was found
    void finalizeCluster(const double pitch, const int stripsInSensor)
    {
      charge = std::accumulate(charges.begin(), charges.end(), 0);
      seedStripIndex  = strips.size() / 2 + 1;
      seedStrip       = (strips.at(0) + seedStripIndex - 1);
      seedCharge      = charges.at(seedStripIndex - 1);
      clusterPosition = pitch * (seedStrip - stripsInSensor / 2);
    };
  };

} // end namespace Belle2
