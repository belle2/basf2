/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
      std::vector<float> m_thetaEdge; /**< lower theta edges from DB object */

      DBObjPtr<ECLCrystalCalib> m_ECLCrystalPhiEdge; /**< lower edges of crystals, phi */
      std::vector<float> m_phiEdge; /**< lower phi edges from DB object */

      DBObjPtr<ECLCrystalCalib> m_ECLCrystalThetaWidth; /**< width in theta */
      std::vector<float> m_thetaWidth; /**< crystal theta widths from DB object */

      DBObjPtr<ECLCrystalCalib> m_ECLCrystalPhiWidth; /**< width in phi */
      std::vector<float> m_phiWidth; /**< crystal phi widths from DB object */

      ECL::ECLNeighbours* m_neighbours{nullptr}; /**< 8 nearest neighbours to crystal */

      std::vector<int> m_thetaIDofCrysID; /**< thetaID of each crystal ID */
      std::vector<int> m_phiIDofCrysID; /**< phiID of each crystal ID */
      std::vector<int> m_crysBetweenMech; /**< crystals between phi mechanical structure per thetaID*/
      const int m_firstBarrelThetaID = 13; /**< first barrel thetaID */
      const int m_lastBarrelThetaID = 58; /**< last barrel thetaID */

    };

  } // end of namespace ECL
} // end of namespace Belle2

