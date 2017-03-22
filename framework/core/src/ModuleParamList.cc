/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <boost/python/object.hpp>
#include <boost/python/list.hpp>

#include <framework/core/ModuleParamList.h>

#include <framework/core/ModuleParamInfoPython.h>
#include <algorithm>

using namespace std;
using namespace Belle2;
using namespace boost::python;


ModuleParamList::ModuleParamList()
{
}


ModuleParamList::~ModuleParamList()
{
  m_paramMap.clear();
}


std::vector<std::string> ModuleParamList::getUnsetForcedParams() const
{
  vector<string> missingParam;
  for (const pair<string, ModuleParamPtr>& mapEntry : m_paramMap) {
    if (mapEntry.second->isForcedInSteering() && !mapEntry.second->isSetInSteering())
      missingParam.push_back(mapEntry.first);
  }
  return missingParam;
}

boost::shared_ptr<boost::python::list> ModuleParamList::getParamInfoListPython() const
{
  boost::shared_ptr<boost::python::list> returnList(new boost::python::list);
  map<string, ModuleParamPtr>::const_iterator mapIter;

  for (mapIter = m_paramMap.begin(); mapIter != m_paramMap.end(); ++mapIter) {
    ModuleParamInfoPython newParamInfo;
    ModuleParamPtr currParam = mapIter->second;

    newParamInfo.m_name = mapIter->first;
    newParamInfo.m_description = currParam->getDescription();
    newParamInfo.m_typeName = currParam->getTypeInfo();
    newParamInfo.m_setInSteering = currParam->isSetInSteering();
    newParamInfo.m_forceInSteering = currParam->isForcedInSteering();
    getParamValuesPython(mapIter->first, newParamInfo.m_defaultValues, true);
    getParamValuesPython(mapIter->first, newParamInfo.m_values, false);

    returnList->append(boost::python::object(newParamInfo));
  }
  return returnList;
}

void ModuleParamList::setParameters(const ModuleParamList& params)
{
  for (const auto& param : params.m_paramMap) {
    auto& myParam = m_paramMap.at(param.first);
    myParam->setValueFromParam(*param.second.get());
  }
}


std::vector<std::string> ModuleParamList::getParameterNames() const
{
  std::vector<std::string> names;
  names.reserve(m_paramMap.size());
  for (const auto& nameAndParam : m_paramMap) {
    names.push_back(nameAndParam.first);
  }
  return names;
}

std::string ModuleParamList::getParameterDescription(const std::string& name) const
{
  return getParameterPtr(name)->getDescription();
}

std::string ModuleParamList::getParameterTypeInfo(const std::string& name) const
{
  return getParameterPtr(name)->getTypeInfo();
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


//==================================================================================
//                                Private methods
//==================================================================================

ModuleParamPtr ModuleParamList::getParameterPtr(const std::string& name) const
{
  //Check if a parameter with the given name exists
  std::map<std::string, ModuleParamPtr>::const_iterator mapIter;
  mapIter = m_paramMap.find(name);

  if (mapIter != m_paramMap.end()) {
    return mapIter->second;
  } else throw (ModuleParameterNotFoundError() << name);
}

std::string ModuleParamList::getParamTypeString(const std::string& name) const
{
  //Check if a parameter with the given name exists
  std::map<std::string, ModuleParamPtr>::const_iterator mapIter;
  mapIter = m_paramMap.find(name);

  if (mapIter != m_paramMap.end()) {
    return mapIter->second.get()->getTypeInfo();
  } else {
    B2FATAL("Module parameter '" + name + "' does not exist!");
  }
  return string();
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
