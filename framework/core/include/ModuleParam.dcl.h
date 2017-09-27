/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef MODULEPARAM_DCL_H_
#define MODULEPARAM_DCL_H_

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
    ModuleParamBase(const std::string& typeInfo, const std::string& description, bool force);

    /**
     * Destructor.
     */
    virtual ~ModuleParamBase();

    /**
     * Returns the type identifier of the parameter as string.
     *
     * The type identifier is used to discriminate between problematic parameter types (e.g. double
     * and int).
     * @return The type identifier as string.
     */
    const std::string& getTypeInfo() const;

    /**
     * Returns the description of the parameter.
     *
     * @return The description of the parameter.
     */
    const std::string& getDescription() const;

    /**
     * Returns true if the parameter was set in the steering file.
     *
     * If the value was not set in the steering file but is still the
     * default value, this method returns false.
     *
     * @return True if the parameter was set in the steering file.
     */
    bool isSetInSteering() const;

    /**
     * Returns true if the parameter has to be set by the user in the steering file.
     *
     * @return True if the parameter has to be set in the steering file.
     */
    bool isForcedInSteering() const;

    /**
     * Pure virtual function.
     * Set the value of the parameter from a python object. The derived templated ModuleParam class
     * calls the converter functions in PyObjConvUtils.h with the right template arguments.
     * Hence the compiler can automatically infer the right converter.
     *
     * @param pyObject The python object which holds the parameter value
     */
    virtual void setValueFromPythonObject(const boost::python::object& pyObject) = 0;

    /**
     * Pure virtual function.
     * Set the value of a python objects to stored parameter value. The derived templated
     * ModuleParam class
     * calls the converter functions in PyObjConvUtils.h with the right template arguments.
     * Hence the compiler can automatically infer the right converters
     *
     * @param pyObject The python object which should be filled with the parameter value
     * @param defaultValues If true, the python object is set to the default value of this
     * parameter.
     */
    virtual void
    setValueToPythonObject(boost::python::object& pyObject, bool defaultValues = false) const = 0;

    /** Set value from other ModuleParam of same type. */
    virtual void setValueFromParam(const ModuleParamBase& param) = 0;

  protected:
    /// The type of the parameter stored as string.
    std::string m_typeInfo;

    /// The (optional) description of the parameter.
    std::string m_description;

    /// If true the parameter has to be set by the user in the steering file.
    bool m_forceInSteering;

    /// True, if the parameter value was changed in the steering file.
    bool m_setInSteering;
  };

  /**
   * A single parameter of the module.
   * Implements a single parameter of the module as a template class.
   * Inherits from the module parameter base class.
   */
  template <class T>
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

#endif /* MODULEPARAM_DCL_H_ */
