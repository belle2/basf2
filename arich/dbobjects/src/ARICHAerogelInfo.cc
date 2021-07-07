/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arich/dbobjects/ARICHAerogelInfo.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;

float ARICHAerogelInfo::getAerogelTransmittanceElement(int lambda) const
{
  if (lambda < c_lambdaMin || lambda > c_lambdaMax) {
    B2INFO("lambda not in right interval!");
    return -1;
  }
  int index = (int) round((lambda - c_lambdaMin) / c_lambdaStep);
  return m_transmittance[index];
}


void ARICHAerogelInfo::setAerogelTransmittanceElement(int lambda, float transmittance)
{
  if (lambda < c_lambdaMin || lambda > c_lambdaMax) {
    B2INFO("lambda not in right interval!");
  }
  int index = (int) round((lambda - c_lambdaMin) / c_lambdaStep);
  m_transmittance[index] = transmittance;
}


void ARICHAerogelInfo::setAerogelTransmittance(std::vector<float> transmittance)
{
  for (unsigned int i = 0; i < transmittance.size(); i++)  m_transmittance.push_back(transmittance[i]);
}


void ARICHAerogelInfo::setAerogelTransmittanceLambda(std::vector<float> lambda)
{
  for (unsigned int i = 0; i < lambda.size(); i++)  m_lambda.push_back(lambda[i]);
}


