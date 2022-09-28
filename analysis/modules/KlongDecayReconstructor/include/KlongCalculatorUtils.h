/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

// dataobjects
#include <analysis/dataobjects/Particle.h>

// ROOT
#include <Math/Vector4D.h>

#include <vector>

namespace Belle2 {

  /**
   * Utility class to calculate the Klong kinematics
   */
  struct KlongCalculatorUtils {
    /**
     * Calculate kinematics of two body B decays containing a K_L0.
     * @param BMomentum 4-vector of B-meson that will be updated by this function
     * @param KMomentum 4-vector of Klong that will be updated by this function
     * @param daughters Original daughters of the B-meson
     * @param m_b PDG-mass of the B-meson
     * @param idx Array index of the other daughter. This will be assigned to the extraInfo, permID, of Klong.
     * @return true if the kinematics is physical
     */
    static bool calculateBtoKlongX(ROOT::Math::PxPyPzEVector& BMomentum,
                                   ROOT::Math::PxPyPzEVector& KMomentum,
                                   const std::vector<Particle*> daughters,
                                   double m_b,
                                   int& idx);
  };
}
