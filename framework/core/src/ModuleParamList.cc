/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/ModuleParamList.h>

using namespace std;
using namespace Belle2;
using namespace boost::python;


ModuleParamList::ModuleParamList()
{
  //Fill the map with the type identifiers and type information about the parameters.
  m_paramTypeInfoMap.insert(make_pair(typeid(int).name(), ParamTypeInfo(ParamTypeInfo::c_SingleParam, ParamTypeInfo::c_IntegerParam, "Integer")));
  m_paramTypeInfoMap.insert(make_pair(typeid(double).name(), ParamTypeInfo(ParamTypeInfo::c_SingleParam, ParamTypeInfo::c_DoubleParam, "Double")));
  m_paramTypeInfoMap.insert(make_pair(typeid(string).name(), ParamTypeInfo(ParamTypeInfo::c_SingleParam, ParamTypeInfo::c_StringParam, "String")));
  m_paramTypeInfoMap.insert(make_pair(typeid(bool).name(), ParamTypeInfo(ParamTypeInfo::c_SingleParam, ParamTypeInfo::c_BoolParam, "Boolean")));
  m_paramTypeInfoMap.insert(make_pair(typeid(vector<int>).name(), ParamTypeInfo(ParamTypeInfo::c_ListParam, ParamTypeInfo::c_IntegerParam, "List(Integer)")));
  m_paramTypeInfoMap.insert(make_pair(typeid(vector<double>).name(), ParamTypeInfo(ParamTypeInfo::c_ListParam, ParamTypeInfo::c_DoubleParam, "List(Double)")));
  m_paramTypeInfoMap.insert(make_pair(typeid(vector<string>).name(), ParamTypeInfo(ParamTypeInfo::c_ListParam, ParamTypeInfo::c_StringParam, "List(String)")));
  m_paramTypeInfoMap.insert(make_pair(typeid(vector<bool>).name(), ParamTypeInfo(ParamTypeInfo::c_ListParam, ParamTypeInfo::c_BoolParam, "List(Boolean)")));
}


ModuleParamList::~ModuleParamList()
{
  m_paramMap.clear();
  m_paramTypeInfoMap.clear();
}


ParamTypeInfo::ParamTypeInfo ModuleParamList::getParamTypeInfo(const std::string& name) const
{
  string typeName = getParamTypeString(name);
  ParamTypeInfo::ParamTypeInfo notSupportedType(ParamTypeInfo::c_NotSupportedPBT, ParamTypeInfo::c_NotSupportedPVT, "Not supported");

  if (!typeName.empty()) {
    map<string, ParamTypeInfo>::const_iterator mapIter = m_paramTypeInfoMap.find(typeName);

    if (mapIter != m_paramTypeInfoMap.end()) return mapIter->second;
    else return notSupportedType;
  } else return notSupportedType;
}


bool ModuleParamList::hasUnsetForcedParams() const
{
  map<string, ModuleParamPtr>::const_iterator mapIter;

  for (mapIter = m_paramMap.begin(); mapIter != m_paramMap.end(); mapIter++) {
    if ((mapIter->second->isForcedInSteering()) && (!mapIter->second->isSetInSteering())) return true;
  }

  return false;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                   Python API
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
    newParamInfo.m_setInSteering = currParam->isSetInSteering();
    newParamInfo.m_forceInSteering = currParam->isForcedInSteering();
    getParamValuesPython(mapIter->first, newParamInfo.m_defaultValues, true);
    getParamValuesPython(mapIter->first, newParamInfo.m_values, false);

    returnList.append(boost::python::object(newParamInfo));
  }
  return returnList;
}


void ModuleParamList::setParamObjectPython(const std::string& name, const boost::python::object& pyObj)
{
  ParamTypeInfo::ParamTypeInfo paramInfo = getParamTypeInfo(name);

  if (paramInfo.m_paramBasicType != ParamTypeInfo::c_SingleParam) {
    ERROR("The parameter type of parameter '" + name + "' is not a single parameter value !")
    return;
  }

  switch (paramInfo.m_paramValueType) {
    case ParamTypeInfo::c_IntegerParam:
      setParamObjectTemplatePython<int>(name, pyObj);
      break;
    case ParamTypeInfo::c_DoubleParam:
      setParamObjectTemplatePython<double>(name, pyObj);
      break;
    case ParamTypeInfo::c_StringParam:
      setParamObjectTemplatePython<string>(name, pyObj);
      break;
    case ParamTypeInfo::c_BoolParam:
      setParamObjectTemplatePython<bool>(name, pyObj);
      break;
    default:
      ERROR("The parameter type of parameter '" + name + "' is not a supported single value type !")
  }
}


void ModuleParamList::setParamListPython(const std::string& name, const boost::python::list& pyList)
{
  ParamTypeInfo::ParamTypeInfo paramInfo = getParamTypeInfo(name);

  if (paramInfo.m_paramBasicType != ParamTypeInfo::c_ListParam) {
    ERROR("The parameter type of parameter '" + name + "' is not a list parameter value !")
    return;
  }

  switch (paramInfo.m_paramValueType) {
    case ParamTypeInfo::c_IntegerParam:
      setParamListTemplatePython<int>(name, pyList);
      break;
    case ParamTypeInfo::c_DoubleParam:
      setParamListTemplatePython<double>(name, pyList);
      break;
    case ParamTypeInfo::c_StringParam:
      setParamListTemplatePython<string>(name, pyList);
      break;
    case ParamTypeInfo::c_BoolParam:
      setParamListTemplatePython<bool>(name, pyList);
      break;
    default:
      ERROR("The parameter type of parameter '" + name + "' is not a supported list value type !")
  }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


//==================================================================================
//                                Private methods
//==================================================================================

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


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                   Python API
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void ModuleParamList::getParamValuesPython(const std::string& name, boost::python::list& outputList, bool defaultValues) const
{
  ParamTypeInfo::ParamTypeInfo paramInfo = getParamTypeInfo(name);

  switch (paramInfo.m_paramBasicType) {
    case ParamTypeInfo::c_SingleParam:
      switch (paramInfo.m_paramValueType) {
        case ParamTypeInfo::c_IntegerParam:
          getParamObjectValuesTemplatePython<int>(name, outputList, defaultValues);
          break;
        case ParamTypeInfo::c_DoubleParam:
          getParamObjectValuesTemplatePython<double>(name, outputList, defaultValues);
          break;
        case ParamTypeInfo::c_StringParam:
          getParamObjectValuesTemplatePython<string>(name, outputList, defaultValues);
          break;
        case ParamTypeInfo::c_BoolParam:
          getParamObjectValuesTemplatePython<bool>(name, outputList, defaultValues);
          break;
        default:
          ERROR("The parameter type of parameter '" + name + "' is not a supported parameter value type !")
      }
      break;
    case ParamTypeInfo::c_ListParam:
      switch (paramInfo.m_paramValueType) {
        case ParamTypeInfo::c_IntegerParam:
          getParamListValuesTemplatePython< vector<int> >(name, outputList, defaultValues);
          break;
        case ParamTypeInfo::c_DoubleParam:
          getParamListValuesTemplatePython< vector<double> >(name, outputList, defaultValues);
          break;
        case ParamTypeInfo::c_StringParam:
          getParamListValuesTemplatePython< vector<string> >(name, outputList, defaultValues);
          break;
        case ParamTypeInfo::c_BoolParam:
          getParamListValuesTemplatePython< vector<bool> >(name, outputList, defaultValues);
          break;
        default:
          ERROR("The parameter type of parameter '" + name + "' is not a supported parameter value type !")
      }
      break;
    default:
      ERROR("The parameter type of parameter '" + name + "' is not a supported basic parameter type !")
  }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
