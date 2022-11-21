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
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <analysis/ClusterUtility/ClusterUtils.h>

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
                                   const double m_b,
                                   int& idx)
    {

      ROOT::Math::PxPyPzEVector klDaughters;
      ROOT::Math::PxPyPzEVector pDaughters;
      double m_j = 0;
      for (auto daughter : daughters) {
        if (daughter->getPDGCode() == Const::Klong.getPDGCode()) {
          const Belle2::KLMCluster* klm_cluster = daughter->getKLMCluster();
          const Belle2::ECLCluster* ecl_cluster = daughter->getECLCluster();
          if (klm_cluster != nullptr)
            klDaughters += klm_cluster->getMomentum();
          else if (ecl_cluster != nullptr) {
            ClusterUtils clutls;
            klDaughters += clutls.Get4MomentumFromCluster(ecl_cluster, ECLCluster::EHypothesisBit::c_neutralHadron);
          }
        } else {
          pDaughters += daughter->get4Vector();

          m_j = daughter->getPDGMass();
          idx = daughter->getArrayIndex() + idx * 100;
        }
      }

      if (daughters.size() == 3) {
        m_j = pDaughters.M();
      }

      const double m_k = Const::Klong.getMass();

      const double s_p = (klDaughters.Vect().Unit()).Dot(pDaughters.Vect());
      const double m_sum = (m_b * m_b) - (m_j * m_j) - (m_k * m_k);

      const double s_p2 = s_p * s_p;
      const double m_sum2 = m_sum * m_sum;
      const double s_pm = s_p * m_sum;
      const double e_j2 = pDaughters.E() * pDaughters.E();
      const double m_k2 = m_k * m_k;

      const double k_mag1 = (s_pm + std::sqrt((s_p2) * (m_sum2) - 4 * ((e_j2) - (s_p2)) * ((e_j2) * (m_k2) - (m_sum2) / 4))) / (2 *
                            (e_j2 - s_p2));
      const double k_mag2 = (s_pm - std::sqrt((s_p2) * (m_sum2) - 4 * ((e_j2) - (s_p2)) * ((e_j2) * (m_k2) - (m_sum2) / 4))) / (2 *
                            (e_j2 - s_p2));


      ROOT::Math::PxPyPzEVector missDaughters;

      if (k_mag1 > 0)
        missDaughters = k_mag1 * klDaughters / klDaughters.P();
      else
        missDaughters = k_mag2 * klDaughters / klDaughters.P();

      missDaughters.SetE(std::sqrt(m_k * m_k + missDaughters.P2()));

      if (isnan(missDaughters.P()))
        return false;

      ROOT::Math::PxPyPzEVector mom = pDaughters + missDaughters;
      mom.SetE(std::sqrt(m_b * m_b + mom.P2()));
      if (isnan(mom.P()))
        return false;

      KMomentum = missDaughters;
      BMomentum = mom;

      return true;
    };

  };
}
