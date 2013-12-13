/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dataobjects/ParticleInfo.h>

#include <framework/logging/Logger.h>

using namespace Belle2;

ClassImp(ParticleInfo)

ParticleInfo::ParticleInfo()
{

}

bool ParticleInfo::addVariable(const std::string name, const float value)
{
  auto ret = m_variables.insert(std::pair<std::string, float>(name, value));
  return ret.second;
}

bool ParticleInfo::isAvailable(const std::string name) const
{
  return m_variables.find(name) != m_variables.end();
}

float ParticleInfo::getValue(const std::string name) const
{
  auto it = m_variables.find(name);
  if (it == m_variables.end()) {
    B2WARNING("ParticleInfo: Given Name " << name << " doesn't exist in ParticleInfo Map. 0 is returned as default. Be aware!");
    return 0;
  }
  return it->second;
}
