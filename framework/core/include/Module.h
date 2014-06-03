/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Heck                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MODULE_H
#define MODULE_H

#include <framework/core/ModuleParamList.h>
#include <framework/core/PathElement.h>
#include <framework/core/CondParser.h>
#include <framework/logging/LogConfig.h>

#include <boost/shared_ptr.hpp>

#include <list>
#include <string>
#include <set>

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
  typedef boost::shared_ptr<Module> ModulePtr;


  /**
   * Base class for Modules.
   *
   * A module is the smallest building block of the framework.
   * A typical event processing chain consists of a Path containing
   * modules. By inheriting from this base class, various types of
   * modules can be created.
   *
   * Each module is identified by its unique name, and should end in ...Module.
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
   * https://belle2.cc.kek.jp/~twiki/bin/view/Software/Basf2manual#Module_Development
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
    };

    /** Different options for behaviour _after_ a conditional path was executed. */
    enum class EAfterConditionPath {
      c_End, /**< End current event after the conditional path. */
      c_Continue, /**< After the conditional path, resume execution after this module. */
    };

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
     * Destructor.
     *
     * Use the destructor to release the memory you allocated in the constructor.
     */
    virtual ~Module();

    /**
     * Initialize the Module.
     *
     * This method is called only once before the actual event processing starts.
     * Use this method to initialize variables, open files etc.
     *
     * This method has to be implemented by subclasses.
     */
    virtual void initialize() {};

    /**
     * Called when entering a new run.
     *
     * Called at the beginning of each run, the method gives you the chance to
     * change run dependent constants like alignment parameters, etc.
     *
     * This method has to be implemented by subclasses.
     */
    virtual void beginRun() {};

    /**
     * This method is the core of the module.
     *
     * This method is called for each event. All processing of the event has to
     * take place in this method.
     *
     * This method has to be implemented by subclasses.
     */
    virtual void event() {};

    /**
     * This method is called if the current run ends.
     *
     * Use this method to store information, which should be aggregated over one run.
     *
     * This method has to be implemented by subclasses.
     */
    virtual void endRun() {};

    /**
     * This method is called at the end of the event processing.
     *
     * This method is called only once after the event processing finished.
     * Use this method for cleaning up, closing files, etc.
     *
     * This method has to be implemented by subclasses.
     */
    virtual void terminate() {};

    /**
     * Returns the name of the module.
     *
     * @return The name of the module as string.
     */
    const std::string& getName() const {return m_name;}

    /**
     * Returns the package this module is in.
     */
    const std::string& getPackage() const {return m_package;}


    /**
     * Returns the description of the module.
     *
     * @return The description of the module as string.
     */
    const std::string& getDescription() const {return m_description;}

    /**
     * Returns the log system configuration.
     *
     * @return The log system configuration.
     */
    LogConfig& getLogConfig() {return m_logConfig;}

    /**
     * Set the log system configuration.
     *
     * @param logConfig The log system configuration.
     */
    void setLogConfig(const LogConfig& logConfig) {m_logConfig = logConfig;}

    /**
     * Configure the log level.
     *
     * @param logLevel The log level.
     */
    void setLogLevel(int logLevel);

    /**
     * Configure the debug messaging level.
     *
     * @param debugLevel The debug level.
     */
    void setDebugLevel(int debugLevel);

    /**
     * Configure the abort log level.
     *
     * @param abortLevel The abort log level.
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
     * Sets the condition path of the module
     *
     * @param path
     */
    void setConditionPath(const boost::shared_ptr<Path>& path) { m_conditionPath = path; };


    /**
     * Sets the condition of the module.
     *
     * See https://belle2.cc.kek.jp/~twiki/bin/view/Software/ModCondTut or CondParser::parseCondition() for a description of the syntax.
     *
     * Please be careful: Avoid creating cyclic paths, e.g. by linking a condition
     * to a path which is processed before the path where this module is
     * located in.
     *
     * @param expression The expression of the condition.
     * @param path       Shared pointer to the Path which will be executed if the condition is evaluated to true.
     * @param afterConditionPath  What to do after executing 'path'.
     */
    void if_value(const std::string& expression, boost::shared_ptr<Path> path, EAfterConditionPath afterConditionPath = EAfterConditionPath::c_End);

    /**
     * A simplified version to set the condition of the module.
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
    void if_false(boost::shared_ptr<Path> path, EAfterConditionPath afterConditionPath = EAfterConditionPath::c_End);

    /**
     * A simplified version to set the condition of the module.
     *
     * Please be careful: Avoid creating cyclic paths, e.g. by linking a condition
     * to a path which is processed before the path where this module is
     * located in.
     *
     * It is equivalent to the if_value() method, using the expression ">=1".
     * This method is meant to be used together with the setReturnValue(bool value) method.
     *
     * @param path Shared pointer to the Path which will be executed if the return value is _false_.
     * @param afterConditionPath  What to do after executing 'path'.
     */
    void if_true(boost::shared_ptr<Path> path, EAfterConditionPath afterConditionPath = EAfterConditionPath::c_End);

    /**
     * Returns true if a condition was set for the module.
     */
    bool hasCondition() const { return m_hasCondition; };

    /**
     * If a condition was set, it is evaluated and the result is returned.
     *
     * If no condition or result value was defined, the method returns false.
     * Otherwise, the condition is evaluated and the result of the evaluation returned.
     * To speed up the evaluation, the condition string was already parsed in the method if_value().
     *
     * @return True if a condition and return value exists and the condition expression was evaluated to true.
     */
    bool evalCondition();

    /**
     * Returns the path of the condition.
     *
     * @return The path of the condition.
     */
    const boost::shared_ptr<Path>& getConditionPath() const {return m_conditionPath; };


    /** What to do after a conditional path is finished. */
    EAfterConditionPath getAfterConditionPath() const { return m_afterConditionPath; }

    /**
     * Returns true if all specified property flags are available in this module.
     *
     * @param propertyFlags The flags which should be compared with the module flags.
     * @return True if all specified property flags are available in this module.
     */
    bool hasProperties(unsigned int propertyFlags) const;

    /**
     * Returns true if the module has still unset parameters which the user has to set in the steering file.
     *
     * @return True if the module has still unset parameters which the user has to set in the steering file.
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
    ModuleParam<T>& getParam(const std::string& name) const throw(ModuleParamList::ModuleParameterNotFoundError, ModuleParamList::ModuleParameterTypeError);


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
    boost::python::list* getParamInfoListPython() const;

    /**
     * Exposes methods of the Module class to Python.
     */
    static void exposePythonAPI();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


  protected:

    /**
     * Sets the flags for the module properties.
     *
     * @param propertyFlags bitwise OR of EModulePropFlags
     */
    void setPropertyFlags(unsigned int propertyFlags);

    /**
     * Sets the description of the module.
     *
     * @param description A description of the module.
     */
    void setDescription(const std::string& description);

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

    /**
     * Set the name of the module just for internal use.
     *
     * \note
     * The module name is set when using the REG_MODULE macro,
     * there is usually no need to call this function directly.
     *
     * @param name The name of the module
     */
    void setModuleName(const std::string& name) { m_name = name; };

    /** Replace existing parameter list. */
    void setParamList(const ModuleParamList& params) { m_moduleParamList = params; }


  private:
    /** no submodules. */
    std::list<ModulePtr> getModules() const { return std::list<ModulePtr>(); }

    std::string m_name;           /**< The name of the module, saved as a string. */
    std::string m_package;        /**< Package this module is found in (may be empty). */
    std::string m_description;    /**< The description of the module. */
    unsigned int m_propertyFlags; /**< The properties of the module as bitwise or (with |) of EModulePropFlags. */

    LogConfig m_logConfig;        /**< The log system configuration of the module. */

    ModuleParamList m_moduleParamList; /**< List storing and managing all parameter of the module. */

    bool m_hasReturnValue;     /**< True, if the return value is set. */
    int  m_returnValue;        /**< The return value. */

    bool m_hasCondition;     /**< True, if a condition was set for the module. */
    boost::shared_ptr<Path> m_conditionPath; /**< The path which which will be executed if the condition is evaluated to true. */
    Belle2::CondParser::EConditionOperators m_conditionOperator;  /**< The operator of the condition (set by parsing the condition expression). */
    int m_conditionValue;                    /**< Numeric value used in the condition (set by parsing the condition expression). */
    EAfterConditionPath m_afterConditionPath; /**< What to do after a conditional path is finished. */

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //                    Python API
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


    /** return the module name. */
    virtual std::string getPathString() const { return getName(); }

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

    template<class U> friend class ModuleProxy;
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
  ModuleParam<T>& Module::getParam(const std::string& name) const throw(ModuleParamList::ModuleParameterNotFoundError, ModuleParamList::ModuleParameterTypeError)
  {
    return m_moduleParamList.getParameter<T>(name);
  }


  //------------------------------------------------------
  //             Define convenient typdefs
  //------------------------------------------------------

  /**
   * Class that defines the < comparison operator ModulePtrs. Used to declare a set of ModulePtrs.
   */
  struct ModulePtrOperators {
    //! Comparison operator for two ModulePtr.
    bool operator()(const ModulePtr& a, const ModulePtr& b) {
      return a.get() < b.get();
    }
  };

  /**
   * Class that defines the equality operator for ModulePtrs.
   */
  struct ModulePtrOperatorsEq: public std::binary_function<ModulePtr, ModulePtr, bool> {
    //! Equality operator for two ModulePtr.
    bool operator()(const ModulePtr& a, const ModulePtr& b) const {
      return a.get() == b.get();
    }
  };

  /** Defines a std::set of shared module pointers.*/
  typedef std::set<ModulePtr, ModulePtrOperators> ModulePtrSet;
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
     * @param moduleName The type name of the module.
     */
    ModuleProxyBase(const std::string& moduleName, const std::string& package);

    /**
     * The destructor of the ModuleProxyBase class.
     */
    virtual ~ModuleProxyBase() {};

    /**
     * Abstract method which creates a new module and returns a shared pointer to it.
     * Instances of modules can only be created by this method.
     * @return A shared pointer to the created module instance.
     */
    virtual ModulePtr createModule() const = 0;

    /**
     * Returns the module name of the module associated to this proxy.
     * @return The module name of the module associated to this proxy.
     */
    const std::string& getModuleName() const {return m_moduleName; }


  protected:

    std::string m_moduleName; /**< The type name of the module. (without trailing "Module") */
    std::string m_package; /**< Package this module is found in (may be empty). */
  };


  /**
   * The templated proxy class.
   * Be defining a global variable of this class, any module can be registered to the ModuleManager.
   * For a module definition of MyTestModule, you should add "REG_MODULE(MyTest)" to the .cc file to register
   * the module.
   */
  template <class T>
  class ModuleProxy : public ModuleProxyBase {

  public:

    /**
     * The constructor of the ModuleProxy class.
     * Calls the constructor of the base class.
     * @param moduleName The type name of the module.
     */
    ModuleProxy(const std::string& moduleName, const std::string& package = "") : ModuleProxyBase(moduleName, package) {};

    /**
     * The destructor of the ModuleProxy class.
     */
    virtual ~ModuleProxy() {};

    /**
     * Creates a new module and returns a shared pointer to it.
     * Instances of modules can only be created by this method.
     * @return A shared pointer to the created module instance.
     */
    ModulePtr createModule() const {
      ModulePtr nm(new T());
      nm->m_name = m_moduleName;
      nm->m_package = m_package;
      return nm;
    }
  };


  //------------------------------------------------------
  //             Define convenient macros
  //------------------------------------------------------
#ifdef _PACKAGE_
  /** Register the given module (without 'Module' suffix) with the framework. */
#define REG_MODULE(moduleName) ModuleProxy<moduleName##Module> regProxy##moduleName(#moduleName, _PACKAGE_);
#else
  /** Register the given module (without 'Module' suffix) with the framework. */
#define REG_MODULE(moduleName) ModuleProxy<moduleName##Module> regProxy##moduleName(#moduleName);
#endif

  //-------------------------------

} // end namespace Belle2

#endif // MODULE_H

