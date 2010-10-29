/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MODULEPARAM_H_
#define MODULEPARAM_H_

#include <boost/python.hpp>
#include <boost/python/list.hpp>
#include <boost/shared_ptr.hpp>

#include <map>
#include <string>
#include <typeinfo>
#include <list>


namespace Belle2 {


  /**
   * Base class for module parameter.
   * The base class stores all common information for parameters.
   */
  class ModuleParamBase {

  public:

    /**
     * Constructor.
     *
     * @param typeInfo The type string of the parameter.
     * @param description The description of the parameter.
     * @param force If true the parameter has to be set in the steering file by the user.
     */
    ModuleParamBase(const std::string& typeInfo, const std::string& description, bool force)
        : m_typeInfo(typeInfo), m_description(description), m_forceInSteering(force), m_setInSteering(false) {};

    /**
     * Destructor.
     */
    virtual ~ModuleParamBase() {};

    /**
     * Returns the type identifier of the parameter as string.
     *
     * The type identifier is used to discriminate between problematic parameter types (e.g. double and int).
     * @return The type identifier as string.
     */
    const std::string& getTypeInfo() const {return m_typeInfo; }

    /**
     * Returns the description of the parameter.
     *
     * @return The description of the parameter.
     */
    const std::string& getDescription() const {return m_description; }

    /**
     * Returns true if the parameter was set in the steering file.
     *
     * If the value was not set in the steering file but is still the
     * default value, this method returns false.
     *
     * @return True if the parameter was set in the steering file.
     */
    const bool isSetInSteering() const {return m_setInSteering; }


    /**
     * Returns true if the parameter has to be set by the user in the steering file.
     *
     * @return True if the parameter has to be set in the steering file.
     */
    const bool isForcedInSteering() const {return m_forceInSteering; }


  protected:

    std::string m_typeInfo;        /**< The type of the parameter stored as string. */
    std::string m_description;     /**< The (optional) description of the parameter. */
    bool        m_forceInSteering; /**< If true the parameter has to be set by the user in the steering file. */
    bool        m_setInSteering;   /**< True, if the parameter value was changed in the steering file. */


  private:

  };


  /**
   * A single parameter of the module.
   * Implements a single parameter of the module as a template class.
   * Inherits from the module parameter base class.
   */
  template<class T>
  class ModuleParam : public ModuleParamBase {

  public:

    /**
     * Constructor.
     *
     * A parameter consists of a reference pointing to a member variable in the module
     * which stores and allows fast access to the parameter value. In addition the type
     * of the parameter is saved and optionally a description can be given.
     *
     * @param paramVariable Reference to the variable which stores the parameter value.
     * @param description The optional description of the parameter.
     * @param force If true the parameter has to be set by the user in the steering file.
     */
    ModuleParam(T& paramVariable, const std::string& description = "", bool force = false)
        : ModuleParamBase(typeid(T).name(), description, force), m_paramVariable(paramVariable) {};

    /**
     * Destructor.
     */
    virtual ~ModuleParam() {};

    /**
     * Sets the value of a parameter.
     *
     * @param value The parameter value which should be assigned to the parameter.
     */
    void setValue(T value) {
      m_paramVariable = value;
      m_setInSteering = true;
    }

    /**
     * Sets the default value of a parameter.
     *
     * @param defaultValue The parameter default value of the parameter.
     */
    void setDefaultValue(T defaultValue) {
      m_defaultValue = defaultValue;
      m_paramVariable = defaultValue;
      m_setInSteering = false;
    }

    /**
     * Returns the value of the parameter.
     *
     * @return The value of the parameter.
     */
    T& getValue() {return m_paramVariable; }

    /**
     * Returns the default value of the parameter.
     *
     * @return The default value of the parameter.
     */
    T& getDefaultValue() {return m_defaultValue; }

    /**
     * Resets the parameter value by assigning the default value to the parameter value.
     */
    void resetValue() {
      m_paramVariable = m_defaultValue;
      m_setInSteering = false;
    };


  protected:


  private:

    T m_defaultValue;   /**< The default value of the parameter. */
    T& m_paramVariable; /**< Reference to the member variable in the module which stores the parameter value. */

  };


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  //                  Python API
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  /**
   * Class to store basic information about a parameter.
   * This class is used in the Python API to provide the user with information about a parameter.
   */
  class ModuleParamInfoPython {

  public:

    std::string m_name;                  /**< The name of the parameter. */
    std::string m_typeName;              /**< The name of the type of the parameter. */
    boost::python::list m_defaultValues; /**< The default values of the parameter as a python list. */
    boost::python::list m_values;        /**< The values of the parameter as a python list. */
    std::string m_description;           /**< The description of the parameter. */
    bool m_forceInSteering;              /**< If true the parameter has to be set by the user in the steering file. */
    bool m_setInSteering;                /**< True if the parameter was set in the steering file. */


    /**
     * Exposes methods of the ModuleParam class to Python.
     */
    static void exposePythonAPI() {
      //Python class definition
      boost::python::class_<ModuleParamInfoPython>("ModuleParamInfo")
      .def_readonly("name", &ModuleParamInfoPython::m_name)
      .def_readonly("type", &ModuleParamInfoPython::m_typeName)
      .def_readonly("default", &ModuleParamInfoPython::m_defaultValues)
      .def_readonly("values", &ModuleParamInfoPython::m_values)
      .def_readonly("description", &ModuleParamInfoPython::m_description)
      .def_readonly("forceInSteering", &ModuleParamInfoPython::m_forceInSteering)
      .def_readonly("setInSteering", &ModuleParamInfoPython::m_setInSteering)
      ;
    }
  };
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


  //------------------------------------------------------
  //             Define convenient typdefs
  //------------------------------------------------------

  typedef boost::shared_ptr<ModuleParamBase> ModuleParamPtr;


} //end of Belle2 namespace

#endif /* MODULEPARAM_H_ */
