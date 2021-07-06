/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/dataobjects/ContinuumSuppression.h>

using namespace Belle2;

void ContinuumSuppression::addThrustB(const TVector3& thrustB)
{
  m_thrustB = thrustB;
}

void ContinuumSuppression::addThrustO(const TVector3& thrustO)
{
  m_thrustO = thrustO;
}

void ContinuumSuppression::addThrustBm(float thrustBm)
{
  m_thrustBm = thrustBm;
}

void ContinuumSuppression::addThrustOm(float thrustOm)
{
  m_thrustOm = thrustOm;
}

void ContinuumSuppression::addCosTBTO(float cosTBTO)
{
  m_cosTBTO = cosTBTO;
}

void ContinuumSuppression::addCosTBz(float cosTBz)
{
  m_cosTBz = cosTBz;
}

void ContinuumSuppression::addR2(float R2)
{
  m_R2 = R2;
}

void ContinuumSuppression::addKsfwFS0(const std::vector<float>& KsfwFS0)
{
  m_ksfwFS0 = KsfwFS0;
}

void ContinuumSuppression::addKsfwFS1(const std::vector<float>& KsfwFS1)
{
  m_ksfwFS1 = KsfwFS1;
}

void ContinuumSuppression::addCleoConesALL(const std::vector<float>& CleoConesALL)
{
  m_cleoConesALL = CleoConesALL;
}

void ContinuumSuppression::addCleoConesROE(const std::vector<float>& CleoConesROE)
{
  m_cleoConesROE = CleoConesROE;
}
