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
//  KsfwMoments.h                       //
//                                      //
//  moment-calculation of the k_sfw     //
//  improved Super-Fox-Wolfram moments  //
//  to be used with rooksfw.{cc,h}      //
//                                      //
//  M. Nakao                            //
//                                      //
//////////////////////////////////////////

#ifndef KSFWMOMENTS_H
#define KSFWMOMENTS_H

#include <vector>
#include <map>
#include <string>
#include <utility>
#include <iostream>
#include <sstream>

#include <analysis/VariableManager/Manager.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <framework/datastore/StoreArray.h>
#include <analysis/ContinuumSuppression/FuncPtr.h>

#include <analysis/utility/PCmsLabTransform.h>

namespace Belle2 {

// TODO: are these necessary? doesn't compile if include 'class TVector3'.
  class Particle;
//class TVector3;

// ----------------------------------------------------------------------
// KsfwMoments
// ----------------------------------------------------------------------
  class KsfwMoments {
  public:
    KsfwMoments() : m_uf(0) {

      for (int i = 0; i < 2; i++)
        for (int j = 0; j < 3; j++)
          for (int k = 0; k < 5; k++)
            m_Hso[i][j][k] = 0.0;

      for (int i = 0; i < 2; i++)
        for (int j = 0; j < 5; j++)
          m_Hoo[i][j] = 0.0;

      for (int i = 0; i < 2; i++) {
        m_et[i] = 0.0;
        m_mm2[i] = 0.0;
      }
    };

    KsfwMoments(double Hso0_max,
                std::vector<std::pair<TVector3, int>> p3_cms_q_sigA,
                std::vector<std::pair<TVector3, int>> p3_cms_q_sigB,
                std::vector<std::pair<TVector3, int>> p3_cms_q_roe,
                TLorentzVector p_cms_missA,
                TLorentzVector p_cms_missB,
                double et[2]
               );
    ~KsfwMoments() {};

    int usefinal(int uf) { if (uf == 0 || uf == 1) m_uf = uf; return m_uf; }

    int usefinal() const { return m_uf; }
    double mm2(int uf = -1) const { return m_mm2[uf < 0 ? m_uf : uf]; }
    double et(int uf = -1) const { return m_et[uf < 0 ? m_uf : uf]; }
    double Hoo(int i, int uf = -1) const {
      return m_Hoo[uf < 0 ? m_uf : uf][i];
    }
    double Hso(int i, int j, int uf = -1) const {
      return m_Hso[uf < 0 ? m_uf : uf][i][j];
    }
    double var(int i) { // the same order as the original k_sfw.cc
      if (i < 0)  return 0;
      if (i < 1)  return et();
      if (i < 4)  return Hso(i - 1, 0);
      if (i < 5)  return Hso(0,   1);
      if (i < 8)  return Hso(i - 5, 2);
      if (i < 9)  return Hso(0,   3);
      if (i < 12) return Hso(i - 9, 4);
      if (i < 17) return Hoo(i - 12);
      return 0;
    }

  private:
    int m_uf;
    double m_Hso[2][3][5];
    double m_Hoo[2][5];
    double m_et[2];
    double m_mm2[2];
  };

} // Belle2 namespace

#endif
