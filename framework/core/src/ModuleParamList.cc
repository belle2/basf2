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


bool ModuleParamList::hasUnsetForcedParams() const
{
  map<string, ModuleParamPtr>::const_iterator mapIter;

  for (mapIter = m_paramMap.begin(); mapIter != m_paramMap.end(); ++mapIter) {
    if ((mapIter->second->isForcedInSteering()) && (!mapIter->second->isSetInSteering())) return true;
  }

  return false;
}

boost::python::list* ModuleParamList::getParamInfoListPython() const
{
  // TODO Is this a memory leak?
  boost::python::list* returnList = new boost::python::list;
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


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


//==================================================================================
//                                Private methods
//==================================================================================

ModuleParamPtr ModuleParamList::getParameterPtr(const std::string& name) const throw(ModuleParameterNotFoundError)
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
