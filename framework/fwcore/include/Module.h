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

#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/python/list.hpp>
#include <boost/python/dict.hpp>

#include <framework/fwcore/CondParser.h>
#include <framework/fwcore/ModuleParamList.h>
#include <framework/logging/Logger.h>

#include <vector>
#include <list>
#include <string>
#include <set>
#include <map>

namespace Belle2 {

  class Path;

  //! Base Class for Modules
  /*! You have to inherit from this class to run over events.
  */
  class Module {

  public:

    //! The process record types
    enum EProcessRecordType {
      prt_Event,       /*!< The default value: Marks event data */
      prt_BeginRun,    /*!< Marks the beginning of a new run (only used for the first method in the chain) */
      prt_EndRun,      /*!< Marks the end of a new run (only used for the first method in the chain) */
      prt_EndOfData    /*!< Marks the end of the data. This will stop the
                          processing of the events. This is especially useful
                          for input modules which run out of events before the
                          maximum number of events is reached. The processing of
                          the events is stopped immediately after the module
                          setting this EProcessRecordType has finished.*/
    };

    //! Each module can be tagged with property flags, which indicate certain features of the module.
    enum EModulePropFlags {
      c_TriggersNewRun          = 1,   /*!< This module is able to trigger new runs. */
      c_TriggersEndOfData       = 2,   /*!< This module is able to send the message that there is no more data available. */
      c_ReadsDataSingleProcess  = 4,   /*!< This module is able to read data from a single data stream (disk/server). */
      c_ReadsDataMultiProcess   = 8,   /*!< This module is able to read data from an event streaming server. */
      c_WritesDataSingleProcess = 16,  /*!< This module is able to write data into a single data stream (disk/server). */
      c_WritesDataMultiProcess  = 32,  /*!< This module is able to write data to an event streaming server. */
      c_RequiresSingleProcess   = 64,  /*!< This module requires the framework to run in single processing mode. */
      c_RequiresGUISupport      = 128  /*!< This module requires the framework to have GUI support built-in. */
    };

    //! Constructor
    /*!
      Create and allocate memory for variables here. Add the module parameters in this method.
      \param type The type of the Module (is saved as a string)
    */
    Module(const std::string& type);

    //! Destructor
    /*!
       Use the destructor to release the memory you allocated in the constructor.
    */
    virtual ~Module();

    //! Returns a new instance of the module.
    /*!
        This method has to be implemented by subclasses.
    */
    virtual boost::shared_ptr<Module> newModule() {boost::shared_ptr<Module> nm(new Module("Module")); return nm; };

    //! Initialize the Module
    /*! This method is called only once before the actual event processing starts.
        Use this method to initialize variables, open files etc.

        This method has to be implemented by subclasses.
    */
    virtual void initialize() {};

    //! Called when entering a new run
    /*! Called at the beginning of each run, the method gives you the chance to change run dependent constants like alignment parameters, etc.

        This method has to be implemented by subclasses.
    */
    virtual void beginRun() {};

    //! This method is the core of the module.
    /*! This method is called for each event. All processing of the event has to take place in this method.

        This method has to be implemented by subclasses.
    */
    virtual void event() {};

    //! This method is called if the current run ends.
    /*! Use this method to store information, which should be aggregated over one run.

        This method has to be implemented by subclasses.
    */
    virtual void endRun() {};

    //! This method is called at the end of the event processing.
    /*! This method is called only once after the event processing finished.
        Use this method for cleaning up, closing files, etc.

        This method has to be implemented by subclasses.
    */
    virtual void terminate() {};

    //! Returns the type of the module
    /*!
        \return The type of the module as string
    */
    const std::string& getType() const {return m_type;}

    //! Returns the description of the module
    /*!
        \return The description of the module as string
    */
    const std::string& getDescription() const {return m_description;}

    //! Returns the log level used for this module.
    /*!
        \return Returns the log level of the module.
    */
    LogCommon::ELogLevel getLogLevel() {return static_cast<LogCommon::ELogLevel>(m_logLevel); };

    //! Returns the debug messaging level used for this module.
    /*!
        \return Returns the debug messaging level of the module.
    */
    int getDebugLevel() {return m_debugLevel; };

    //! Sets the condition of the module.
    /*!
        Please be careful: Avoid creating cyclic paths, e.g. by linking a condition
        to a path which is processed before the path where this module is
        located in.

        \param expression The expression of the condition.
        \return Shared pointer to the Path, which will be executed if the condition is evaluated to true.
    */
    void setCondition(const std::string& expression, boost::shared_ptr<Path> path);

    //! A simplified version to set the condition of the module.
    /*!
        Please be careful: Avoid creating cyclic paths, e.g. by linking a condition
        to a path which is processed before the path where this module is
        located in.

        It is equivalent to the setCondition() method, using the expression "<1".
        This method is meant to be used together with the setReturnValue(bool value) method.
        \return Shared pointer to the Path, which will be executed if the condition is evaluated to true.
    */
    void setCondition(boost::shared_ptr<Path> path);

    //! Returns true if a condition and a return value was set for the module.
    /*!
        \return True if a condition and a return value was set for the module.
    */
    bool hasCondition() const { return m_hasCondition; };

    //! If a condition was set, it is evaluated and the result is returned.
    /*!
        If no condition or result value was defined, the method returns false.
        Otherwise, the condition is evaluated and the result of the evaluation returned.
        To speed up the evaluation, the condition string was already parsed in the method setCondition().

        \return True if a condition and return value exists and the condition expression was evaluated to true.
    */
    bool evalCondition();

    //! Returns the path of the condition.
    /*!
        \return The path of the condition.
    */
    boost::shared_ptr<Path> getConditionPath() const {return m_conditionPath; };

    //! Returns true if all specified property flags are available in this module.
    /*!
        \param propertyFlags The flags which should be compared with the module flags.
        \return True if all specified property flags are available in this module.
    */
    bool hasProperties(unsigned int propertyFlags);

    //! Returns the current process record type.
    /*!
        \return The current process record type.
    */
    EProcessRecordType getProcessRecordType() const {return m_processRecordType; };

    //! Sets the flag if the module was registered by the user to the framework.
    /*!
        \param regFramework Set to true if the module was registered by the user to the framework.
    */
    void setRegisteredToFramework(bool regFramework) {m_registeredToFramework = regFramework; };

    //! Returns true if the module was registered by the user to the framework.
    /*!
        \return True if the module was registered by the user to the framework.
    */
    bool isRegisteredToFramework() const {return m_registeredToFramework; };

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

    //! Sets the flags for the module properties.
    /*!
        \param propertyFlags The flags for the module properties.
    */
    void setPropertyFlags(unsigned int propertyFlags);

    //! Sets the description of the module.
    /*!
        \param description A short description of the module.
    */
    void setDescription(const std::string description);

    //! Adds a new parameter to the module. This method has to be called in the constructor of the module.
    /*!
        \param name The unique name of the parameter.
        \param paramVariable The local member variable of the module to which the value from the steering file is written.
        \param defaultValue The default value of the parameter which is used if there was no value given in the steering file.
        \param description Optional: a short description of the parameter.
    */
    template<typename T>
    void addParam(const std::string& name, T& paramVariable, const T& defaultValue, const std::string& description = "");

    //! Sets the return value for this module as integer.
    /*! The value can be used in the steering file to divide the analysis chain
        into several paths.

        \param value The value of the return value
    */
    void setReturnValue(int value);

    //! Sets the return value for this module as bool.
    /*! The bool value is saved as an integer with the convention 1 meaning true and 0 meaning false.
        The value can be used in the steering file to divide the analysis chain
        into several paths.

        \param value The value of the return value
    */
    void setReturnValue(bool value);

    //! Sets the process record type.
    /*! Usually this method is used by data generating or data reading modules.
        Please note: Only for the first module in the chain the BEGIN_RUN and
        END_RUN process record types are checked.

        \param processRecordType The process record type to be returned.
    */
    void setProcessRecordType(EProcessRecordType processRecordType);


  private:

    //! This variable controls the messaging level
    /*! \sa setDebugFlag
    */
    int m_debugFlag;

    std::string m_type;           /*!< The type of the module, saved as a string. */
    std::string m_description;    /*!< The description of the module. */
    unsigned int m_propertyFlags; /*!< The properties of the module (Master, multi processing etc.) saved as bitwise flags. */
    bool m_registeredToFramework; /*!< True if the module was registered by the user to the framework. */

    int m_logLevel;            /*!< The log messaging level of the module. Defined as int for the parameter handling. */
    int m_debugLevel;          /*!< The debug messaging level of the module. */

    ModuleParamList m_moduleParamList; /*!< List storing and managing all parameter of the module. */

    bool m_hasReturnValue;     /*!< True, if the default return value is set. */
    int  m_returnValue;        /*!< The default return value. */
    EProcessRecordType m_processRecordType;  /*!< The current process record type. */

    bool m_hasCondition;     /*!< True, if a condition was set for the module. */
    boost::shared_ptr<Path> m_conditionPath; /*!< The path which which will be executed if the condition is evaluated to true. */
    Belle2::CondParser::EConditionOperators m_conditionOperator;  /*!< The operator of the condition (set by parsing the condition expression). */
    int m_conditionValue;                    /*!< The value of the condition (set by parsing the condition expression). */

    //! Implements a method for setting boost::python objects.
    /*! The method supports the following types: int, double, string, bool
        The conversion of the python object to the C++ type and the final storage of the
        parameter value is done by specializing the template method setParamObjectTemplate().

        \param name The unique name of the parameter.
        \param pyObj The object which should be converted and stored as the parameter value.
    */
    void setParamObject(const std::string& name, const boost::python::object& pyObj);

    //! Implements a method for setting boost::python lists.
    /*! The method supports lists of the following types: int, double, string, bool
        The conversion of the python list to the std::vector and the final storage of the
        parameter value is done by specializing the template method setParamListTemplate().

        \param name The unique name of the parameter.
        \param pyList The list which should be converted to a std::vector and stored as the parameter value.
    */
    void setParamList(const std::string& name, const boost::python::list& pyList);

    //! Implements a method for reading the parameter values from a boost::python dictionary.
    /*! The key of the dictionary has to be the name of the parameter and the value has to
        be of one of the supported parameter types (both, single parameters and lists are allowed).

        \param dictionary The python dictionary from which the parameter values are read.
    */
    void setParamDict(const boost::python::dict& dictionary);

    friend class ModuleManager;
  };


  //------------------------------------------------------
  //       Implementation of template based methods
  //------------------------------------------------------

  template<typename T>
  void Module::addParam(const std::string& name, T& paramVariable, const T& defaultValue, const std::string& description)
  {
    m_moduleParamList.addParameter(name, paramVariable, defaultValue, description);
  }


  //------------------------------------------------------
  //             Define convenient typdefs
  //------------------------------------------------------

  typedef boost::shared_ptr<Module> ModulePtr;

  struct ModulePtrOperators {
    bool operator()(const ModulePtr& a, const ModulePtr& b) {
      return a.get() < b.get();
    }
  };

  struct ModulePtrOperatorsEq: public std::binary_function<ModulePtr, ModulePtr, bool> {
    bool operator()(const ModulePtr& a, const ModulePtr& b) const {
      return a.get() == b.get();
    }
  };

  typedef std::set<ModulePtr, ModulePtrOperators> ModulePtrSet;
  typedef std::list<ModulePtr> ModulePtrList;


  //------------------------------------------------------
  //             Define convenient macros
  //------------------------------------------------------
#define NEW_MODULE(className) virtual ModulePtr newModule() { ModulePtr nm(new className()); return nm; };
#define REG_MODULE(className) ModuleManager::Registrator<className> reg##className;
  //-------------------------------

} // end namespace Belle2

#endif // MODULE_H

