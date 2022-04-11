/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//////////////////////////////////////////
//                                      //
//  KsfwMoments.cc                      //
//                                      //
//  moment-calculation of the k_sfw     //
//  improved Super-Fox-Wolfram moments  //
//  to be used with rooksfw.{cc,h}      //
//                                      //
//  M. Nakao                            //
//                                      //
//////////////////////////////////////////

#include <analysis/ContinuumSuppression/KsfwMoments.h>


namespace Belle2 {

  /**
   * Legendre polynomials
   */
  inline double legendre(const double z, const int i)
  {
    switch (i) {
      case 0:  return 1.;
      case 1:  return z;
      case 2:  return 1.5 * z * z - 0.5;
      case 3:  return z * (2.5 * z * z - 1.5);
      case 4:  return (4.375 * z * z * z * z - 3.75 * z * z + 0.375);
      default: return 0;
    }
  }
// ----------------------------------------------------------------------
// Constructor
// (copied from k_sfw.cc with minimum modification)
// ----------------------------------------------------------------------
  KsfwMoments::KsfwMoments(double Hso0_max,
                           std::vector<std::pair<ROOT::Math::XYZVector, int>> p3_cms_q_sigA,
                           std::vector<std::pair<ROOT::Math::XYZVector, int>> p3_cms_q_sigB,
                           std::vector<std::pair<ROOT::Math::XYZVector, int>> p3_cms_q_roe,
                           const ROOT::Math::PxPyPzEVector& p_cms_missA,
                           const ROOT::Math::PxPyPzEVector& p_cms_missB,
                           const double et[2]
                          )
  {
    // Private member needs to be initialized. Here it is initialized to -1 (illegal value).
    m_uf = -1;

    m_et[0] = et[0];
    m_et[1] = et[1];

    m_mm2[0] = p_cms_missA.E() > 0
               ? p_cms_missA.mag2()
               : -p_cms_missA.E() * p_cms_missA.E() - p_cms_missA.P2();
    m_mm2[1] = p_cms_missB.E() > 0
               ? p_cms_missB.mag2()
               : -p_cms_missB.E() * p_cms_missB.E() - p_cms_missB.P2();

    //========================
    // Calculate discriminants
    //========================
    std::vector<std::pair<ROOT::Math::XYZVector, int>>::iterator pqi, pqj;

    // Calculate Hso components
    for (int i = 0; i < 3; i++) {
      for (int k = 0; k < 5; k++) {
        m_Hso[0][i][k] = m_Hso[1][i][k] = 0;
      }
    }

    // Signal A (use_finalstate_for_sig == 0)
    for (pqi = p3_cms_q_sigA.begin(); pqi != p3_cms_q_sigA.end(); ++pqi) {
      const double pi_mag((pqi->first).R());
      for (pqj = p3_cms_q_roe.begin(); pqj != p3_cms_q_roe.end(); ++pqj) {
        const double pj_mag((pqj->first).R());
        const double ij_cos((pqi->first).Dot(pqj->first) / pi_mag / pj_mag);
        const int c_or_n(0 == (pqj->second) ? 1 : 0);  // 0: charged 1: neutral
        for (int k = 0; k < 5; k++) {
          m_Hso[0][c_or_n][k] += (k % 2)
                                 ? (pqi->second) * (pqj->second) * pj_mag * legendre(ij_cos, k)
                                 : pj_mag * legendre(ij_cos, k);
        }
      }
      const double p_miss_mag(p_cms_missA.P());
      const double i_miss_cos((pqi->first).Dot(p_cms_missA.Vect()) / pi_mag / p_miss_mag);
      for (int k = 0; k < 5; k++) {
        m_Hso[0][2][k] += (k % 2) ? 0 : p_miss_mag * legendre(i_miss_cos, k);
      }
    }

    // Signal B (use_finalstate_for_sig == 1)
    for (pqi = p3_cms_q_sigB.begin(); pqi != p3_cms_q_sigB.end(); ++pqi) {
      const double pi_mag((pqi->first).R());
      for (pqj = p3_cms_q_roe.begin(); pqj != p3_cms_q_roe.end(); ++pqj) {
        const double pj_mag((pqj->first).R());
        const double ij_cos((pqi->first).Dot(pqj->first) / pi_mag / pj_mag);
        const int c_or_n(0 == (pqj->second) ? 1 : 0);  // 0: charged 1: neutral
        for (int k = 0; k < 5; k++) {
          m_Hso[1][c_or_n][k] += (k % 2)
                                 ? (pqi->second) * (pqj->second) * pj_mag * legendre(ij_cos, k)
                                 : pj_mag * legendre(ij_cos, k);
        }
      }
      const double p_miss_mag(p_cms_missB.P());
      const double i_miss_cos((pqi->first).Dot(p_cms_missB.Vect()) / pi_mag / p_miss_mag);
      for (int k = 0; k < 5; k++) {
        m_Hso[1][2][k] += (k % 2) ? 0 : p_miss_mag * legendre(i_miss_cos, k);
      }
    }

    // Add missing to the lists
    std::vector<std::pair<ROOT::Math::XYZVector, int>> p3_cms_q_roeA(p3_cms_q_roe), p3_cms_q_roeB(p3_cms_q_roe);
    p3_cms_q_roeA.emplace_back(p_cms_missA.Vect(), 0);
    p3_cms_q_roeB.emplace_back(p_cms_missB.Vect(), 0);

    // Calculate Hoo components
    for (int k = 0; k < 5; k++) {
      m_Hoo[0][k] = m_Hoo[1][k] = 0;
    }
    for (pqi = p3_cms_q_roeA.begin(); pqi != p3_cms_q_roeA.end(); ++pqi) {
      const double pi_mag((pqi->first).R());
      for (pqj = p3_cms_q_roeA.begin(); pqj != pqi; ++pqj) {
        const double pj_mag((pqj->first).R());
        const double ij_cos((pqi->first).Dot(pqj->first) / pi_mag / pj_mag);
        for (int k = 0; k < 5; k++) {
          m_Hoo[0][k] += (k % 2)
                         ? (pqi->second) * (pqj->second) * pi_mag * pj_mag * legendre(ij_cos, k)
                         : pi_mag * pj_mag * legendre(ij_cos, k);
        }
      }
    }
    for (pqi = p3_cms_q_roeB.begin(); pqi != p3_cms_q_roeB.end(); ++pqi) {
      const double pi_mag((pqi->first).R());
      for (pqj = p3_cms_q_roeB.begin(); pqj != pqi; ++pqj) {
        const double pj_mag((pqj->first).R());
        const double ij_cos((pqi->first).Dot(pqj->first) / pi_mag / pj_mag);
        for (int k = 0; k < 5; k++) {
          m_Hoo[1][k] += (k % 2)
                         ? (pqi->second) * (pqj->second) * pi_mag * pj_mag * legendre(ij_cos, k)
                         : pi_mag * pj_mag * legendre(ij_cos, k);
        }
      }
    }

    // Normalize so that it does not dependent on delta_e
    for (int k = 0; k < 5; k++) {
      for (int j = 0; j < ((k % 2) ? 1 : 3); j++) {
        m_Hso[0][j][k] /= Hso0_max;
        m_Hso[1][j][k] /= Hso0_max;
      }
      m_Hoo[0][k] /= (Hso0_max * Hso0_max);
      m_Hoo[1][k] /= (Hso0_max * Hso0_max);
    }

  }

} // Belle2 namespace
