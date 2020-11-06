/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Mikihiko Nakao (KEK), Pablo Goldenzweig (KIT),          *
 *                Umberto Tamponi (tamponi@to.infn.it)                    *
 *                                                                        *
 * Original module writen by M. Nakao for Belle                           *
 * Ported to Belle II by P. Goldenzweig                                   *
 * Modified by U. Tamponi                                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/ContinuumSuppression/FoxWolfram.h>

using namespace Belle2;

void FoxWolfram::calculateBasicMoments()
{
  // clear the momenta, in case someone calls this function twice
  for (double& i : m_moment)
    i = 0.;

  const auto begin = m_momenta.begin();
  const auto end = m_momenta.end();

  // loops over the vector pairs
  for (auto iter1 = begin; iter1 != end; iter1++) {
    const TVector3 pVec1 = (*iter1);
    double pMag1 = pVec1.Mag();

    // avoids to iterate twice over the same pairs
    for (auto iter2 = iter1; iter2 != end; iter2++) {
      const TVector3 pVec2 = (*iter2);
      double magProd = pMag1 * pVec2.Mag();  // product of the vector's magnitudes
      double cTheta = pVec1.Dot(pVec2) / magProd; // costheta_ij

      // Since the FW moment definition requires to double count all the
      // pairs of different particles,  but the smart loop implemented here doesn't,
      // multiply each entry by 2.
      if (iter1 != iter2) magProd *= 2;

      // Fills the moments' list.
      // This part is quite ugly, but hard-coding the Legendre polynomials makes the code
      // much faster than using the boost libraries, which are implementing the recursive formulas.
      // This implementation should also be faster than a switch...case one.
      double cTheta2 = cTheta * cTheta;
      double cTheta3 = cTheta2 * cTheta;
      double cTheta4 = cTheta2 * cTheta2;
      m_moment[0] += magProd;
      m_moment[1] += magProd * cTheta;
      m_moment[2] += magProd * 0.5 * (3.*cTheta2 - 1);
      m_moment[3] += magProd * 0.5 * (5.*cTheta3 - 3.*cTheta);
      m_moment[4] += magProd * 0.125 * (35.*cTheta4 - 30.*cTheta2 + 3.);
    }
  }
  return;
}



void FoxWolfram::calculateAllMoments()
{
  // clear the momenta, in case someone calls this function twice
  for (double& i : m_moment)
    i = 0.;

  const auto begin = m_momenta.begin();
  const auto end = m_momenta.end();

  // loops over the vector pairs
  for (auto iter1 = begin; iter1 != end; iter1++) {
    const TVector3 pVec1 = (*iter1);
    double pMag1 = pVec1.Mag();

    // avoids to iterate twice over the same pairs
    for (auto iter2 = iter1; iter2 != end; iter2++) {
      const TVector3 pVec2 = (*iter2);
      double magProd = pMag1 * pVec2.Mag();  // product of the vector's magnitudes
      double cTheta = pVec1.Dot(pVec2) / magProd; // costheta_ij

      // Since the FW moment definition requires to double count all the
      // pairs of different particles,  but the smart loop implemented here doesn't,
      // multiply each entry by 2.
      if (iter1 != iter2) magProd *= 2;

      // Fills the moments' list.
      // This part is quite ugly, but hard-coding the Legendre polynomials makes the code
      // much faster than using the boost libraries, which are implementing the recursive formulas.
      // This implementation should also be faster than a switch...case one.
      double cTheta2 = cTheta * cTheta;
      double cTheta3 = cTheta2 * cTheta;
      double cTheta4 = cTheta2 * cTheta2;
      double cTheta5 = cTheta4 * cTheta;
      double cTheta6 = cTheta3 * cTheta3;
      double cTheta7 = cTheta6 * cTheta;
      double cTheta8 = cTheta4 * cTheta4;

      m_moment[0] += magProd;
      m_moment[1] += magProd * cTheta;
      m_moment[2] += magProd * 0.5 * (3.*cTheta2 - 1);
      m_moment[3] += magProd * 0.5 * (5.*cTheta3 - 3.*cTheta);
      m_moment[4] += magProd * 0.125 * (35.*cTheta4 - 30.*cTheta2 + 3.);
      m_moment[5] += magProd * 0.125 * (63.*cTheta5 - 70 * cTheta3 + 15.*cTheta);
      m_moment[6] += magProd * 0.0625 * (231.*cTheta6 - 315 * cTheta4 + 105 * cTheta2 - 5.);
      m_moment[7] += magProd * 0.0625 * (429.*cTheta7 - 693.*cTheta5 + 315.*cTheta3 - 35.*cTheta);
      m_moment[8] += magProd * 0.0078125 * (6435.*cTheta8 - 12012.*cTheta6 + 6930.*cTheta4 - 1260.*cTheta2 + 35.);
    }
  }
  return;
}

