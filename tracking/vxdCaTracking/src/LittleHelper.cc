/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/LittleHelper.h"
#include <framework/logging/Logger.h>

#include <TRandom.h>

using namespace std;
using namespace Belle2;

void LittleHelper::resetValues(double mean, double sigma)
{
  m_meanValue = mean;
  m_sigmaValue = sigma;
}


bool LittleHelper::checkSanity(double low, double high, double mean, double sigma)
{
  if ((mean + 2 * sigma) < low or(mean - 2 * sigma) > high) {
    B2ERROR("checkSanity failed (LittleHelper), low: " << low << ", high: " << high << ", mean: " << mean << ", sigma: " << sigma);
    return false;
  } // mean is safe to use
  return true; // mean is not safe to use (danger of near infinite loop)
}



double LittleHelper::smearValueGauss(double low, double high, double mean, double sigma) /// double mean = m_meanValue, double sigma = m_sigmaValue
{
  if (sigma < 0) { sigma = -sigma; }
  bool isSane = checkSanity(low, high, mean, sigma);
  if (isSane == false) { B2ERROR("smearing value failed (smearValueGauss), returning initial value!"); return mean; }
  double rngValue = low - 1;
  while (rngValue < low or rngValue > high) {
    rngValue = gRandom->Gaus(mean, sigma);
  }
  return rngValue;
}
double LittleHelper::smearValueGauss(double low, double high) { return smearValueGauss(low, high, m_meanValue, m_sigmaValue); }



double LittleHelper::smearPositiveGauss(double high, double mean, double sigma)
{
  return smearValueGauss(0, high, mean, sigma);
}
double LittleHelper::smearPositiveGauss(double high) {  return smearValueGauss(0, high, m_meanValue, m_sigmaValue); }


double LittleHelper::smearNormalizedGauss(double mean, double sigma)
{
  return smearValueGauss(0, 1, mean, sigma);
}
double LittleHelper::smearNormalizedGauss(double mean) { return smearValueGauss(0, 1, mean, m_sigmaValue); }
double LittleHelper::smearNormalizedGauss() { return smearValueGauss(0, 1, m_meanValue, m_sigmaValue); }


double LittleHelper::smearValueUniform(double low, double high, double mean, double sigma)
{
  if (sigma < 0) { sigma = -sigma; }
  bool isSane = checkSanity(low, high, mean, sigma);
  if (isSane == false) { B2ERROR("smearing value failed (smearValueUniform), returning initial value!"); return mean; }
  double rngValue = low - 1;
  while (rngValue < low or rngValue > high) {
    rngValue = gRandom->Uniform(mean - sigma, mean + sigma);
  }
  return rngValue;
}
double LittleHelper::smearValueUniform(double low, double high) { return smearValueUniform(low, high, m_meanValue, m_sigmaValue); }


double LittleHelper::smearPositiveUniform(double high, double mean, double sigma)
{
  return smearValueUniform(0, high, mean, sigma);
}
double LittleHelper::smearPositiveUniform(double high) { return smearValueUniform(0, high, m_meanValue, m_sigmaValue); }


double LittleHelper::smearNormalizedUniform(double mean, double sigma)
{
  return smearValueUniform(0, 1, mean, sigma);
}
double LittleHelper::smearNormalizedUniform() { return smearValueUniform(0, 1, m_meanValue, m_sigmaValue);  }
