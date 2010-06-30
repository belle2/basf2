/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MODULEPARAMS_H_
#define MODULEPARAMS_H_

#include <boost/python.hpp>
#include <boost/python/list.hpp>

#include <framework/fwcore/FwExceptions.h>
#include <framework/fwcore/ModuleParam.h>
#include <framework/fwcore/PyObjConvUtils.h>
#include <framework/logging/Logger.h>

#include <map>
#include <vector>
#include <string>


namespace Belle2 {

  //! The parameter type info class
  /*! Stores information about each supported parameter type.
  */
  class ParamTypeInfo {
  public:

    //! supported parameter types
    enum EParamType {
      c_SingleInt,    /*!< A single integer parameter value */
      c_SingleDouble, /*!< A single double parameter value */
      c_SingleString, /*!< A single string parameter value */
      c_SingleBool,   /*!< A single boolean parameter value */
      c_ListInt,      /*!< A list of integer parameter values */
      c_ListDouble,   /*!< A list of double parameter values */
      c_ListString,   /*!< A list of string parameter values */
      c_ListBool,     /*!< A list of boolean parameter values */
      c_NotSupported  /*!< Not supported parameter type */
    };

    EParamType  m_paramType;
    std::string m_readableName;

    ParamTypeInfo(EParamType paramType, std::string readableName) :
        m_paramType(paramType), m_readableName(readableName) {};
  };


  //! The Module parameter list class
  /*! Stores and manages all parameters of a module.
  */
  class ModuleParamList {

  public:

    //! Constructor
    ModuleParamList();

    //! Destructor
    ~ModuleParamList();

    //! Adds a new parameter to the module list.
    /*!
        A parameter consists of a reference pointing to a member variable in the module
        which stores and allows fast access to the parameter value. In addition the type
        of the parameter is saved and optionally a description can be given.

        \param name The unique name of the parameter.
        \param paramVariable Reference to the variable which stores the parameter value.
        \param defaultValue The default value of the parameter.
        \param description An optional description of the parameter.
    */
    template<typename T>
    void addParameter(const std::string& name, T& paramVariable, const T& defaultValue, const std::string& description = "");

    //! Sets the value of a parameter given by its name.
    /*!
        A template based method which is invoked by the module.

        \param name The unique name of the parameter.
        \param value The parameter value which should be assigned to the parameter given by its name.
    */
    template<typename T>
    void setParameter(const std::string& name, T& value);

    //! Returns the parameter type information of the given parameter.
    /*!
        \param name The unique name of the parameter.
        \return The parameter type information.
    */
    ParamTypeInfo::ParamTypeInfo getParamTypeInfo(const std::string& name) const;

    //! Implements a method for setting boost::python objects.
    /*! The method supports the following types: int, double, string, bool
        The conversion of the python object to the C++ type and the final storage of the
        parameter value is done by specializing the template method setParamObjectTemplate().

        \param name The unique name of the parameter.
        \param pyObj The object which should be converted and stored as the parameter value.
    */
    void setParamObject(const std::string& name, const boost::python::object& pyObj);

    //! Implements a template based method for setting boost::python objects.
    /*! The method converts a boost::python object to a C++ type and stores the result as the
        parameter value.

        \param name The unique name of the parameter.
        \param pyObj The object which should be converted and stored as the parameter value.
    */
    template<typename T>
    void setParamObjectTemplate(const std::string& name, const boost::python::object& pyObj);

    //! Implements a method for setting boost::python lists.
    /*! The method supports lists of the following types: int, double, string, bool
        The conversion of the python list to the std::vector and the final storage of the
        parameter value is done by specializing the template method setParamListTemplate().

        \param name The unique name of the parameter.
        \param pyList The list which should be converted to a std::vector and stored as the parameter value.
    */
    void setParamList(const std::string& name, const boost::python::list& pyList);

    //! Implements a template based method for setting boost::python lists.
    /*! The method converts a boost::python list to a std::vector and stores the result as the
        parameter value.

        \param name The unique name of the parameter.
        \param pyList The list which should be converted to a std::vector and stored as the parameter value.
    */
    template<typename T>
    void setParamListTemplate(const std::string& name, const boost::python::list& pyList);

    //! Returns a python list containing the default values of the given parameter.
    /*!
        Calls according to the parameter type a specialized version of the template method getParamDefaultValuesTemplate().
        \param name The unique name of the parameter.
        \param Reference to the output list containing the parameter default values. For single value parameter, the list only consists of one element.
    */
    void getParamDefaultValues(const std::string& name, boost::python::list& outputList) const;

    //! Returns a python list containing the default values of the given single value parameter (template method).
    /*!
        \param name The unique name of the parameter.
        \param outputList The python list containing the single parameter default value.
    */
    template<typename T>
    void getParamDefaultValuesSingleTemplate(const std::string& name, boost::python::list& outputList) const;

    //! Returns a python list containing the default values of the given list value parameter (template method).
    /*!
        \param name The unique name of the parameter.
        \param outputList The python list containing the list parameter default values.
    */
    template<typename T>
    void getParamDefaultValuesListTemplate(const std::string& name, boost::python::list& outputList) const;

    //! Returns the type identifier of the parameter as string.
    /*!
        The type identifier is used to discriminate between problematic parameter types (e.g. double and int)

        \param name The unique name of the parameter.
        \return The type identifier as string. Returns an empty string if a parameter with the given name could not be found.
    */
    std::string getParamTypeString(const std::string& name) const;


    //--------------------------------------------------
    //                   Python API
    //--------------------------------------------------

    //! Returns a python list of all parameters.
    /*!
        Each item in the list consists of the name of the parameter, a string describing its type,
        a python list of all default values and the description of the parameter.
        \return A python list containing the parameters of this parameter list.
    */
    boost::python::list getParamInfoListPython() const;

    //! Exposes methods of the Module class to Python.
    static void exposePythonAPI();


  protected:


  private:

    std::map<std::string, ModuleParamPtr> m_paramMap;  /*!< Stores the module parameters together with a string name as key. */
    std::map<std::string, ParamTypeInfo>  m_paramTypeInfoMap; /*!< Map which stores the typeid name as key and information about the parameter as value. */

    //! Returns a reference to a module parameter. The returned parameter has already the correct type.
    /*!
       Throws an exception of type FwExcModuleParameterNotFound if a parameter with the given name does not exist.
       Throws an exception of type FwExcModuleParameterType if the parameter type of does not match to the template parameter.

        \param name The unique name of the parameter.
        \return A reference to a module parameter having the correct type.
    */
    template<typename T>
    ModuleParam<T>& getModuleParameter(const std::string& name) const throw(FwExcModuleParameterNotFound, FwExcModuleParameterType);
  };


  //------------------------------------------------------
  //       Implementation of template based methods
  //------------------------------------------------------

  template<typename T>
  ModuleParam<T>& ModuleParamList::getModuleParameter(const std::string& name) const throw(FwExcModuleParameterNotFound, FwExcModuleParameterType)
  {
    //Check if a parameter with the given name exists
    std::map<std::string, ModuleParamPtr>::const_iterator mapIter;
    mapIter = m_paramMap.find(name);

    if (mapIter != m_paramMap.end()) {
      ModuleParamPtr moduleParam = mapIter->second;

      //Check the type of the stored parameter (currently done using the type identifier string)
      if (moduleParam->getTypeInfo() == typeid(T).name()) {
        ModuleParam<T>* explModParam = static_cast< ModuleParam<T>* >(moduleParam.get());
        return *explModParam;
      } else throw FwExcModuleParameterType(moduleParam->getTypeInfo(), typeid(T).name());
    } else throw FwExcModuleParameterNotFound(name);
  }


  template<typename T>
  void ModuleParamList::addParameter(const std::string& name, T& paramVariable, const T& defaultValue, const std::string& description)
  {
    ModuleParamPtr newParam(new ModuleParam<T>(paramVariable, description));

    //Check if a parameter with the given name already exists
    std::map<std::string, ModuleParamPtr>::iterator mapIter;
    mapIter = m_paramMap.find(name);

    if (mapIter == m_paramMap.end()) {
      m_paramMap.insert(std::make_pair(name, newParam));
      ModuleParam<T>* explModParam = static_cast< ModuleParam<T>* >(newParam.get());
      explModParam->setDefaultValue(defaultValue);
    } else {
      ERROR("A parameter with the name '" + name + "' already exists ! The name of a module parameter must be unique within a module.");
    }
  }


  template<typename T>
  void ModuleParamList::setParameter(const std::string& name, T& value)
  {
    try {
      ModuleParam<T>& explModParam = getModuleParameter<T>(name);
      explModParam.setValue(value);

    } catch (FwExcModuleParameterNotFound& exc) {
      ERROR("Could not find the parameter with the name '" + exc.getParamInfo1() + "' ! The value of the parameter could NOT be set.");

    } catch (FwExcModuleParameterType& exc) {
      std::string errorString = std::string("The type of the module parameter '") + name + std::string("' (") + exc.getParamInfo1();
      errorString += std::string(") is different from the type of the value it should be set to (") + exc.getParamInfo2() + std::string(").");
      errorString += std::string("The value of the parameter could NOT be set.");
      ERROR(errorString);
    }
  }


  template<typename T>
  void ModuleParamList::setParamObjectTemplate(const std::string& name, const boost::python::object& pyObj)
  {
    boost::python::extract<T> valueProxy(pyObj);
    if (valueProxy.check()) {
      T tmpValue = static_cast<T>(valueProxy);
      setParameter(name, tmpValue);
    } else {
      ERROR("Could not set a module parameter: The python object defined by '" + name + "' could not be converted !");
    }
  }


  template<typename T>
  void ModuleParamList::setParamListTemplate(const std::string& name, const boost::python::list& pyList)
  {
    std::vector<T> tmpList;
    int nList = boost::python::len(pyList);

    for (int iList = 0; iList < nList; ++iList) {
      boost::python::extract<T> checkValue(pyList[iList]);
      if (checkValue.check()) {
        tmpList.push_back(checkValue);
      } else {
        ERROR("Could not set a module parameter: A python object defined in the list '" + name + "' could not be converted !");
      }
    }
    setParameter(name, tmpList);
  }


  template<typename T>
  void ModuleParamList::getParamDefaultValuesSingleTemplate(const std::string& name, boost::python::list& outputList) const
  {
    try {
      ModuleParam<T>& explModParam = getModuleParameter<T>(name);
      PyObjConvUtils::addSingleValueToList<T>(explModParam.getDefaultValue(), outputList);

    } catch (FwExcModuleParameterNotFound& exc) {
      ERROR("Could not find the parameter with the name '" + exc.getParamInfo1() + "' ! The value of the parameter could NOT be set.");

    } catch (FwExcModuleParameterType& exc) {
      std::string errorString = std::string("The type of the module parameter '") + name + std::string("' (") + exc.getParamInfo1();
      errorString += std::string(") is different from the type of the value it should be set to (") + exc.getParamInfo2() + std::string(").");
      errorString += std::string("The value of the parameter could NOT be set.");
      ERROR(errorString);
    }
  }


  template<typename T>
  void ModuleParamList::getParamDefaultValuesListTemplate(const std::string& name, boost::python::list& outputList) const
  {
    try {
      ModuleParam<T>& explModParam = getModuleParameter<T>(name);
      PyObjConvUtils::addSTLVectorToList<T>(explModParam.getDefaultValue(), outputList);

    } catch (FwExcModuleParameterNotFound& exc) {
      ERROR("Could not find the parameter with the name '" + exc.getParamInfo1() + "' ! The value of the parameter could NOT be set.");

    } catch (FwExcModuleParameterType& exc) {
      std::string errorString = std::string("The type of the module parameter '") + name + std::string("' (") + exc.getParamInfo1();
      errorString += std::string(") is different from the type of the value it should be set to (") + exc.getParamInfo2() + std::string(").");
      errorString += std::string("The value of the parameter could NOT be set.");
      ERROR(errorString);
    }
  }


} //end of Belle2 namespace

#endif /* MODULEPARAMS_H_ */
