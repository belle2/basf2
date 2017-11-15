/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Heck                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/ModuleParamList.h>
#include <framework/core/ModuleCondition.h>
#include <framework/core/PathElement.h>

#include <framework/core/FrameworkExceptions.h>
#include <framework/logging/LogConfig.h>
#include <framework/logging/Logger.h>

#include <memory>

#include <list>
#include <string>
#include <memory>

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

  class Module;
  class Path;

  /** Defines a pointer to a module object as a boost shared pointer. */
  typedef std::shared_ptr<Module> ModulePtr;

  /**
   * Base class for Modules.
   *
   * A module is the smallest building block of the framework.
   * A typical event processing chain consists of a Path containing
   * modules. By inheriting from this base class, various types of
   * modules can be created.
   *
   * Each module class is identified by its unique name, and should end in ...Module.
   * To make the module 'SomeRecoModule' known to the framework, use REG_MODULE(SomeReco).
   * It will then show up in the module list as 'SomeReco'.
   *
   * Modules can also define a return value (int or bool) using setReturnValue(),
   * which can be used in the steering file to split the Path based on the set value:
   *  \code
      module_with_condition.if_value("<1", another_path)
      \endcode
   * In case the module condition for a given event is less than 1, the execution
   * will be diverted into another_path for this event. You could for example set
   * a special return value if an error occurs, and divert the execution into a
   * path containing RootOutput if it is found; saving only the data producing/
   * produced by the error.
   *
   * The 'Module Development' section in the manual provides detailed information
   * on how to create modules, setting parameters, or using return values/conditions:
   * https://confluence.desy.de/display/BI/Software+Basf2manual#Module_Development
   */
  class Module : public PathElement {

  public:

    /** Each module can be tagged with property flags, which indicate certain features of the module. */
    enum EModulePropFlags {
      c_Input                       = 1,  /**< This module is an input module (reads data). */
      c_Output                      = 2,  /**< This module is an output module (writes data). */
      c_ParallelProcessingCertified = 4,  /**< This module can be run in parallel processing mode safely (All I/O must be done through the data store, in particular, the module must not write any files.) */
      c_HistogramManager            = 8, /**< This module is used to manage histograms accumulated by other modules */
      c_InternalSerializer          = 16,  /**< This module is an internal serializer/deserializer for parallel processing */
      c_TerminateInAllProcesses     = 32,  /**< When using parallel processing, call this module's terminate() function in all processes(). This will also ensure that there is exactly one process (single-core if no parallel modules found) or at least one input, one main and one output process. */
      c_DontCollectStatistics       = 64,  /**< No statistics is collected for this module. */
    };

    /// Forward the EAfterConditionPath definition from the ModuleCondition.
    typedef ModuleCondition::EAfterConditionPath EAfterConditionPath;

    /**
     * Constructor.
     *
     * In the constructor of your derived class, you can create and allocate memory for variables.
     * Add the module parameters in this method. You should also set a description using setDescription() here.
     *
     * Please avoid producing output in the constructor, as it would show up in
     * the module list (basf2 -m).
     */
    Module();

    /**
     * Initialize the Module.
     *
     * This method is called once before the actual event processing starts.
     * Use this method to initialize variables, open files etc.
     *
     * This method can be implemented by subclasses.
     */
    virtual void initialize() {};

    /**
     * Called when entering a new run.
     *
     * Called at the beginning of each run, the method gives you the chance to
     * change run dependent constants like alignment parameters, etc.
     *
     * This method can be implemented by subclasses.
     */
    virtual void beginRun() {};

    /**
     * This method is the core of the module.
     *
     * This method is called for each event. All processing of the event has to
     * take place in this method.
     *
     * This method can be implemented by subclasses.
     */
    virtual void event() {};

    /**
     * This method is called if the current run ends.
     *
     * Use this method to store information, which should be aggregated over one run.
     *
     * This method can be implemented by subclasses.
     */
    virtual void endRun() {};

    /**
     * This method is called at the end of the event processing.
     *
     * This method is called only once after the event processing finished.
     * Use this method for cleaning up, closing files, etc.
     *
     * This method can be implemented by subclasses.
     */
    virtual void terminate() {};

    /**
     * Returns the name of the module.
     *
     * This can be changed via e.g. set_name() in the steering file to give
     * more useful names if there is more than one module of the same type.
     *
     * For identifying the type of a module, using  getType() (or type()
     * in Python) is recommended.
     */
    const std::string& getName() const {return m_name;}

    /**
     * Returns the type of the module (i.e. class name minus 'Module')
     */
    const std::string& getType() const;

    /**
     * Returns the package this module is in.
     */
    const std::string& getPackage() const {return m_package;}

    /**
     * Returns the description of the module.
     */
    const std::string& getDescription() const {return m_description;}

    /**
     * Set the name of the module.
     *
     * \note
     * The module name is set when using the REG_MODULE macro,
     * but the module can be renamed before calling process()
     * using the set_name() function in your steering file.
     *
     * @param name The name of the module
     */
    void setName(const std::string& name) { m_name = name; };
    /**
     * Sets the flags for the module properties.
     *
     * @param propertyFlags bitwise OR of EModulePropFlags
     */
    void setPropertyFlags(unsigned int propertyFlags);

    /**
     * Returns the log system configuration.
     */
    LogConfig& getLogConfig() {return m_logConfig;}

    /**
     * Set the log system configuration.
     */
    void setLogConfig(const LogConfig& logConfig) {m_logConfig = logConfig;}

    /**
     * Configure the log level.
     */
    void setLogLevel(int logLevel);

    /**
     * Configure the debug messaging level.
     */
    void setDebugLevel(int debugLevel);

    /**
     * Configure the abort log level.
     */
    void setAbortLevel(int abortLevel);

    /**
     * Configure the printed log information for the given level.
     *
     * @param logLevel The log level (one of LogConfig::ELogLevel)
     * @param logInfo  What kind of info should be printed? ORed combination of LogConfig::ELogInfo flags.
     */
    void setLogInfo(int logLevel, unsigned int logInfo);


    /**
     * Add a condition to the module. Please note that successive calls of this function will add more than
     * one condition to the module.
     * If more than one condition results in true, only the *last* of them will be used.
     *
     * See https://confluence.desy.de/display/BI/Software+ModCondTut or ModuleCondition for a description of the syntax.
     *
     * Please be careful: Avoid creating cyclic paths, e.g. by linking a condition
     * to a path which is processed before the path where this module is
     * located in.
     *
     * @param expression The expression of the condition.
     * @param path       Shared pointer to the Path which will be executed if the condition is evaluated to true.
     * @param afterConditionPath  What to do after executing 'path'.
     */
    void if_value(const std::string& expression, std::shared_ptr<Path> path,
                  EAfterConditionPath afterConditionPath = EAfterConditionPath::c_End);

    /**
     * A simplified version to add a condition to the module. Please note that successive calls of this function will
     * add more than one condition to the module.
     * If more than one condition results in true, only the *last* of them will be used.
     *
     * Please be careful: Avoid creating cyclic paths, e.g. by linking a condition
     * to a path which is processed before the path where this module is
     * located in.
     *
     * It is equivalent to the if_value() method, using the expression "<1".
     * This method is meant to be used together with the setReturnValue(bool value) method.
     *
     * @param path Shared pointer to the Path which will be executed if the return value is _false_.
     * @param afterConditionPath  What to do after executing 'path'.
     */
    void if_false(std::shared_ptr<Path> path, EAfterConditionPath afterConditionPath = EAfterConditionPath::c_End);

    /**
     * A simplified version to set the condition of the module. Please note that successive calls of this function will
     * add more than one condition to the module.
     * If more than one condition results in true, only the *last* of them will be used.
     *
     * Please be careful: Avoid creating cyclic paths, e.g. by linking a condition
     * to a path which is processed before the path where this module is
     * located in.
     *
     * It is equivalent to the if_value() method, using the expression ">=1".
     * This method is meant to be used together with the setReturnValue(bool value) method.
     *
     * @param path Shared pointer to the Path which will be executed if the return value is _true_.
     * @param afterConditionPath  What to do after executing 'path'.
     */
    void if_true(std::shared_ptr<Path> path, EAfterConditionPath afterConditionPath = EAfterConditionPath::c_End);

    /**
     * Returns true if at least one condition was set for the module.
     */
    bool hasCondition() const { return not m_conditions.empty(); };

    /** Return a pointer to the first condition (or nullptr, if none was set) */
    const ModuleCondition* getCondition() const
    {
      if (m_conditions.empty()) {
        return nullptr;
      } else {
        return &m_conditions.front();
      }
    }

    /** Return all set conditions for this module. */
    const std::vector<ModuleCondition>& getAllConditions() const
    {
      return m_conditions;
    }

    /**
     * If at least one condition was set, it is evaluated and true returned if at least one condition returns true.
     *
     * If no condition or result value was defined, the method returns false.
     * Otherwise, the condition is evaluated and true returned, if at least one condition returns true.
     * To speed up the evaluation, the condition strings were already parsed in the method if_value().
     *
     * @return True if at least one condition and return value exists and at least one condition expression was
     *         evaluated to true.
     */
    bool evalCondition() const;

    /** Returns the path of the last true condition (if there is at least one, else reaturn a null pointer).  */
    std::shared_ptr<Path> getConditionPath() const;

    /** What to do after the conditional path is finished. (defaults to c_End if no condition is set)*/
    Module::EAfterConditionPath getAfterConditionPath() const;

    /** Return all condition paths currently set (no matter if the condition is true or not). */
    std::vector<std::shared_ptr<Path>> getAllConditionPaths() const;

    /**
     * Returns true if all specified property flags are available in this module.
     *
     * @param propertyFlags Ored EModulePropFlags which should be compared with the module flags.
     */
    bool hasProperties(unsigned int propertyFlags) const;

    /**
     * Returns true and prints error message if the module has unset parameters which the user has to set in the steering file.
     */
    bool hasUnsetForcedParams() const;

    /** Return module param list. */
    const ModuleParamList& getParamList() const { return m_moduleParamList; }

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
    ModuleParam<T>& getParam(const std::string& name) const;

    /** Create an independent copy of this module.
     *
     * Note that parameters are shared, so changing them on a cloned module will also affect
     * the original module.
     */
    std::shared_ptr<PathElement> clone() const override;


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //                   Python API
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    /**
     * Returns a python list of all parameters.
     *
     * Each item in the list consists of the name of the parameter, a string describing its type,
     * a python list of all default values and the description of the parameter.
     * @return A python list containing the parameters of this parameter list.
     */
    std::shared_ptr<boost::python::list> getParamInfoListPython() const;

    /**
     * Exposes methods of the Module class to Python.
     */
    static void exposePythonAPI();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


  protected:
    //@{
    /** Wrappers to make the methods without "def_" prefix callable from Python. Overridden in PyModule. */

    /**
     * Wrapper method for the virtual function initialize() that has the implementation
     * to be used in a call from Python.
     */
    virtual void def_initialize() { initialize(); }

    /**
     * Wrapper method for the virtual function beginRun() that has the implementation
     * to be used in a call from Python.
     */
    virtual void def_beginRun() { beginRun(); }

    /**
     * Wrapper method for the virtual function event() that has the implementation
     * to be used in a call from Python.
     */
    virtual void def_event() { event(); }

    /**
     * This method can receive that the current run ends as a call from the Python side.
     *
     * For regular C++-Modules that forwards the call to the regular endRun() method.
     */
    virtual void def_endRun() { endRun(); }

    /**
     * Wrapper method for the virtual function terminate() that has the implementation
     * to be used in a call from Python.
     */
    virtual void def_terminate() { terminate(); }
    //@}


    /**
     * Sets the description of the module.
     *
     * @param description A description of the module.
     */
    void setDescription(const std::string& description);

    /** Set the module type. Only for use by internal modules (which don't use the normal REG_MODULE mechanism). */
    void setType(const std::string& type);

    /**
     * Adds a new parameter to the module. This method has to be called in the constructor of the module.
     *
     * @param name The unique name of the parameter.
     * @param paramVariable The member variable of the module to which the value from the steering file is written.
     * @param description A description of the parameter.
     * @param defaultValue The default value of the parameter which is used if there was no value given in the steering file.
     */
    template<typename T>
    void addParam(const std::string& name, T& paramVariable, const std::string& description, const T& defaultValue);

    /**
     * Adds a new enforced parameter to the module. This method has to be called in the constructor of the module.
     * The user has to set the value for this parameter in the steering file.
     *
     * @param name The unique name of the parameter.
     * @param paramVariable The member variable of the module to which the value from the steering file is written.
     * @param description A description of the parameter.
     */
    template<typename T>
    void addParam(const std::string& name, T& paramVariable, const std::string& description);

    /**
     * Sets the return value for this module as integer.
     * The value can be used in the steering file to divide the analysis chain
     * into several paths.
     *
     * @param value The value of the return value.
     */
    void setReturnValue(int value);

    /**
     * Sets the return value for this module as bool.
     * The bool value is saved as an integer with the convention 1 meaning true and 0 meaning false.
     * The value can be used in the steering file to divide the analysis chain
     * into several paths.
     *
     * @param value The value of the return value.
     */
    void setReturnValue(bool value);

    /** Replace existing parameter list. */
    void setParamList(const ModuleParamList& params) { m_moduleParamList = params; }


  private:
    /** no submodules, return empty list */
    std::list<ModulePtr> getModules() const override { return std::list<ModulePtr>(); }

    std::string m_name;           /**< The name of the module, saved as a string (user-modifiable) */
    std::string m_type;           /**< The type of the module, saved as a string. */
    std::string m_package;        /**< Package this module is found in (may be empty). */
    std::string m_description;    /**< The description of the module. */
    unsigned int m_propertyFlags; /**< The properties of the module as bitwise or (with |) of EModulePropFlags. */

    LogConfig m_logConfig;        /**< The log system configuration of the module. */

    ModuleParamList m_moduleParamList; /**< List storing and managing all parameter of the module. */

    bool m_hasReturnValue;     /**< True, if the return value is set. */
    int  m_returnValue;        /**< The return value. */

    std::vector<ModuleCondition> m_conditions; /**< Module condition, only non-null if set. */

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //                    Python API
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    /** return the module name. */
    std::string getPathString() const override;

    /**
     * Implements a method for setting boost::python objects.
     * The method supports the following types: list, dict, int, double, string, bool
     * The conversion of the python object to the C++ type and the final storage of the
     * parameter value is done in the ModuleParam class.
     *
     * @param name The unique name of the parameter.
     * @param pyObj The object which should be converted and stored as the parameter value.
     */
    void setParamPython(const std::string& name, const boost::python::object& pyObj);

    /**
     * Implements a method for reading the parameter values from a boost::python dictionary.
     * The key of the dictionary has to be the name of the parameter and the value has to
     * be of one of the supported parameter types.
     *
     * @param dictionary The python dictionary from which the parameter values are read.
     */
    void setParamPythonDict(const boost::python::dict& dictionary);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    friend class ModuleProxyBase;
  };


  //------------------------------------------------------
  //       Implementation of template based methods
  //------------------------------------------------------
  template<typename T>
  void Module::addParam(const std::string& name, T& paramVariable, const std::string& description, const T& defaultValue)
  {
    m_moduleParamList.addParameter(name, paramVariable, description, defaultValue);
  }


  template<typename T>
  void Module::addParam(const std::string& name, T& paramVariable, const std::string& description)
  {
    m_moduleParamList.addParameter(name, paramVariable, description);
  }

  template<typename T>
  ModuleParam<T>& Module::getParam(const std::string& name) const
  {
    return m_moduleParamList.getParameter<T>(name);
  }


  //------------------------------------------------------
  //             Define convenient typdefs
  //------------------------------------------------------

  /** Defines a std::list of shared module pointers.*/
  typedef std::list<ModulePtr> ModulePtrList;


  //------------------------------------------------------
  //  Proxy class for creating an instance of the module
  //------------------------------------------------------
  /**
   * The base module proxy class is used to create new instances of a module.
   * By loading a shared library, which contains modules, the proxy class of each
   * module is automatically registered to the global ModuleManager. If an instance of
   * a module is required, its proxy class is responsible to create an instance of the
   * module.
   */
  class ModuleProxyBase {

  public:
    /**
     * The constructor of the ModuleProxyBase class.
     * The constructor registers the proxy to the ModuleManager.
     * @param moduleType The type name of the module.
     * @param package the package which contains the module
     */
    ModuleProxyBase(const std::string& moduleType, const std::string& package);

    /**
     * The destructor of the ModuleProxyBase class.
     */
    virtual ~ModuleProxyBase() {};

    /**
     * Abstract method which creates a new module and returns a shared pointer to it.
     * Instances of modules can only be created by this method.
     * @return A shared pointer to the created module instance.
     */
    ModulePtr createModule() const
    {
      ModulePtr nm(createInstance());
      nm->setType(m_moduleType);
      nm->setName(m_moduleType);
      nm->m_package = m_package;
      return nm;
    }

    /**
     * Returns the module name of the module associated to this proxy.
     */
    const std::string& getModuleName() const {return m_moduleType; }


  protected:
    /** create a new instance of the module in question */
    virtual Module* createInstance() const = 0;

    std::string m_moduleType; /**< The type name of the module. (without trailing "Module") */
    std::string m_package; /**< Package this module is found in (may be empty). */
  };


#ifndef _PACKAGE_
#define _PACKAGE_ "" /**< make sure REG_MODULE works outside the normal build system */
#endif

  //------------------------------------------------------
  //             Define convenient macro
  //------------------------------------------------------
  /** Register the given module (without 'Module' suffix) with the framework. */
#define REG_MODULE(moduleName) namespace { struct ModuleProxy##moduleName: public ModuleProxyBase { \
      ModuleProxy##moduleName(): ModuleProxyBase(#moduleName, "" _PACKAGE_) {} \
      virtual ::Belle2::Module* createInstance() const override final { return new moduleName##Module(); } \
    } proxy##moduleName##Module; }

} // end namespace Belle2
