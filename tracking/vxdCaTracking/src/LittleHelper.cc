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
#include <math.h>       /* floor */
#include <TRandom.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Tracking;


bool LittleHelper::checkSanity(double low, double high, double mean, double sigma)
{
  if ((mean + 2 * sigma) < low or (mean - 2 * sigma) > high) {
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


int LittleHelper::getRandomIntegerUniform(int low, int high)
{
  return floor(gRandom->Uniform(low, high));
}

int LittleHelper::getRandomIntegerGauss(int mean, int sigma)
{
  return floor(gRandom->Gaus(mean, sigma));
}