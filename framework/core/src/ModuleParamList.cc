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
#include <boost/python/extract.hpp>

#include <framework/core/ModuleParamList.h>

#include <framework/core/ModuleParamInfoPython.h>
#include <framework/core/PyObjConvUtils.h>

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


ParamTypeInfo ModuleParamList::getParamTypeInfo(const std::string& name) const
{
  string typeName = getParamTypeString(name);
  ParamTypeInfo notSupportedType(ParamTypeInfo::c_NotSupportedPBT, ParamTypeInfo::c_NotSupportedPVT, "Not supported");

  if (!typeName.empty()) {
    map<string, ParamTypeInfo>::const_iterator mapIter = m_paramTypeInfoMap.find(typeName);

    if (mapIter != m_paramTypeInfoMap.end()) return mapIter->second;
  }
  return notSupportedType;
}


bool ModuleParamList::hasUnsetForcedParams() const
{
  map<string, ModuleParamPtr>::const_iterator mapIter;

  for (mapIter = m_paramMap.begin(); mapIter != m_paramMap.end(); ++mapIter) {
    if ((mapIter->second->isForcedInSteering()) && (!mapIter->second->isSetInSteering())) return true;
  }

  return false;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                   Python API
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template<typename T>
void setParamObjectTemplatePython(ModuleParamList& params, const std::string& name, const boost::python::object& pyObj)
{
  boost::python::extract<T> valueProxy(pyObj);
  if (valueProxy.check()) {
    T tmpValue = static_cast<T>(valueProxy);
    params.setParameter(name, tmpValue);
  } else {
    B2ERROR("Could not set a module parameter: The python object defined by '" + name + "' could not be converted!")
  }
}


template<typename T>
void setParamListTemplatePython(ModuleParamList& params, const std::string& name, const boost::python::list& pyList)
{
  std::vector<T> tmpList;
  int nList = boost::python::len(pyList);

  for (int iList = 0; iList < nList; ++iList) {
    boost::python::extract<T> checkValue(pyList[iList]);
    if (checkValue.check()) {
      tmpList.push_back(checkValue);
    } else {
      B2ERROR("Could not set a module parameter: A python object defined in the list '" + name + "' could not be converted!")
    }
  }
  params.setParameter(name, tmpList);
}


template<typename T>
void getParamObjectValuesTemplatePython(const ModuleParamList& params, const std::string& name, boost::python::list& outputList, bool defaultValues)
{
  try {
    ModuleParam<T>& explModParam = params.getParameter<T>(name);
    ParamTypeInfo paramInfo = params.getParamTypeInfo(name);

    if (paramInfo.m_paramBasicType != ParamTypeInfo::c_SingleParam) {
      B2ERROR("The parameter type of parameter '" + name + "' is not a single parameter value!")
      return;
    }

    if (defaultValues) {
      PyObjConvUtils::addSingleValueToList<T>(explModParam.getDefaultValue(), outputList);
    } else {
      PyObjConvUtils::addSingleValueToList<T>(explModParam.getValue(), outputList);
    }
  } catch (ModuleParamList::ModuleParameterNotFoundError& exc) {
    B2ERROR(exc.what())
  } catch (ModuleParamList::ModuleParameterTypeError& exc) {
    B2ERROR(exc.what())
  }
}


template<typename T>
void getParamListValuesTemplatePython(const ModuleParamList& params, const std::string& name, boost::python::list& outputList, bool defaultValues)
{
  try {
    ModuleParam<T>& explModParam = params.getParameter<T>(name);
    ParamTypeInfo paramInfo = params.getParamTypeInfo(name);

    if (paramInfo.m_paramBasicType != ParamTypeInfo::c_ListParam) {
      B2ERROR("The parameter type of parameter '" + name + "' is not a list parameter value!")
      return;
    }

    if (defaultValues) {
      PyObjConvUtils::addSTLVectorToList<T>(explModParam.getDefaultValue(), outputList);
    } else {
      PyObjConvUtils::addSTLVectorToList<T>(explModParam.getValue(), outputList);
    }
  } catch (ModuleParamList::ModuleParameterNotFoundError& exc) {
    B2ERROR(exc.what())
  } catch (ModuleParamList::ModuleParameterTypeError& exc) {
    B2ERROR(exc.what())
  }
}
boost::python::list* ModuleParamList::getParamInfoListPython() const
{
  boost::python::list* returnList = new boost::python::list;
  map<string, ModuleParamPtr>::const_iterator mapIter;

  for (mapIter = m_paramMap.begin(); mapIter != m_paramMap.end(); ++mapIter) {
    ModuleParamInfoPython newParamInfo;
    ModuleParamPtr currParam = mapIter->second;

    newParamInfo.m_name = mapIter->first;
    newParamInfo.m_description = currParam->getDescription();
    newParamInfo.m_typeName = getParamTypeInfo(mapIter->first).m_readableName;
    newParamInfo.m_setInSteering = currParam->isSetInSteering();
    newParamInfo.m_forceInSteering = currParam->isForcedInSteering();
    getParamValuesPython(mapIter->first, newParamInfo.m_defaultValues, true);
    getParamValuesPython(mapIter->first, newParamInfo.m_values, false);

    returnList->append(boost::python::object(newParamInfo));
  }
  return returnList;
}


void ModuleParamList::setParamObjectPython(const std::string& name, const boost::python::object& pyObj)
{
  ParamTypeInfo paramInfo = getParamTypeInfo(name);

  if (paramInfo.m_paramBasicType != ParamTypeInfo::c_SingleParam) {
    B2ERROR("The parameter type of parameter '" + name + "' is not a single parameter value!")
    return;
  }

  switch (paramInfo.m_paramValueType) {
    case ParamTypeInfo::c_IntegerParam:
      setParamObjectTemplatePython<int>(*this, name, pyObj);
      break;
    case ParamTypeInfo::c_DoubleParam:
      setParamObjectTemplatePython<double>(*this, name, pyObj);
      break;
    case ParamTypeInfo::c_StringParam:
      setParamObjectTemplatePython<string>(*this, name, pyObj);
      break;
    case ParamTypeInfo::c_BoolParam:
      setParamObjectTemplatePython<bool>(*this, name, pyObj);
      break;
    default:
      B2ERROR("The parameter type of parameter '" + name + "' is not a supported single value type!")
  }
}


void ModuleParamList::setParamListPython(const std::string& name, const boost::python::list& pyList)
{
  ParamTypeInfo paramInfo = getParamTypeInfo(name);

  if (paramInfo.m_paramBasicType != ParamTypeInfo::c_ListParam) {
    B2ERROR("The parameter type of parameter '" + name + "' is not a list parameter value!")
    return;
  }

  switch (paramInfo.m_paramValueType) {
    case ParamTypeInfo::c_IntegerParam:
      setParamListTemplatePython<int>(*this, name, pyList);
      break;
    case ParamTypeInfo::c_DoubleParam:
      setParamListTemplatePython<double>(*this, name, pyList);
      break;
    case ParamTypeInfo::c_StringParam:
      setParamListTemplatePython<string>(*this, name, pyList);
      break;
    case ParamTypeInfo::c_BoolParam:
      setParamListTemplatePython<bool>(*this, name, pyList);
      break;
    default:
      B2ERROR("The parameter type of parameter '" + name + "' is not a supported list value type!")
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
    B2FATAL("Module parameter '" + name + "' does not exist!");
  }
  return string();
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                   Python API
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void ModuleParamList::getParamValuesPython(const std::string& name, boost::python::list& outputList, bool defaultValues) const
{
  ParamTypeInfo paramInfo = getParamTypeInfo(name);

  switch (paramInfo.m_paramBasicType) {
    case ParamTypeInfo::c_SingleParam:
      switch (paramInfo.m_paramValueType) {
        case ParamTypeInfo::c_IntegerParam:
          getParamObjectValuesTemplatePython<int>(*this, name, outputList, defaultValues);
          break;
        case ParamTypeInfo::c_DoubleParam:
          getParamObjectValuesTemplatePython<double>(*this, name, outputList, defaultValues);
          break;
        case ParamTypeInfo::c_StringParam:
          getParamObjectValuesTemplatePython<string>(*this, name, outputList, defaultValues);
          break;
        case ParamTypeInfo::c_BoolParam:
          getParamObjectValuesTemplatePython<bool>(*this, name, outputList, defaultValues);
          break;
        default:
          B2ERROR("The parameter type of parameter '" + name + "' is not a supported parameter value type!")
      }
      break;
    case ParamTypeInfo::c_ListParam:
      switch (paramInfo.m_paramValueType) {
        case ParamTypeInfo::c_IntegerParam:
          getParamListValuesTemplatePython< vector<int> >(*this, name, outputList, defaultValues);
          break;
        case ParamTypeInfo::c_DoubleParam:
          getParamListValuesTemplatePython< vector<double> >(*this, name, outputList, defaultValues);
          break;
        case ParamTypeInfo::c_StringParam:
          getParamListValuesTemplatePython< vector<string> >(*this, name, outputList, defaultValues);
          break;
        case ParamTypeInfo::c_BoolParam:
          getParamListValuesTemplatePython< vector<bool> >(*this, name, outputList, defaultValues);
          break;
        default:
          B2ERROR("The parameter type of parameter '" + name + "' is not a supported parameter value type!")
      }
      break;
    default:
      B2ERROR("The parameter type of parameter '" + name + "' is not a supported basic parameter type!")
  }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
