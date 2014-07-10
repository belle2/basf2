/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Mikihiko Nakao (KEK), Pablo Goldenzweig (KIT)           *
 *   Original module writen by M. Nakao for Belle                         *
 *   Ported to Belle II by P. Goldenzweig                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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

// Own include
#include <analysis/utility/PSelector.h>
#include <analysis/VariableManager/Manager.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>

#include <iostream>
#include <iomanip>
#include <math.h>
#include <vector>
#include <string>
#include <sstream>

#include <analysis/utility/KsfwMoments.h>

namespace Belle2 {

// ----------------------------------------------------------------------
// legendre
// ----------------------------------------------------------------------
  inline double
  legendre(const double z, const int i)
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
// constructor
// (copied from k_sfw.cc with minimum modification)
// ----------------------------------------------------------------------
  KsfwMoments::KsfwMoments(double Hso0_max,
                           std::vector<TVector3> p_cms_sigA,
                           std::vector<TVector3> p_cms_sigB,
                           std::vector<TVector3> p_cms_roe,
                           std::vector<int> Q_sigA,
                           std::vector<int> Q_sigB,
                           std::vector<int> Q_roe,
                           TLorentzVector p_cms_missA,
                           TLorentzVector p_cms_missB,
                           double et[2]
                          )
  {
    m_et[0] = et[0];
    m_et[1] = et[1];

    m_mm2[0] = p_cms_missA.E() > 0
               ? p_cms_missA.Mag2()
               : -p_cms_missA.E() * p_cms_missA.E() - p_cms_missA.Vect().Mag2();
    m_mm2[1] = p_cms_missB.E() > 0
               ? p_cms_missB.Mag2()
               : -p_cms_missB.E() * p_cms_missB.E() - p_cms_missB.Vect().Mag2();

    //========================
    // calculate discriminants
    //========================
    std::vector<TVector3>::iterator pi, pj;
    std::vector<int>::iterator Qi, Qj;

    // calculate Hso components
    for (int i = 0; i < 3; i++) {
      for (int k = 0; k < 5; k++) {
        m_Hso[0][i][k] = m_Hso[1][i][k] = 0;
      }
    }

    // signal A (use_finalstate_for_sig == 0)
    for (pi = p_cms_sigA.begin(), Qi = Q_sigA.begin();
         pi != p_cms_sigA.end(); pi++, Qi++) {
      const double pi_mag((*pi).Mag());
      for (pj = p_cms_roe.begin(), Qj = Q_roe.begin();
           pj != p_cms_roe.end(); pj++, Qj++) {
        const double pj_mag((*pj).Mag());
        const double ij_cos((*pi) * (*pj) / pi_mag / pj_mag);
        const int c_or_n(0 == (*Qj) ? 1 : 0);  // 0: charged 1: neutral
        for (int k = 0; k < 5; k++) {
          m_Hso[0][c_or_n][k] += (k % 2)
                                 ? (*Qi) * (*Qj) * pj_mag * legendre(ij_cos, k)
                                 : pj_mag * legendre(ij_cos, k);
        }
      }
      const double p_miss_mag(p_cms_missA.Rho());
      const double i_miss_cos((*pi)*p_cms_missA.Vect() / pi_mag / p_miss_mag);
      for (int k = 0; k < 5; k++) {
        m_Hso[0][2][k] += (k % 2) ? 0 : p_miss_mag * legendre(i_miss_cos, k);
      }
    }

    // signal B (use_finalstate_for_sig == 1)
    for (pi = p_cms_sigB.begin(), Qi = Q_sigB.begin();
         pi != p_cms_sigB.end(); pi++, Qi++) {
      const double pi_mag((*pi).Mag());
      for (pj = p_cms_roe.begin(), Qj = Q_roe.begin();
           pj != p_cms_roe.end(); pj++, Qj++) {
        const double pj_mag((*pj).Mag());
        const double ij_cos((*pi) * (*pj) / pi_mag / pj_mag);
        const int c_or_n(0 == (*Qj) ? 1 : 0);  // 0: charged 1: neutral
        for (int k = 0; k < 5; k++) {
          m_Hso[1][c_or_n][k] += (k % 2)
                                 ? (*Qi) * (*Qj) * pj_mag * legendre(ij_cos, k)
                                 : pj_mag * legendre(ij_cos, k);
        }
      }
      const double p_miss_mag(p_cms_missB.Rho());
      const double i_miss_cos((*pi)*p_cms_missB.Vect() / pi_mag / p_miss_mag);
      for (int k = 0; k < 5; k++) {
        m_Hso[1][2][k] += (k % 2) ? 0 : p_miss_mag * legendre(i_miss_cos, k);
      }
    }

    // add missing to the lists
    std::vector<TVector3> p_cms_roeA(p_cms_roe), p_cms_roeB(p_cms_roe);
    p_cms_roeA.push_back(p_cms_missA.Vect());
    p_cms_roeB.push_back(p_cms_missB.Vect());
    Q_roe.push_back(0);

    // calculate Hoo components
    for (int k = 0; k < 5; k++) {
      m_Hoo[0][k] = m_Hoo[1][k] = 0;
    }
    for (pi = p_cms_roeA.begin(), Qi = Q_roe.begin();
         pi != p_cms_roeA.end(); pi++, Qi++) {
      const double pi_mag((*pi).Mag());
      for (pj = p_cms_roeA.begin(), Qj = Q_roe.begin();
           pj != pi; pj++, Qj++) {
        const double pj_mag((*pj).Mag());
        const double ij_cos((*pi) * (*pj) / pi_mag / pj_mag);
        for (int k = 0; k < 5; k++) {
          m_Hoo[0][k] += (k % 2)
                         ? (*Qi) * (*Qj) * pi_mag * pj_mag * legendre(ij_cos, k)
                         : pi_mag * pj_mag * legendre(ij_cos, k);
        }
      }
    }
    for (pi = p_cms_roeB.begin(), Qi = Q_roe.begin();
         pi != p_cms_roeB.end(); pi++, Qi++) {
      const double pi_mag((*pi).Mag());
      for (pj = p_cms_roeB.begin(), Qj = Q_roe.begin();
           pj != pi; pj++, Qj++) {
        const double pj_mag((*pj).Mag());
        const double ij_cos((*pi) * (*pj) / pi_mag / pj_mag);
        for (int k = 0; k < 5; k++) {
          m_Hoo[1][k] += (k % 2)
                         ? (*Qi) * (*Qj) * pi_mag * pj_mag * legendre(ij_cos, k)
                         : pi_mag * pj_mag * legendre(ij_cos, k);
        }
      }
    }

    // nomalize so that it does not dependent on delta_e
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
