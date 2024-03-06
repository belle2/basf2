/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/utility/CDCDedxHadSat.h>

using namespace Belle2;

double CDCDedxHadSat::D2I(double cosTheta,  double D)
{
  getHadronVector();
  std::cout << "Entering D2I" << std::endl;
  std::cout << m_hadronpars[0] << std::endl;

  double absCosTheta   = fabs(cosTheta);
  double projection    = pow(absCosTheta, m_hadronpars[3]) + m_hadronpars[2];
  if (projection == 0) {
    B2WARNING("Something wrong with dE/dx hadron ants!");
    return D;
  }

  double chargeDensity = D / projection;
  double numerator     = 1 + m_hadronpars[0] * chargeDensity;
  double denominator   = 1 + m_hadronpars[1] * chargeDensity;

  if (denominator == 0) {
    B2WARNING("Something wrong with dE/dx hadron ants!");
    return D;
  }

  double I = D * m_hadronpars[4] * numerator / denominator;
  std::cout << "I:   " << I << std::endl;

  return I;
}

double CDCDedxHadSat::I2D(double cosTheta,  double I)
{
  getHadronVector();
  std::cout << "Entering I2D" << std::endl;
  std::cout << m_hadronpars[0] << std::endl;

  double absCosTheta = fabs(cosTheta);
  double projection  = pow(absCosTheta, m_hadronpars[3]) + m_hadronpars[2];

  if (projection == 0 || m_hadronpars[4] == 0) {
    B2WARNING("Something wrong with dE/dx hadron ants!");
    return I;
  }

  double a =  m_hadronpars[0] / projection;
  double b =  1 - m_hadronpars[1] / projection * (I / m_hadronpars[4]);
  double c = -1.0 * I / m_hadronpars[4];

  if (b == 0 && a == 0) {
    B2WARNING("both a and b coefficiants for hadron correction are 0");
    return I;
  }

  double discr = b * b - 4.0 * a * c;
  if (discr < 0) {
    B2WARNING("negative discriminant; return uncorrectecd value");
    return I;
  }

  double D = (a != 0) ? (-b + sqrt(discr)) / (2.0 * a) : -c / b;
  if (D < 0) {
    B2WARNING("D is less 0! will try another solution");
    D = (a != 0) ? (-b - sqrt(discr)) / (2.0 * a) : -c / b;
    if (D < 0) {
      B2WARNING("D is still less 0! just return uncorrectecd value");
      return I;
    }
  }
  std::cout << "D:   " << D << std::endl;
  return D;
}
