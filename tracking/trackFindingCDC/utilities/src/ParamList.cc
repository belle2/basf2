/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/utilities/ParamList.h>
#include <tracking/trackFindingCDC/utilities/ParamList.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

ParamList::ParamList() = default;
ParamList::~ParamList() = default;

bool
ParamList::hasParameter(const std::string& name) const
{
  auto itParam = m_paramsByName.find(name);
  return itParam != m_paramsByName.end();
}

std::vector<std::string>
ParamList::getParameterNames() const
{
  std::vector<std::string> result;
  for (const auto& paramAndName : m_paramsByName) {
    result.push_back(paramAndName.first);
  }
  return result;
}

std::string
ParamList::getParameterDescription(const std::string& name) const
{
  auto itParam = m_paramsByName.find(name);
  if (itParam == m_paramsByName.end()) return "(unknown parameter name '" + name + "')";
  else return itParam->second->getDescription();
}

void
ParamList::transferTo(ModuleParamList* moduleParamList)
{
  for (const auto& paramAndName : m_paramsByName) {
    const std::unique_ptr<ParamBase>& paramBase = paramAndName.second;
    paramBase->transferTo(moduleParamList);
  }
}
