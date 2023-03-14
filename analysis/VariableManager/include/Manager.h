/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <framework/logging/Logger.h>

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <memory>
#include <variant>
#include <type_traits>

namespace Belle2 {
  class Particle;

  namespace Variable {
    /** Global list of available variables.
     *
     *  Each variable has an associated unique string key through
     *  which it can be accessed. The Manager can be used to get
     *  an instance of the Variable class for this key. All ordinary variables take a
     *  const Particle* and return a double.
     *
     *
     *  <h2>List of available variables</h2>
     *  To simply get a list of all variables registered in the main analysis library,
     *  run <tt>basf2 analysis/scripts/variables.py</tt> .
     *
     *  <h2>Parameter variables</h2>
     *  Parameter variables take a const Particle* and a const std::vector<double>&. They return a double.
     *  If a Parameter Variable is accessed via the VariableManager::getVariable a ordinary variable is
     *  dynamically registered which calls the Parameter variable with the correct parameters.

     *  <h2>Meta variables</h2>
     *  Meta variables take a const std::vector<std::string>& and return a pointer to a ordinary variable function.
     *  If a Meta Variable is accessed via the VariableManager::getVariable a ordinary variable is
     *  dynamically registered which calls the function returned by the meta variable.
     *
     *  There are a number of meta-variables that can be combined with existing variables to do more powerful calculations:
     *  <dl>
     *  <dt>abs(varName)</dt>
     *   <dd>Return absolute value of variable varName.</dd>
     *  <dt>daughter(N, varName)</dt>
     *   <dd>Calculate variable varName for Nth daughter</dd>
     *  <dt>daughterProductOf(varName)</dt>
     *   <dd>Calculate variable varName for each daughter, and return product of values</dd>
     *  <dt>daughterSumOf(varName)</dt>
     *   <dd>Calculate variable varName for each daughter, and return sum of values</dd>
     *  <dt>extraInfo(extraInfoName)</dt>
     *   <dd>Return value of extra info 'extraInfoName' from Particle (see Particle::getExtraInfo()).</dd>
     *  </dl>
     *  So, if you wanted to get the momentum of the first daughter, you can use "daughter(1, p)" anywhere where variable
     *  names are accepted. Nesting is also possible, e.g. "daughter(1, abs(extraInfo(SignalProbability)))" can be used.
     *
     *  <h2>Adding variables</h2>
     *  Variables will automatically register themselves when
     *  the library that contains them is loaded. They only need to be put in a .cc file,
     *  with a call to REGISTER_VARIABLE() in the global scope to set a name and description.
     *  Newly registered variables will be added to the group defined by the last call to VARIABLE_GROUP(),
     *  please search the list of variables (see 'List of available variables' section) for a matching group before adding your own.
     *
     *  Example:
        \code
        double particleFlavorType(const Particle* part)
        {
          return part->getFlavorType();
        }
        REGISTER_VARIABLE("flavor", particleFlavorType, "flavor type of decay (0=unflavored, 1=flavored)");
        \endcode
     *
     *  This registers the variable "flavor" with the given function and description.
     *  Note that only alphanumeric characters (0-9, a-Z) plus '_' are permitted in variable names.
     *
     *  Variables can then be accessed using getVariable(name) and getVariables().
     *
     *
     *  <h2>Python interface</h2>
     *  This class is exported to Python, and can be used to use variables in Python basf2 modules:
        \code
        from variables import variables

        from ROOT.Math import PxPyPzEVector
        someParticle = Belle2.Particle(PxPyPzEVector(1.0, 0, 0, 1.0), 321)
        print(variables.evaluate('E', someParticle))
        \endcode
     *
     */
    class Manager {
      /**
       * NOTE: the python interface is documented manually in analysis/doc/Variables.rst
       * (because we use ROOT to expose this in python rather than boost::python).
       *
       * Please also keep that up-to-date with any modifications.
       */

    public:
      /** Typedef for variable return type, can either be double, int or bool in std::variant */
      typedef std::variant<double, int, bool> VarVariant;
      /** functions stored take a const Particle* and return VarVariant. */
      typedef std::function<VarVariant(const Particle*)> FunctionPtr;
      /** parameter functions stored take a const Particle*, const std::vector<double>& and return VarVariant. */
      typedef std::function<VarVariant(const Particle*, const std::vector<double>&)> ParameterFunctionPtr;
      /** meta functions stored take a const std::vector<std::string>& and return a FunctionPtr. */
      typedef std::function<FunctionPtr(const std::vector<std::string>&)> MetaFunctionPtr;
      /** Typedef for the cut, that we use Particles as our base objects. */
      typedef Particle Object;

      /** data type of variables */
      enum VariableDataType {
        c_double = 0,
        c_int    = 1,
        c_bool   = 2
      };

      /** Base class for information common to all types of variables. */
      struct VarBase {
        std::string name; /**< Unique identifier of the function, used as key. */
        std::string description; /**< Description of what this function does. */
        std::string group; /**< Associated group. */
        VariableDataType variabletype; /**< data type of variable */
        /** ctor */
        VarBase(const std::string& n, const std::string& d, const std::string& g, const VariableDataType& v)
          : name(n), description(d), group(g), variabletype(v) { }

        /** function to extend the description of the variable */
        void extendDescriptionString(const std::string& d)
        {
          description.append(d);
        }
      };

      /** A variable returning a floating-point value for a given Particle. */
      struct Var : public VarBase {
        FunctionPtr function; /**< Pointer to function. */
        /** ctor */
        Var(const std::string& n, FunctionPtr f, const std::string& d, const std::string& g = "",
            const VariableDataType& v = VariableDataType::c_double)
          : VarBase(n, d, g, v), function(f) { }
      };

      /** A variable taking additional floating-point arguments to influence the behaviour. */
      struct ParameterVar : public VarBase {
        ParameterFunctionPtr function; /**< Pointer to function. */
        /** ctor */
        ParameterVar(const std::string& n, ParameterFunctionPtr f, const std::string& d, const std::string& g = "",
                     const VariableDataType& v = VariableDataType::c_double)
          : VarBase(n, d, g, v), function(f) { }
      };

      /** A variable taking string arguments returning a variable. */
      struct MetaVar : public VarBase {
        MetaFunctionPtr function; /**< Pointer to function. */
        /** ctor */
        explicit MetaVar(const std::string& n, MetaFunctionPtr f, const std::string& d, const std::string& g = "",
                         const VariableDataType& v = VariableDataType::c_double)
          : VarBase(n, d, g, v), function(f) { }
      };

      /** get singleton instance. */
      static Manager& Instance();

      /** Get the variable belonging to the given key.
       *
       * Returns NULL if name not found.
       */
      const Var* getVariable(std::string name);

      /** Get Meta and Parameter Variables for a function call which is already parsed into name and arguments
       *
       * Used by FunctionNode because there we already have the MetaVariable in parsed form
       *
       * Returns NULL if name not found
       */
      const Var* getVariable(const std::string& functionName, const std::vector<std::string>& functionArguments);

      /**
       * Get variables belonging to the given keys
       */
      std::vector<const Belle2::Variable::Manager::Var*> getVariables(const std::vector<std::string>& variables);

      /** Add alias
       * Return true if the alias was successfully added
       */
      bool addAlias(const std::string& alias, const std::string& variable);

      /**
       * Print existing aliases
       */
      void printAliases();

      /**
       * Clear existing aliases
       */
      void clearAliases();

      /** Resolve alias
       * Return original variable name
       */
      std::string resolveAlias(const std::string& alias);

      /** Add collection
       * Return true if the collection was successfully added
       */
      bool addCollection(const std::string& collection, const std::vector<std::string>& variables);

      /** Get Collection
       * Returns variable names corresponding to the given collection
       */
      std::vector<std::string> getCollection(const std::string& collection);

      /** Resolve Collection
       * Returns variable names corresponding to the given collection or if it is not a collection just the variable itself
       */
      std::vector<std::string> resolveCollections(const std::vector<std::string>& variables);

      /** Return list of all variables (in order registered). */
      std::vector<const Belle2::Variable::Manager::VarBase*> getVariables() const { return m_variablesInRegistrationOrder; }


      /** All variables registered after VARIABLE_GROUP(groupName) will be added to this group. */
      void setVariableGroup(const std::string& groupName);

      /** Register a variable. */
      void registerVariable(const std::string& name, const Manager::FunctionPtr& f, const std::string& description,
                            const Manager::VariableDataType& v, const std::string& unit = "");
      /** Register a variable that takes floating-point arguments (see Variable::Manager::ParameterFunctionPtr). */
      void registerVariable(const std::string& name, const Manager::ParameterFunctionPtr& f, const std::string& description,
                            const Manager::VariableDataType& v, const std::string& unit = "");
      /** Register a meta-variable that takes string arguments and returns a variable(see Variable::Manager::MetaFunctionPtr). */
      void registerVariable(const std::string& name, const Manager::MetaFunctionPtr& f, const std::string& description,
                            const Manager::VariableDataType& v);
      /** Make a variable deprecated. */
      void deprecateVariable(const std::string& name, bool make_fatal, const std::string& version, const std::string& description);

      /**
       * Check if a variable is deprecated.
       * If it is marked as deprecated a warning is issued or a B2FATAL aborts the event loop.
       * Which action is taken depends on the depreciation setting.
       */
      void checkDeprecatedVariable(const std::string& name);

      /** evaluate variable 'varName' on given Particle.
       *
       * Mainly provided for the Python interface. For performance critical code, it is recommended to use getVariable() once and keep the Var* pointer around.
       *
       * Throws exception if variable isn't found. Assumes 'p' is != NULL.
       */
      double evaluate(const std::string& varName, const Particle* p);

      /** Return list of all variable names (in order registered). */
      std::vector<std::string> getNames() const;

      /** Return a list of all variable alias names (in reverse order added). */
      std::vector<std::string> getAliasNames() const;

      /** Abort with B2FATAL if name is not a valid name for a variable. */
      void assertValidName(const std::string& name);

    private:
      Manager() {};
      /** Copy constructor disabled (not defined). */
      Manager(const Manager&);
      ~Manager();

      /** Creates and registers a concrete variable (Var) from a MetaVar, ParameterVar or numeric constant. */
      bool createVariable(const std::string& name);

      /** Creates and registers a MetaVar, ParameterVar
        * Called by the corresponding getVariable(const std::string&, const std::vector<std::string>&) to register non-existing variables
        * The fullname of the function call is also passed for alias resolving.
        */
      bool createVariable(const std::string& fullname, const std::string& functionName,
                          const std::vector<std::string>& functionArguments);

      /** Group last set via VARIABLE_GROUP(). */
      std::string m_currentGroup;

      /** List of variables in registration order. */
      std::vector<const VarBase*> m_variablesInRegistrationOrder;

      /** List of registered aliases. */
      std::map<std::string, std::string> m_alias;
      /** List of registered collections. */
      std::map<std::string, std::vector<std::string>> m_collection;
      /** List of registered variables. */
      std::map<std::string, std::shared_ptr<Var>> m_variables;
      /** List of registered parameter variables. */
      std::map<std::string, std::shared_ptr<ParameterVar>> m_parameter_variables;
      /** List of registered meta variables. */
      std::map<std::string, std::shared_ptr<MetaVar>> m_meta_variables;
      /** List of deprecated variables. */
      std::map<std::string, std::pair<bool, std::string>> m_deprecated;
    };

    /** Internal class that registers a variable with Manager when constructed. */
    class Proxy {
    public:
      /** constructor. */
      Proxy(const std::string& name, Manager::FunctionPtr f, const std::string& description, Manager::VariableDataType v,
            const std::string& unit = "")
      {
        Manager::Instance().registerVariable(name, f, description, v, unit);
      }
      /** constructor. */
      Proxy(const std::string& name, Manager::ParameterFunctionPtr f, const std::string& description, Manager::VariableDataType v,
            const std::string& unit = "")
      {
        Manager::Instance().registerVariable(name, f, description, v, unit);
      }
      /** constructor. */
      Proxy(const std::string& name, Manager::MetaFunctionPtr f, const std::string& description, Manager::VariableDataType v)
      {
        Manager::Instance().registerVariable(name, f, description, v);
      }
    };

    /** Internal class that registers a variable group with Manager when constructed. */
    class GroupProxy {
    public:
      /** constructor. */
      explicit GroupProxy(const std::string& groupName)
      {
        Manager::Instance().setVariableGroup(groupName);
      }
    };

    /** Internal class that registers a variable as deprecated. */
    class DeprecateProxy {
    public:
      /** constructor. */
      DeprecateProxy(const std::string& name, bool make_fatal, const std::string& version, const std::string& description)
      {
        Manager::Instance().deprecateVariable(name, make_fatal, version, description);
      }
    };

    template<typename T>
    std::function<T> make_function(T* t)
    {
      return { t };
    }

    template<typename T>
    Belle2::Variable::Manager::VariableDataType get_function_type(const std::string& name, T* t)
    {
      auto func =  std::function{t};
      using ReturnType = typename decltype(func)::result_type;
      if (std::is_same_v<ReturnType, double>) {
        return Belle2::Variable::Manager::VariableDataType::c_double;
      } else if (std::is_same_v<ReturnType, int>) {
        return Belle2::Variable::Manager::VariableDataType::c_int;
      } else if (std::is_same_v<ReturnType, bool>) {
        return Belle2::Variable::Manager::VariableDataType::c_bool;
      } else {
        B2FATAL("Metavariables must be registered using the REGISTER_METAVARIABLE macro." << LogVar("Variablename", name));
      }
    }

    /** \def VARMANAGER_CONCATENATE_DETAIL(x, y)
     * Internal macro to generate unique name.
     */
#define VARMANAGER_CONCATENATE_DETAIL(x, y) x##y
    /** \def VARMANAGER_CONCATENATE(x, y)
     * Internal macro to generate unique name.
     */
#define VARMANAGER_CONCATENATE(x, y) VARMANAGER_CONCATENATE_DETAIL(x, y)
    /** \def VARMANAGER_MAKE_UNIQUE(x)
     * Internal macro to generate unique name.
     */
#define VARMANAGER_MAKE_UNIQUE(x) VARMANAGER_CONCATENATE(x, __LINE__)

    /** \def REGISTER_VARIABLE_NO_UNIT(name, function, description)
     *
     * Register a variable under the key 'name' with given function and description.
     * \sa Manager
     */
#define REGISTER_VARIABLE_NO_UNIT(name, function, description) \
  static Proxy VARMANAGER_MAKE_UNIQUE(_variableproxy)(std::string(name), Belle2::Variable::make_function(function), std::string(description), Belle2::Variable::get_function_type(name,function));

    /** \def REGISTER_VARIABLE_WITH_UNIT(name, function, description, unit)
     *
     * Register a variable under the key 'name' with given function, description, and unit.
     * \sa Manager
     */
#define REGISTER_VARIABLE_WITH_UNIT(name, function, description, unit) \
  static Proxy VARMANAGER_MAKE_UNIQUE(_variableproxy)(std::string(name), Belle2::Variable::make_function(function), std::string(description), Belle2::Variable::get_function_type(name,function), std::string(unit));

    /** \def PICK_FIFTH_ARG(arg1, arg2, arg3, arg4, arg5, ...)
     *
     * Helper macro to determine number of provided arguments.
     *
     */
#define PICK_FIFTH_ARG(arg1, arg2, arg3, arg4, arg5, ...) arg5

    /** \def REGISTER_VARIABLE_MACRO_CHOOSER(...)
     *
     * Helper macro to pick REGISTER_VARIABLE function based on number of provided arguments
     *
     */
#define REGISTER_VARIABLE_MACRO_CHOOSER(...) PICK_FIFTH_ARG(__VA_ARGS__, REGISTER_VARIABLE_WITH_UNIT, REGISTER_VARIABLE_NO_UNIT, )

    /** \def REGISTER_VARIABLE(...)
     *
     * Generic REGISTER_VARIABLE function
     *
     */
#define REGISTER_VARIABLE(...) REGISTER_VARIABLE_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

    /** \def REGISTER_METAVARIABLE(name, function, description, variabledatatype)
     *
     * Register a variable under the key 'name' with given function and description.
     * \sa Manager
     */
#define REGISTER_METAVARIABLE(name, function, description, variabledatatype) \
  static Proxy VARMANAGER_MAKE_UNIQUE(_variableproxy)(std::string(name), Belle2::Variable::make_function(function), std::string(description), Belle2::Variable::Manager::VariableDataType(variabledatatype));

    /** \def VARIABLE_GROUP(groupName)
     *
     * All variables registered after this will be added to this group, which mainly affects the output when printing the variable list.
     * \sa Manager
     */
#define VARIABLE_GROUP(groupName) \
  static GroupProxy VARMANAGER_MAKE_UNIQUE(_variablegroupproxy)(groupName);

  }
  /** \def MAKE_DEPRECATED(name, make_fatal, version, description)
   *
   * Registers a variable as deprecated
   */
#define MAKE_DEPRECATED(name, make_fatal, version, description) \
  static DeprecateProxy VARMANAGER_MAKE_UNIQUE(_deprecateproxy)(std::string(name),  bool(make_fatal), std::string(version), std::string(description));
}
