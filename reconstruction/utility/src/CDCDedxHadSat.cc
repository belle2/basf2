/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/utility/CDCDedxHadSat.h>

using namespace Belle2;

void CDCDedxHadSat::setParameters(std::string infile)
{

  B2INFO("\n\t Hadron Saturation: Using parameters from file --> " << infile);

  std::ifstream fin;
  fin.open(infile.c_str());

  if (!fin.good()) B2FATAL("\tWARNING: CANNOT FIND parameters.txt!");

  fin >> m_alpha ;
  fin >> m_gamma ;
  fin >> m_delta ;
  fin >> m_power ;
  fin >> m_ratio ;

  fin.close();
}

void CDCDedxHadSat::setParameters()
{

  if (!m_DBHadronCor || m_DBHadronCor->getSize() == 0) {
    B2FATAL("No hadron correction parameters!");
  } else {
    m_alpha = m_DBHadronCor->getHadronPar(0);
    m_gamma = m_DBHadronCor->getHadronPar(1);
    m_delta = m_DBHadronCor->getHadronPar(2);
    m_power = m_DBHadronCor->getHadronPar(3);
    m_ratio = m_DBHadronCor->getHadronPar(4);
  }
}


// define these here to be used in other classes
double CDCDedxHadSat::D2I(double cosTheta, double D = 1) const
{

  double absCosTheta   = fabs(cosTheta);
  double projection    = pow(absCosTheta, m_power) + m_delta;

  if (projection == 0) {
    B2WARNING("Something wrong with dE/dx hadron ants!");
    return D;
  }

  double chargeDensity = D / projection;
  double numerator     = 1 + m_alpha * chargeDensity;
  double denominator   = 1 + m_gamma * chargeDensity;
  if (denominator == 0) {
    B2WARNING("Something wrong with dE/dx hadron ants!");
    return D;
  }
  double I = D * m_ratio * numerator / denominator;

  return I;
}

double CDCDedxHadSat::I2D(double cosTheta, double I = 1) const
{

  double absCosTheta = fabs(cosTheta);
  double projection  = pow(absCosTheta, m_power) + m_delta;

  if (projection == 0 || m_ratio == 0) {
    B2WARNING("Something wrong with dE/dx hadron ants!");
    return I;
  }
  double a =  m_alpha / projection;
  double b =  1 - m_gamma / projection * (I / m_ratio);
  double c = -1.0 * I / m_ratio;

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
  return D;
}

double
CDCDedxHadSat::D2I(double cosTheta, double D, double alpha, double gamma, double delta, double power, double ratio) const
{

  double absCosTheta   = fabs(cosTheta);
  double projection    = pow(absCosTheta, power) + delta;
  if (projection == 0) {
    B2INFO("\t HadronSaturation: D2I Something wrong with dE/dx hadron constants!");
    return D;
  }

  double chargeDensity = D / projection;
  double numerator     = 1 + alpha * chargeDensity;
  double denominator   = 1 + gamma * chargeDensity;
  if (denominator == 0) {
    B2INFO("\t HadronSaturation: D2I Something wrong with dE/dx hadron constants!");
    return D;
  }

  double I = D * ratio * numerator / denominator;

  return I;
}

double
CDCDedxHadSat::I2D(double cosTheta, double I, double alpha, double gamma, double delta, double power, double ratio) const
{

  double absCosTheta = fabs(cosTheta);
  double projection  = pow(absCosTheta, power) + delta;

  if (projection == 0 || ratio == 0) {
    B2INFO("\t HadronSaturation: I2D Something wrong with dE/dx hadron constants!");
    return I;
  }

  double a =  alpha / projection;
  double b =  1 - (gamma / projection) * (I / ratio);
  double c = -1.0 * I / ratio;

  if (b == 0 && a == 0) {
    B2INFO("\t HadronSaturation: both a and b coefficiants for hadron correction are 0");
    return I;
  }

  double discr = b * b - 4.0 * a * c;
  if (discr < 0) {
    B2INFO("negative discriminant; return uncorrectecd value");
    return I;
  }

  double D = (a != 0) ? (-b + sqrt(discr)) / (2.0 * a) : -c / b;

  if (D < 0) {
    B2INFO("D is less 0! will try another solution");
    D = (a != 0) ? (-b - sqrt(discr)) / (2.0 * a) : -c / b;
    if (D < 0) {
      B2INFO("D is still less 0! just return uncorrectecd value");
      return I;
    }
  }

  return D;
}