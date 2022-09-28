/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/modules/KlongDecayReconstructor/KlongCalculatorUtils.h>

// dataobjects
#include <mdst/dataobjects/KLMCluster.h>

using namespace Belle2;

bool KlongCalculatorUtils::calculateBtoKlongX(ROOT::Math::PxPyPzEVector& BMomentum,
                                              ROOT::Math::PxPyPzEVector& KMomentum,
                                              const std::vector<Particle*> daughters,
                                              double m_b,
                                              int& idx)
{

  ROOT::Math::PxPyPzEVector pDaughters;
  for (auto daughter : daughters) {
    if (daughter->getPDGCode() != Const::Klong.getPDGCode()) {
      pDaughters += daughter->get4Vector();
    }
  }

  ROOT::Math::PxPyPzEVector klDaughters;
  for (auto daughter : daughters) {
    if (daughter->getPDGCode() == Const::Klong.getPDGCode()) {
      klDaughters += daughter->getKLMCluster()->getMomentum();
    }
  }

  double k_mag1 = 0.;
  double k_mag2 = 0.;
  double m_k = Const::Klong.getMass();
  double m_j = 0;

  for (auto daughter : daughters) {
    if (daughter->getPDGCode() != Const::Klong.getPDGCode()) {
      m_j = daughter->getPDGMass();
      idx = daughter->getArrayIndex() + idx * 100;
    }
  }

  if (daughters.size() == 3) {
    m_j = pDaughters.M();
  }

  double s_p = (klDaughters.Vect().Unit()).Dot(pDaughters.Vect());
  double m_sum = (m_b * m_b) - (m_j * m_j) - (m_k * m_k);
  double e_j = pDaughters.E();

  double s_p2 = s_p * s_p;
  double m_sum2 = m_sum * m_sum;
  double s_pm = s_p * m_sum;
  double e_j2 = e_j * e_j;
  double m_k2 = m_k * m_k;

  k_mag1 = (s_pm + std::sqrt((s_p2) * (m_sum2) - 4 * ((e_j2) - (s_p2)) * ((e_j2) * (m_k2) - (m_sum2) / 4))) / (2 * (e_j2 - s_p2));
  k_mag2 = (s_pm - std::sqrt((s_p2) * (m_sum2) - 4 * ((e_j2) - (s_p2)) * ((e_j2) * (m_k2) - (m_sum2) / 4))) / (2 * (e_j2 - s_p2));


  ROOT::Math::PxPyPzEVector missDaughters;

  if (k_mag1 > 0)
    missDaughters = k_mag1 * klDaughters / klDaughters.P();
  else
    missDaughters = k_mag2 * klDaughters / klDaughters.P();

  missDaughters.SetE(std::sqrt(m_k * m_k + missDaughters.P2()));

  if (isnan(missDaughters.P()))
    return false;

  for (auto daughter : daughters) {
    if (daughter->getPDGCode() == Const::Klong.getPDGCode()) {
      KMomentum = missDaughters;
    }
  }

  ROOT::Math::PxPyPzEVector mom = pDaughters + missDaughters;
  mom.SetE(std::sqrt(m_b * m_b + mom.P2()));
  if (isnan(mom.P()))
    return false;
  BMomentum = mom;

  return true;
}
