/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/utility/CDCDedxMeanPred.h>

using namespace Belle2;

double CDCDedxMeanPred::meanCurve(double* x, double* par, int version) const
{
  // calculate the predicted mean value as a function of beta-gamma (bg)
  // this is done with a different function depending on the value of bg
  double f = 0;

  if (version == 0) {
    if (par[0] == 1)
      f = par[1] * std::pow(std::sqrt(x[0] * x[0] + 1), par[3]) / std::pow(x[0], par[3]) *
          (par[2] - par[5] * std::log(1 / x[0])) - par[4] + std::exp(par[6] + par[7] * x[0]);
    else if (par[0] == 2)
      f = par[1] * std::pow(x[0], 3) + par[2] * x[0] * x[0] + par[3] * x[0] + par[4];
    else if (par[0] == 3)
      f = -1.0 * par[1] * std::log(par[4] + std::pow(1 / x[0], par[2])) + par[3];
  }

  return f;
}

double CDCDedxMeanPred::getMean(double bg)
{
  // CDCDedxMeanPred M;
  m_meanpars = getMeanVector();

  // define the section of the mean to use
  double A = 0, B = 0, C = 0;
  if (bg < 4.5)
    A = 1;
  else if (bg < 10)
    B = 1;
  else
    C = 1;

  double x[1]; x[0] = bg;
  double parsA[9];
  double parsB[5];
  double parsC[5];

  parsA[0] = 1; parsB[0] = 2; parsC[0] = 3;
  for (int i = 0; i < 15; ++i) {
    if (i < 7) parsA[i + 1] = m_meanpars[i];
    else if (i < 11) parsB[i % 7 + 1] = m_meanpars[i];
    else parsC[i % 11 + 1] = m_meanpars[i];
  }

  // calculate dE/dx from the Bethe-Bloch mean
  double partA = meanCurve(x, parsA, 0);
  double partB = meanCurve(x, parsB, 0);
  double partC = meanCurve(x, parsC, 0);

  return (A * partA + B * partB + C * partC);
}

