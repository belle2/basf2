/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <fwcore/ModuleParamList.h>

using namespace std;
using namespace Belle2;
using namespace boost::python;


ModuleParamList::ModuleParamList()
{
  //Fill the map with the type identifiers and type information about the parameters.
  m_paramTypeInfoMap.insert(make_pair(typeid(int).name(), ParamTypeInfo(ParamTypeInfo::c_SingleInt, "Integer")));
  m_paramTypeInfoMap.insert(make_pair(typeid(double).name(), ParamTypeInfo(ParamTypeInfo::c_SingleDouble, "Double")));
  m_paramTypeInfoMap.insert(make_pair(typeid(string).name(), ParamTypeInfo(ParamTypeInfo::c_SingleString, "String")));
  m_paramTypeInfoMap.insert(make_pair(typeid(bool).name(), ParamTypeInfo(ParamTypeInfo::c_SingleBool, "Boolean")));
  m_paramTypeInfoMap.insert(make_pair(typeid(vector<int>).name(), ParamTypeInfo(ParamTypeInfo::c_ListInt, "List(Integer)")));
  m_paramTypeInfoMap.insert(make_pair(typeid(vector<double>).name(), ParamTypeInfo(ParamTypeInfo::c_ListDouble, "List(Double)")));
  m_paramTypeInfoMap.insert(make_pair(typeid(vector<string>).name(), ParamTypeInfo(ParamTypeInfo::c_ListString, "List(String)")));
  m_paramTypeInfoMap.insert(make_pair(typeid(vector<bool>).name(), ParamTypeInfo(ParamTypeInfo::c_ListBool, "List(Boolean)")));
}


ModuleParamList::~ModuleParamList()
{
  m_paramMap.clear();
  m_paramTypeInfoMap.clear();
}


ParamTypeInfo::ParamTypeInfo ModuleParamList::getParamTypeInfo(const std::string& name) const
{
  string typeName = getParamTypeString(name);
  ParamTypeInfo::ParamTypeInfo notSupportedType(ParamTypeInfo::c_NotSupported, "Not supported");

  if (!typeName.empty()) {
    map<string, ParamTypeInfo>::const_iterator mapIter = m_paramTypeInfoMap.find(typeName);

    if (mapIter != m_paramTypeInfoMap.end()) return mapIter->second;
    else return notSupportedType;
  } else return notSupportedType;
}


void ModuleParamList::setParamObject(const std::string& name, const boost::python::object& pyObj)
{
  ParamTypeInfo::ParamTypeInfo paramInfo = getParamTypeInfo(name);

  switch (paramInfo.m_paramType) {
    case ParamTypeInfo::c_SingleInt:
      setParamObjectTemplate<int>(name, pyObj);
      break;
    case ParamTypeInfo::c_SingleDouble:
      setParamObjectTemplate<double>(name, pyObj);
      break;
    case ParamTypeInfo::c_SingleString:
      setParamObjectTemplate<string>(name, pyObj);
      break;
    case ParamTypeInfo::c_SingleBool:
      setParamObjectTemplate<bool>(name, pyObj);
      break;
    default:
      ERROR("The parameter type of parameter '" + name + "' is not a supported single value type !")
  }
}


void ModuleParamList::setParamList(const std::string& name, const boost::python::list& pyList)
{
  ParamTypeInfo::ParamTypeInfo paramInfo = getParamTypeInfo(name);

  switch (paramInfo.m_paramType) {
    case ParamTypeInfo::c_ListInt:
      setParamListTemplate<int>(name, pyList);
      break;
    case ParamTypeInfo::c_ListDouble:
      setParamListTemplate<double>(name, pyList);
      break;
    case ParamTypeInfo::c_ListString:
      setParamListTemplate<string>(name, pyList);
      break;
    case ParamTypeInfo::c_ListBool:
      setParamListTemplate<bool>(name, pyList);
      break;
    default:
      ERROR("The parameter type of parameter '" + name + "' is not a supported list value type !")
  }
}


void ModuleParamList::getParamDefaultValues(const std::string& name, boost::python::list& outputList) const
{
  ParamTypeInfo::ParamTypeInfo paramInfo = getParamTypeInfo(name);

  switch (paramInfo.m_paramType) {
    case ParamTypeInfo::c_SingleInt:
      getParamDefaultValuesSingleTemplate<int>(name, outputList);
      break;
    case ParamTypeInfo::c_SingleDouble:
      getParamDefaultValuesSingleTemplate<double>(name, outputList);
      break;
    case ParamTypeInfo::c_SingleString:
      getParamDefaultValuesSingleTemplate<string>(name, outputList);
      break;
    case ParamTypeInfo::c_SingleBool:
      getParamDefaultValuesSingleTemplate<bool>(name, outputList);
      break;
    case ParamTypeInfo::c_ListInt:
      getParamDefaultValuesListTemplate< vector<int> >(name, outputList);
      break;
    case ParamTypeInfo::c_ListDouble:
      getParamDefaultValuesListTemplate< vector<double> >(name, outputList);
      break;
    case ParamTypeInfo::c_ListString:
      getParamDefaultValuesListTemplate< vector<string> >(name, outputList);
      break;
    case ParamTypeInfo::c_ListBool:
      getParamDefaultValuesListTemplate< vector<bool> >(name, outputList);
      break;
    default:
      ERROR("The parameter type of parameter '" + name + "' is not a supported single value type !")
  }
}


std::string ModuleParamList::getParamTypeString(const std::string& name) const
{
  //Check if a parameter with the given name exists
  std::map<std::string, ModuleParamPtr>::const_iterator mapIter;
  mapIter = m_paramMap.find(name);

  if (mapIter != m_paramMap.end()) {
    return mapIter->second.get()->getTypeInfo();
  } else {
    ERROR("Could not find the type of the parameter '" + name + "' !");
  }
  return string();
}


//=====================================================================
//                          Python API
//=====================================================================

boost::python::list ModuleParamList::getParamInfoListPython() const
{
  boost::python::list returnList;
  map<string, ModuleParamPtr>::const_iterator mapIter;

  for (mapIter = m_paramMap.begin(); mapIter != m_paramMap.end(); mapIter++) {
    ModuleParamInfoPython newParamInfo;
    ModuleParamPtr currParam = mapIter->second;

    newParamInfo.m_name = mapIter->first;
    newParamInfo.m_description = currParam->getDescription();
    newParamInfo.m_typeName = getParamTypeInfo(mapIter->first).m_readableName;
    getParamDefaultValues(mapIter->first, newParamInfo.m_defaultValues);

    returnList.append(boost::python::object(newParamInfo));
  }
  return returnList;
}


void ModuleParamList::exposePythonAPI()
{
  //Python class definition
  class_<ModuleParamInfoPython>("ModuleParamInfo")
  .def_readonly("name", &ModuleParamInfoPython::m_name)
  .def_readonly("type", &ModuleParamInfoPython::m_typeName)
  .def_readonly("default", &ModuleParamInfoPython::m_defaultValues)
  .def_readonly("description", &ModuleParamInfoPython::m_description)
  ;
}
