/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/dbobjects/CDCDedxHadronCor.h>
#include <framework/logging/Logger.h>
#include <cmath>

namespace Belle2 {

  double CDCDedxHadronCor::D2I(double cosTheta, double D) const
  {
    const auto& params = m_hadronpars;
    if (params.size() < 5) {
      B2WARNING("Vector of dE/dx hadron constants too short!");
      return D;
    }

    double projection = std::pow(std::abs(cosTheta), params[3]) + params[2];
    if (projection == 0) {
      B2WARNING("Something wrong with dE/dx hadron constants!");
      return D;
    }

    double chargeDensity = D / projection;
    double numerator = 1 + params[0] * chargeDensity;
    double denominator = 1 + params[1] * chargeDensity;

    if (denominator == 0) {
      B2WARNING("Something wrong with dE/dx hadron constants!");
      return D;
    }

    double I = D * params[4] * numerator / denominator;
    return I;
  }


  double CDCDedxHadronCor::I2D(double cosTheta, double I) const
  {
    const auto& params = m_hadronpars;
    if (params.size() < 5) {
      B2WARNING("Vector of dE/dx hadron constants too short!");
      return I;
    }

    double projection  = std::pow(std::abs(cosTheta), params[3]) + params[2];
    if (projection == 0 or params[4] == 0) {
      B2WARNING("Something wrong with dE/dx hadron constants!");
      return I;
    }

    double a =  params[0] / projection;
    double b =  1 - params[1] / projection * (I / params[4]);
    double c = -1.0 * I / params[4];

    if (b == 0 and a == 0) {
      B2WARNING("both a and b coefficiants for hadron correction are 0");
      return I;
    }

    double discr = b * b - 4.0 * a * c;
    if (discr < 0) {
      B2WARNING("negative discriminant; return uncorrectecd value");
      return I;
    }

    double D = (a != 0) ? (-b + std::sqrt(discr)) / (2.0 * a) : -c / b;
    if (D < 0) {
      D = (a != 0) ? (-b - std::sqrt(discr)) / (2.0 * a) : -c / b;
      if (D < 0) {
        B2WARNING("D is less than 0; return uncorrectecd value");
        return I;
      }
    }

    return D;
  }

}
