/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/utility/CDCDedxSigmaPred.h>

using namespace Belle2;

double CDCDedxSigmaPred::sigmaCurve(double* x, const double* par, int version) const
{
  // calculate the predicted mean value as a function of beta-gamma (bg)
  // this is done with a different function depending dE/dx, nhit, and cos(theta)
  double f = 0;

  if (version == 0) {
    if (par[0] == 1) { // return dedx parameterization
      f = par[1] + par[2] * x[0];
    } else if (par[0] == 2) { // return nhit or sin(theta) parameterization
      f = par[1] * std::pow(x[0], 4) + par[2] * std::pow(x[0], 3) +
          par[3] * x[0] * x[0] + par[4] * x[0] + par[5];
    } else if (par[0] == 3) { // return cos(theta) parameterization
      f = par[1] * exp(-0.5 * pow(((x[0] - par[2]) / par[3]), 2)) +
          par[4] * pow(x[0], 6) + par[5] * pow(x[0], 5) + par[6] * pow(x[0], 4) +
          par[7] * pow(x[0], 3) + par[8] * x[0] * x[0] + par[9] * x[0] + par[10];
    }
  }

  return f;
}

double CDCDedxSigmaPred::getSigma(double dedx, double nhit, double cos, double timereso)
{
  double correction  = cosPrediction(cos) * nhitPrediction(nhit) * ionzPrediction(dedx) * timereso;
  return correction;
}


double CDCDedxSigmaPred::nhitPrediction(double nhit)
{

  m_sigmapars = getSigmaVector();
  double x[1];
  double nhitpar[6];

  nhitpar[0] = 2;
  for (int i = 0; i < 5; ++i) {
    nhitpar[i + 1] = m_sigmapars[i + 2];
  }

  // determine sigma from the nhit parameterization
  x[0] = nhit;

  double corNHit;
  int nhit_min = 8, nhit_max = 37;

  if (nhit <  nhit_min) {
    x[0] = nhit_min;
    corNHit = sigmaCurve(x, nhitpar, 0) * sqrt(nhit_min / nhit);
  } else if (nhit > nhit_max) {
    x[0] = nhit_max;
    corNHit = sigmaCurve(x, nhitpar, 0) * sqrt(nhit_max / nhit);
  } else corNHit = sigmaCurve(x, nhitpar, 0);


  return corNHit;
}

double CDCDedxSigmaPred::ionzPrediction(double dedx)
{

  m_sigmapars = getSigmaVector();
  double x[1];
  double dedxpar[3];

  dedxpar[0] = 1;

  for (int i = 0; i < 2; ++i) {
    dedxpar[i + 1] = m_sigmapars[i];

  }
  // determine sigma from the parameterization
  x[0] = dedx;
  double corDedx = sigmaCurve(x, dedxpar, 0);

  return corDedx;
}

double CDCDedxSigmaPred::cosPrediction(double cos)
{

  m_sigmapars = getSigmaVector();
  double x[1];
  double corCos;

  if (m_sigmapars.size() == 17) {
    double cospar[11];
    cospar[0] = 3;
    for (int i = 0; i < 10; ++i) {
      cospar[i + 1] = m_sigmapars[i + 7];
    }
    x[0] = cos;
    corCos = sigmaCurve(x, cospar, 0);
  }

  else {
    double sinpar[6];

    sinpar[0] = 2;
    for (int i = 0; i < 5; ++i) {
      sinpar[i + 1] = m_sigmapars[i + 7];
    }

    double sin = sqrt(1 - cos * cos);
    if (sin > 0.99) sin = 0.99;
    x[0] = sin;

    corCos = sigmaCurve(x, sinpar, 0);
  }

  return corCos;
}
