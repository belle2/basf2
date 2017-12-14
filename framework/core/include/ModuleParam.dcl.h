/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/ModuleParamBase.h>

#include <string>

namespace boost {
  namespace python {
    namespace api {
      class object;
    }
    using api::object;
  }
}

namespace Belle2 {
  /**
   * A single parameter of the module.
   * Implements a single parameter of the module as a template class.
   * Inherits from the module parameter base class.
   */
  template <class T>
  class ModuleParam : public ModuleParamBase {

  public:
    /**
     * Static function to generate a string description for the parameter type.
     *
     * This string will be used to identify the contained type and
     * guaranties some level of type safety in this type erasure.
     */
    static std::string TypeInfo();

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
    ModuleParam(T& paramVariable, const std::string& description = "", bool force = false);

    /**
     * Destructor.
     */
    virtual ~ModuleParam();

    /**
     * Sets the value of a parameter.
     *
     * @param value The parameter value which should be assigned to the parameter.
     */
    void setValue(T value);

    /**
     * Sets the default value of a parameter.
     *
     * @param defaultValue The parameter default value of the parameter.
     */
    void setDefaultValue(T defaultValue);

    /**
     * Returns the value of the parameter.
     *
     * @return The value of the parameter.
     */
    T& getValue();

    /**
     * Returns the default value of the parameter.
     *
     * @return The default value of the parameter.
     */
    T& getDefaultValue();

    /**
     * Implements a method for setting boost::python objects.
     *
     * The method supports the following types: list, dict, int, double, string, bool
     * The conversion of the python object to the C++ type and the final storage of the
     * parameter value is done by PyObjConvUtils::convertPythonObject.
     *
     * @param pyObject The object which should be converted and stored as the parameter value.
     */
    virtual void setValueFromPythonObject(const boost::python::object& pyObject) final;

    /**
     * Returns a python object containing the value or the default value of the given parameter.
     *
     * @param pyObject Reference to the python object which is set to the parameter value.
     * @param defaultValues If true returns default value otherwise parameter value.
     */
    virtual void
    setValueToPythonObject(boost::python::object& pyObject, bool defaultValues = false) const final;

    /** Set value from other ModuleParam of same type. */
    virtual void setValueFromParam(const ModuleParamBase& param) final;

    /**
     * Resets the parameter value by assigning the default value to the parameter value.
     */
    void resetValue();

  private:
    /// The default value of the parameter.
    T m_defaultValue;

    /// Reference to the member variable in the module which stores the parameter value.
    T& m_paramVariable;
  };
} // end of Belle2 namespace
