/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MODULEPARAMLIST_H_
#define MODULEPARAMLIST_H_

#include <framework/core/ModuleParam.h>
#include <framework/core/FrameworkExceptions.h>
#include <framework/logging/Logger.h>

#include <framework/core/PyObjConvUtils.h>

#include <boost/shared_ptr.hpp>

#include <map>
#include <string>

namespace boost {
  namespace python {
    class list;
    class dict;
    namespace api {
      class object;
    }
    using api::object;
  }
}


namespace Belle2 {

  /**
   * The Module parameter list class.
   * Stores and manages all parameters of a module.
   */
  class ModuleParamList {

  public:

    //Define exceptions
    /** Exception is thrown if the requested parameter could not be found. */
    BELLE2_DEFINE_EXCEPTION(ModuleParameterNotFoundError,
                            "Could not find the parameter with the name '%1%'! The value of the parameter could NOT be set.");
    /** Exception is thrown if the type of the requested parameter is different from the expected type. */
    BELLE2_DEFINE_EXCEPTION(ModuleParameterTypeError,
                            "The type of the module parameter '%1%' (%2%) is different from the type of the value it should be set to (%3%)!");

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
    void setParameter(const std::string& name, const T& value);

    /** Set values for parameters from other parameter list. */
    void setParameters(const ModuleParamList& params);

    /**
     * Returns the names of all parameters in this parameter list.
     *
     * @return The names of the parameters
     */
    std::vector<std::string> getParameterNames() const;

    /**
     * Returns the description of a parameter given by its name.
     *
     * Throws an exception of type ModuleParameterNotFoundError if a parameter with the given name does not exist.
     *
     * @param name The unique name of the parameter.
     * @return The description of the parameter as string
     */
    std::string getParameterDescription(const std::string& name) const;

    /**
     * Returns the type info of a parameter given by its name.
     *
     * Throws an exception of type ModuleParameterNotFoundError if a parameter with the given name does not exist.
     *
     * @param name The unique name of the parameter.
     * @return The type information as descriptive string
     */
    std::string getParameterTypeInfo(const std::string& name) const;

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
    ModuleParam<T>& getParameter(const std::string& name) const;

    /** Returns list of unset parameters (if they are required to have a value. */
    std::vector<std::string> getUnsetForcedParams() const;

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
    boost::shared_ptr<boost::python::list> getParamInfoListPython() const;

    /**
     * Implements a method for setting boost::python objects.
     *
     * The method supports the following types: dict, list, int, double, string, bool
     * The conversion of the python object to the C++ type and the final storage of the
     * parameter value is done by specialized converter in PyObjConvUtils.h
     *
     * @param name The unique name of the parameter.
     * @param pyObj The object which should be converted and stored as the parameter value.
     */
    template<typename PythonObject>
    void setParamPython(const std::string& name, const PythonObject& pyObj);


    /**
     * Returns a python object containing the value or default value of the given parameter.
     *
     * Calls according to the parameter type a specialized converter in PyObjConvUtils.h
     *
     * @param name The unique name of the parameter.
     * @param pyOutput Reference to the output object containing the parameter value or default values.
     * @param defaultValues If true returns a list of default values otherwise a list of the parameter values.
     */
    template<typename PythonObject>
    void getParamValuesPython(const std::string& name, PythonObject& pyOutput, bool defaultValues) const;


  protected:


  private:

    std::map<std::string, ModuleParamPtr> m_paramMap;  /**< Stores the module parameters together with a string name as key. */

    /**
     * Returns a ModuleParamPtr to a parameter.
     *
     * Throws an exception of type ModuleParameterNotFoundError if a parameter with the given name does not exist.
     *
     * @param name The unique name of the parameter.
     * @return A ModuleParamPtr to a module parameter.
     */
    ModuleParamPtr getParameterPtr(const std::string& name) const;

    /**
     * Returns the type identifier of the parameter as string.
     *
     * The type identifier is used to discriminate between problematic parameter types (e.g. double and int)
     *
     * @param name The unique name of the parameter.
     * @return The type identifier as string. Returns an empty string if a parameter with the given name could not be found.
     */
    std::string getParamTypeString(const std::string& name) const;

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
      B2ERROR("A parameter with the name '" + name + "' already exists! The name of a module parameter must be unique within a module.");
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
      B2ERROR("A parameter with the name '" + name + "' already exists! The name of a module parameter must be unique within a module.");
    }
  }


  template<typename T>
  void ModuleParamList::setParameter(const std::string& name, const T& value)
  {
    try {
      ModuleParam<T>& explModParam = getParameter<T>(name);
      explModParam.setValue(value);
    } catch (ModuleParameterNotFoundError& exc) {
      B2ERROR(exc.what());
    } catch (ModuleParameterTypeError& exc) {
      B2ERROR(exc.what());
    }
  }


  template<typename T>
  ModuleParam<T>& ModuleParamList::getParameter(const std::string& name) const
  {
    //Check if a parameter with the given name exists
    std::map<std::string, ModuleParamPtr>::const_iterator mapIter;
    mapIter = m_paramMap.find(name);

    if (mapIter != m_paramMap.end()) {
      ModuleParamPtr moduleParam = mapIter->second;

      //Check the type of the stored parameter (currently done using the type identifier string)
      if (moduleParam->getTypeInfo() == PyObjConvUtils::Type<T>::name()) {
        ModuleParam<T>* explModParam = static_cast< ModuleParam<T>* >(moduleParam.get());
        return *explModParam;
      } else throw (ModuleParameterTypeError() << name << moduleParam->getTypeInfo() << PyObjConvUtils::Type<T>::name());
    } else throw (ModuleParameterNotFoundError() << name);
  }


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                   Python API
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  template<typename PythonObject>
  void ModuleParamList::setParamPython(const std::string& name, const PythonObject& pyObj)
  {

    ModuleParamPtr p = getParameterPtr(name);
    p->setValueFromPythonObject(pyObj);

  }

  template<typename PythonObject>
  void ModuleParamList::getParamValuesPython(const std::string& name, PythonObject& pyOutput, bool defaultValues) const
  {
    try {
      ModuleParamPtr p = getParameterPtr(name);
      p->setValueToPythonObject(pyOutput, defaultValues);
    } catch (ModuleParamList::ModuleParameterNotFoundError& exc) {
      B2ERROR(exc.what());
    }
  }

} //end of Belle2 namespace

#endif
