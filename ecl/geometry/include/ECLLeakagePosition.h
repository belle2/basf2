/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Return the location information need for photon energy leakage         *
 * corrections                                                            *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty hearty@physics.ubc.ca                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/database/DBObjPtr.h>

namespace Belle2 {
  class ECLCrystalCalib;
  namespace ECL {
    class ECLNeighbours;

    /**
     * Class to get position information for a cluster for leakage corrections
     */
    class ECLLeakagePosition {

    public:

      /**  Constructor */
      ECLLeakagePosition();

      /**  Destructor. */
      ~ECLLeakagePosition();

      /** Return postion. Elements of returned vector: */
      /** cellID, thetaID, region, localThetaBin, localPhiBin, phiMech, status */
      /** region: 0 = forward, 1 = barrel, 2 = backward */
      /** localPhiBin is from edge with mechanical structure, or else lower edge */
      /** phiMech: 0 = mechanical structure on phi edge; 1 = no mechanical structure */
      /** status: 0 = cellID is max energy crystal; 1 = neighbour; 2 = more distant */
      std::vector<int> getLeakagePosition(const int cellIDFromEnergy, const float theta, const float phi, const int nPositions);

    private:

      /** Required geometry payloads */
      DBObjPtr<ECLCrystalCalib> m_ECLCrystalThetaEdge; /**< lower edges of crystals, theta */
      std::vector<float> thetaEdge; /**< lower theta edges from DB object */

      DBObjPtr<ECLCrystalCalib> m_ECLCrystalPhiEdge; /**< lower edges of crystals, phi */
      std::vector<float> phiEdge; /**< lower phi edges from DB object */

      DBObjPtr<ECLCrystalCalib> m_ECLCrystalThetaWidth; /**< width in theta */
      std::vector<float> thetaWidth; /**< crystal theta widths from DB object */

      DBObjPtr<ECLCrystalCalib> m_ECLCrystalPhiWidth; /**< width in phi */
      std::vector<float> phiWidth; /**< crystal phi widths from DB object */

      ECL::ECLNeighbours* neighbours{nullptr}; /**< 8 nearest neighbours to crystal */

      std::vector<int> thetaIDofCrysID; /**< thetaID of each crystal ID */
      std::vector<int> phiIDofCrysID; /**< phiID of each crystal ID */
      std::vector<int> crysBetweenMech; /**< crystals between phi mechanical structure per thetaID*/
      const int firstBarrelThetaID = 13; /**< first barrel thetaID */
      const int lastBarrelThetaID = 58; /**< last barrel thetaID */

    };

  } // end of namespace ECL
} // end of namespace Belle2

