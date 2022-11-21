/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <analysis/ContinuumSuppression/HarmonicMoments.h>

using namespace Belle2;



void HarmonicMoments::calculateBasicMoments()
{
  // Loop over the particles' momenta
  for (auto& p : m_momenta) {
    // Gets momentum and costheta of the vector
    double pMag = p.R();
    double cTheta = p.Dot(m_axis) / pMag;

    // Fills the momenta.
    // This part is quite ugly, but hard-coding the Legendre polynomials makes the code
    // much faster than using the boost libraries, which are implementing the recursive formulas.
    // This implementation should also be faster than a switch...case one.
    double cTheta2 = cTheta * cTheta;
    double cTheta3 = cTheta2 * cTheta;
    double cTheta4 = cTheta2 * cTheta2;

    m_moment[0] += pMag;
    m_moment[1] += pMag * cTheta;
    m_moment[2] += pMag * 0.5 * (3.*cTheta2 - 1);
    m_moment[3] += pMag * 0.5 * (5.*cTheta3 - 3.*cTheta);
    m_moment[4] += pMag * 0.125 * (35.*cTheta4 - 30.*cTheta2 + 3.);
  }
  return;
}


void HarmonicMoments::calculateAllMoments()
{
  // Loop over the particles' momenta
  for (auto& p : m_momenta) {
    // gets momentum and costheta of the vector
    double pMag = p.R();
    double cTheta = p.Dot(m_axis) / pMag;

    // Fills the momenta.
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

    m_moment[0] += pMag;
    m_moment[1] += pMag * cTheta;
    m_moment[2] += pMag * 0.5 * (3.*cTheta2 - 1);
    m_moment[3] += pMag * 0.5 * (5.*cTheta3 - 3.*cTheta);
    m_moment[4] += pMag * 0.125 * (35.*cTheta4 - 30.*cTheta2 + 3.);
    m_moment[5] += pMag * 0.125 * (63.*cTheta5 - 70 * cTheta3 + 15.*cTheta);
    m_moment[6] += pMag * 0.0625 * (231.*cTheta6 - 315 * cTheta4 + 105 * cTheta2 - 5.);
    m_moment[7] += pMag * 0.0625 * (429.*cTheta7 - 693.*cTheta5 + 315.*cTheta3 - 35.*cTheta);
    m_moment[8] += pMag * 0.0078125 * (6435.*cTheta8 - 12012.*cTheta6 + 6930.*cTheta4 - 1260.*cTheta2 + 35.);
  }
  return;
}
