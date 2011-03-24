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

#include <framework/core/FrameworkExceptions.h>
#include <framework/core/ModuleParam.h>
#include <framework/core/PyObjConvUtils.h>
#include <framework/logging/Logger.h>

#include <map>
#include <vector>
#include <string>


namespace Belle2 {

  /**
   * The parameter type info class.
   * Stores information about each supported parameter type.
   */
  class ParamTypeInfo {
  public:

    /** Supported basic parameter types. */
    enum EParamBasicType {
      c_SingleParam,    /**< A single parameter value */
      c_ListParam,      /**< A list of parameter values */
      c_NotSupportedPBT /**< Not supported basic type */
    };

    /** Supported parameter value types. */
    enum EParamValueType {
      c_IntegerParam,   /**< An integer parameter value */
      c_DoubleParam,    /**< A double parameter value */
      c_StringParam,    /**< A string parameter value */
      c_BoolParam,      /**< A boolean parameter value */
      c_NotSupportedPVT /**< Not supported parameter type */
    };

    EParamBasicType  m_paramBasicType; /**< The basic parameter type (single/list). */
    EParamValueType  m_paramValueType; /**< The parameter value type. */
    std::string      m_readableName;   /**< The readable name of the parameter type. */

    /**
     * The ParamTypeInfo constructor.
     *
     * @param paramBasicType The basic parameter type (single/list).
     * @param paramValueType The parameter value type.
     * @param readableName The readable name of the parameter type.
     */
    ParamTypeInfo(EParamBasicType paramBasicType, EParamValueType paramValueType, std::string readableName) :
        m_paramBasicType(paramBasicType), m_paramValueType(paramValueType), m_readableName(readableName) {};
  };


  /**
   * The Module parameter list class.
   * Stores and manages all parameters of a module.
   */
  class ModuleParamList {

  public:

    //Define exceptions
    /** Exception is thrown if the requested parameter could not be found. */
    BELLE2_DEFINE_EXCEPTION(ModuleParameterNotFoundError, "Could not find the parameter with the name '%1%' ! The value of the parameter could NOT be set.");
    /** Exception is thrown if the type of the requested parameter is different from the expected type. */
    BELLE2_DEFINE_EXCEPTION(ModuleParameterTypeError, "The type of the module parameter '%1%' (%2%) is different from the type of the value it should be set to (%3%) !");

    /**
     * Constructor.
     */
    ModuleParamList();

    /**
     * Destructor.
     */
    ~ModuleParamList();

    /**
     * Adds a new parameter to the module list.
     *
     * A parameter consists of a reference pointing to a member variable in the module
     * which stores and allows fast access to the parameter value. In addition the type
     * of the parameter is saved and a description is given.
     *
     * @param name The unique name of the parameter.
     * @param paramVariable Reference to the variable which stores the parameter value.
     * @param description An description of the parameter.
     * @param defaultValue The default value of the parameter.
     */
    template<typename T>
    void addParameter(const std::string& name, T& paramVariable, const std::string& description, const T& defaultValue);

    /**
     * Adds a new enforced parameter to the module list.
     *
     * A parameter consists of a reference pointing to a member variable in the module
     * which stores and allows fast access to the parameter value. In addition the type
     * of the parameter is saved and a description is given.
     * This parameter has to be set by the user in the steering file.
     *
     * @param name The unique name of the parameter.
     * @param paramVariable Reference to the variable which stores the parameter value.
     * @param description An description of the parameter.
     */
    template<typename T>
    void addParameter(const std::string& name, T& paramVariable, const std::string& description);

    /**
     * Sets the value of a parameter given by its name.
     *
     * A template based method which is invoked by the module.
     *
     * @param name The unique name of the parameter.
     * @param value The parameter value which should be assigned to the parameter given by its name.
     */
    template<typename T>
    void setParameter(const std::string& name, T& value);

    /**
     * Returns a reference to a parameter. The returned parameter has already the correct type.
     *
     * Throws an exception of type ModuleParameterNotFoundError if a parameter with the given name does not exist.
     * Throws an exception of type ModuleParameterTypeError if the parameter type of does not match to the template parameter.
     *
     * @param name The unique name of the parameter.
     * @return A reference to a module parameter having the correct type.
     */
    template<typename T>
    ModuleParam<T>& getParameter(const std::string& name) const throw(ModuleParameterNotFoundError, ModuleParameterTypeError);

    /**
     * Returns the parameter type information of the given parameter.
     *
     * @param name The unique name of the parameter.
     * @return The parameter type information.
     */
    ParamTypeInfo getParamTypeInfo(const std::string& name) const;

    /**
     * Returns true if unset parameters exist which the user has to set in the steering file.
     *
     * @return True if unset parameters exist which the user has to set in the steering file.
     */
    bool hasUnsetForcedParams() const;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //                   Python API
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    /**
     * Returns a python list of all parameters.
     *
     * Each item in the list consists of the name of the parameter, a string describing its type,
     * a python list of all values, a python list of all default values, the information of
     * the parameter was set in the steering file and the description of the parameter.
     *
     * @return A python list containing the parameters of this parameter list.
     */
    boost::python::list getParamInfoListPython() const;

    /**
     * Implements a method for setting boost::python objects.
     *
     * The method supports the following types: int, double, string, bool
     * The conversion of the python object to the C++ type and the final storage of the
     * parameter value is done by specializing the template method setParamObjectTemplate().
     *
     * @param name The unique name of the parameter.
     * @param pyObj The object which should be converted and stored as the parameter value.
     */
    void setParamObjectPython(const std::string& name, const boost::python::object& pyObj);

    /**
     * Implements a method for setting boost::python lists.
     *
     * The method supports lists of the following types: int, double, string, bool
     * The conversion of the python list to the std::vector and the final storage of the
     * parameter value is done by specializing the template method setParamListTemplate().
     *
     * @param name The unique name of the parameter.
     * @param pyList The list which should be converted to a std::vector and stored as the parameter value.
     */
    void setParamListPython(const std::string& name, const boost::python::list& pyList);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


  protected:


  private:

    std::map<std::string, ModuleParamPtr> m_paramMap;  /**< Stores the module parameters together with a string name as key. */
    std::map<std::string, ParamTypeInfo>  m_paramTypeInfoMap; /**< Map which stores the typeid name as key and information about the parameter as value. */


    /**
     * Returns the type identifier of the parameter as string.
     *
     * The type identifier is used to discriminate between problematic parameter types (e.g. double and int)
     *
     * @param name The unique name of the parameter.
     * @return The type identifier as string. Returns an empty string if a parameter with the given name could not be found.
     */
    std::string getParamTypeString(const std::string& name) const;


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //                   Python API
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    /**
     * Implements a template based method for setting boost::python objects.
     *
     * The method converts a boost::python object to a C++ type and stores the result as the
     * parameter value.
     *
     * @param name The unique name of the parameter.
     * @param pyObj The object which should be converted and stored as the parameter value.
     */
    template<typename T>
    void setParamObjectTemplatePython(const std::string& name, const boost::python::object& pyObj);

    /**
     * Implements a template based method for setting boost::python lists.
     *
     * The method converts a boost::python list to a std::vector and stores the result as the
     * parameter value.
     *
     * @param name The unique name of the parameter.
     * @param pyList The list which should be converted to a std::vector and stored as the parameter value.
     */
    template<typename T>
    void setParamListTemplatePython(const std::string& name, const boost::python::list& pyList);

    /**
     * Returns a python list containing the default values of the given parameter.
     *
     * Calls according to the parameter type a specialized version of the template method getParamDefaultValuesTemplate().
     *
     * @param name The unique name of the parameter.
     * @param Reference to the output list containing the parameter default values. For single value parameter, the list only consists of one element.
     * @param defaultValues If true returns a list of default values otherwise a list of the parameter values.
     */
    void getParamValuesPython(const std::string& name, boost::python::list& outputList, bool defaultValues = false) const;

    /**
     * Returns a python list containing the parameter/default values of a single value parameter (template method).
     *
     * @param name The unique name of the parameter.
     * @param outputList The python list containing the parameter/default values.
     * @param defaultValues If true returns a list of default values otherwise a list of the parameter values.
     */
    template<typename T>
    void getParamObjectValuesTemplatePython(const std::string& name, boost::python::list& outputList, bool defaultValues = false) const;

    /**
     * Returns a python list containing the parameter/default values of a list parameter (template method).
     *
     * @param name The unique name of the parameter.
     * @param outputList The python list containing the parameter/default values.
     * @param defaultValues If true returns a list of default values otherwise a list of the parameter values.
     */
    template<typename T>
    void getParamListValuesTemplatePython(const std::string& name, boost::python::list& outputList, bool defaultValues = false) const;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  };


  //======================================================
  //       Implementation of template based methods
  //======================================================

  template<typename T>
  void ModuleParamList::addParameter(const std::string& name, T& paramVariable, const std::string& description, const T& defaultValue)
  {
    ModuleParamPtr newParam(new ModuleParam<T>(paramVariable, description, false));

    //Check if a parameter with the given name already exists
    std::map<std::string, ModuleParamPtr>::iterator mapIter;
    mapIter = m_paramMap.find(name);

    if (mapIter == m_paramMap.end()) {
      m_paramMap.insert(std::make_pair(name, newParam));
      ModuleParam<T>* explModParam = static_cast< ModuleParam<T>* >(newParam.get());
      explModParam->setDefaultValue(defaultValue);
    } else {
      B2ERROR("A parameter with the name '" + name + "' already exists ! The name of a module parameter must be unique within a module.")
    }
  }


  template<typename T>
  void ModuleParamList::addParameter(const std::string& name, T& paramVariable, const std::string& description)
  {
    ModuleParamPtr newParam(new ModuleParam<T>(paramVariable, description, true));

    //Check if a parameter with the given name already exists
    std::map<std::string, ModuleParamPtr>::iterator mapIter;
    mapIter = m_paramMap.find(name);

    if (mapIter == m_paramMap.end()) {
      m_paramMap.insert(std::make_pair(name, newParam));
    } else {
      B2ERROR("A parameter with the name '" + name + "' already exists ! The name of a module parameter must be unique within a module.")
    }
  }


  template<typename T>
  void ModuleParamList::setParameter(const std::string& name, T& value)
  {
    try {
      ModuleParam<T>& explModParam = getParameter<T>(name);
      explModParam.setValue(value);
    } catch (ModuleParameterNotFoundError& exc) {
      B2ERROR(exc.what())
    } catch (ModuleParameterTypeError& exc) {
      B2ERROR(exc.what())
    }
  }


  template<typename T>
  ModuleParam<T>& ModuleParamList::getParameter(const std::string& name) const throw(ModuleParameterNotFoundError, ModuleParameterTypeError)
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
      } else throw(ModuleParameterTypeError() << name << moduleParam->getTypeInfo() << typeid(T).name());
    } else throw(ModuleParameterNotFoundError() << name);
  }


  template<typename T>
  void ModuleParamList::setParamObjectTemplatePython(const std::string& name, const boost::python::object& pyObj)
  {
    boost::python::extract<T> valueProxy(pyObj);
    if (valueProxy.check()) {
      T tmpValue = static_cast<T>(valueProxy);
      setParameter(name, tmpValue);
    } else {
      B2ERROR("Could not set a module parameter: The python object defined by '" + name + "' could not be converted !")
    }
  }


  template<typename T>
  void ModuleParamList::setParamListTemplatePython(const std::string& name, const boost::python::list& pyList)
  {
    std::vector<T> tmpList;
    int nList = boost::python::len(pyList);

    for (int iList = 0; iList < nList; ++iList) {
      boost::python::extract<T> checkValue(pyList[iList]);
      if (checkValue.check()) {
        tmpList.push_back(checkValue);
      } else {
        B2ERROR("Could not set a module parameter: A python object defined in the list '" + name + "' could not be converted !")
      }
    }
    setParameter(name, tmpList);
  }


  template<typename T>
  void ModuleParamList::getParamObjectValuesTemplatePython(const std::string& name, boost::python::list& outputList, bool defaultValues) const
  {
    try {
      ModuleParam<T>& explModParam = getParameter<T>(name);
      ParamTypeInfo paramInfo = getParamTypeInfo(name);

      if (paramInfo.m_paramBasicType != ParamTypeInfo::c_SingleParam) {
        B2ERROR("The parameter type of parameter '" + name + "' is not a single parameter value !")
        return;
      }

      if (defaultValues) {
        PyObjConvUtils::addSingleValueToList<T>(explModParam.getDefaultValue(), outputList);
      } else {
        PyObjConvUtils::addSingleValueToList<T>(explModParam.getValue(), outputList);
      }
    } catch (ModuleParameterNotFoundError& exc) {
      B2ERROR(exc.what())
    } catch (ModuleParameterTypeError& exc) {
      B2ERROR(exc.what())
    }
  }


  template<typename T>
  void ModuleParamList::getParamListValuesTemplatePython(const std::string& name, boost::python::list& outputList, bool defaultValues) const
  {
    try {
      ModuleParam<T>& explModParam = getParameter<T>(name);
      ParamTypeInfo paramInfo = getParamTypeInfo(name);

      if (paramInfo.m_paramBasicType != ParamTypeInfo::c_ListParam) {
        B2ERROR("The parameter type of parameter '" + name + "' is not a list parameter value !")
        return;
      }

      if (defaultValues) {
        PyObjConvUtils::addSTLVectorToList<T>(explModParam.getDefaultValue(), outputList);
      } else {
        PyObjConvUtils::addSTLVectorToList<T>(explModParam.getValue(), outputList);
      }
    } catch (ModuleParameterNotFoundError& exc) {
      B2ERROR(exc.what())
    } catch (ModuleParameterTypeError& exc) {
      B2ERROR(exc.what())
    }
  }

} //end of Belle2 namespace

#endif /* MODULEPARAMS_H_ */
