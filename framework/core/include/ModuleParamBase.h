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
} // end of Belle2 namespace
