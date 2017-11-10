/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/core/ModuleParamList.templateDetails.h>

#include <framework/core/ModuleParamInfoPython.h>
#include <framework/core/FrameworkExceptions.h>

#include <boost/python/object.hpp>
#include <boost/python/list.hpp>

#include <algorithm>
#include <utility>
#include <memory>

using namespace Belle2;

/** Exception is thrown if the requested parameter could not be found. */
BELLE2_DEFINE_EXCEPTION(ModuleParameterNotFoundError,
                        "Could not find the parameter with the "
                        "name '%1%'! The value of the parameter "
                        "could NOT be set.");

/** Exception is thrown if the type of the requested parameter is different from the expected type. */
BELLE2_DEFINE_EXCEPTION(ModuleParameterTypeError,
                        "The type of the module parameter '%1%' "
                        "(%2%) is different from the type of the "
                        "value it should be set to (%3%)!");

void ModuleParamList::throwNotFoundError(const std::string& name)
{
  throw (ModuleParameterNotFoundError() << name);
}

void ModuleParamList::throwTypeError(const std::string& name,
                                     const std::string& expectedTypeInfo,
                                     const std::string& typeInfo)
{
  throw (ModuleParameterTypeError() << name << expectedTypeInfo << typeInfo);
}

ModuleParamList::ModuleParamList()
{
}


ModuleParamList::~ModuleParamList()
{
  m_paramMap.clear();
}


std::vector<std::string> ModuleParamList::getUnsetForcedParams() const
{
  std::vector<std::string> missingParam;
  for (const std::pair<std::string, ModuleParamPtr>& mapEntry : m_paramMap) {
    if (mapEntry.second->isForcedInSteering() && !mapEntry.second->isSetInSteering())
      missingParam.push_back(mapEntry.first);
  }
  return missingParam;
}

std::shared_ptr<boost::python::list> ModuleParamList::getParamInfoListPython() const
{
  std::shared_ptr<boost::python::list> returnList(new boost::python::list);
  std::map<std::string, ModuleParamPtr>::const_iterator mapIter;

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
  return std::string();
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
