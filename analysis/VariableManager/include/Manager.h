#pragma once

#include <string>
#include <map>
#include <vector>
#include <functional>

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
        from variables import *
        # for convenience, a Manager instance is already created (called 'variables')

        from ROOT import TLorentzVector
        someParticle = Belle2.Particle(TLorentzVector(1.0, 0, 0, 0), 321)
        print variables.evaluate('E', someParticle)
        \endcode
     *
     *
     *  \note You should probably also update this page if you add a useful function
     *        https://belle2.cc.kek.jp/~twiki/bin/view/Physics/ParticleSelectorFunctions
     */
    class Manager {

    public:
#ifndef __CINT__
      /** functions stored take a const Particle* and return double. */
      typedef std::function<double(const Particle*)> FunctionPtr;
      /** parameter functions stored take a const Particle*, const std::vector<double>& and return double. */
      typedef std::function<double(const Particle*, const std::vector<double>&)> ParameterFunctionPtr;
      /** meta functions stored take a const std::vector<std::string>& and return a FunctionPtr. */
      typedef std::function<FunctionPtr(const std::vector<std::string>&)> MetaFunctionPtr;
#endif

      /** A variable returning a floating-point value for a given Particle. */
      struct Var {
        std::string name; /**< Unique identifier of the function, used as key. */
#ifndef __CINT__
        FunctionPtr function; /**< Pointer to function. */
#endif
        std::string description; /**< Description of what this function does. */
        std::string group; /**< Associated group. */
#if defined(__CINT__) || defined(__ROOTCLING__) || defined(R__DICTIONARY_FILENAME)
        Var() {} /**< default constructor for ROOT */
#else
        /** ctor */
        Var(std::string n, FunctionPtr f, std::string d, std::string g = "") : name(n), function(f), description(d), group(g) { }
#endif
      };

      /** A variable taking additional floating-point arguments to influence the behaviour. */
      struct ParameterVar {
        std::string name; /**< Unique identifier of the function, used as key. */
#ifndef __CINT__
        ParameterFunctionPtr function; /**< Pointer to function. */
#endif
        std::string description; /**< Description of what this function does. */
        std::string group; /**< Associated group. */
#if defined(__CINT__) || defined(__ROOTCLING__) || defined(R__DICTIONARY_FILENAME)
        ParameterVar() {} /**< default constructor for ROOT */
#else
        /** ctor */
        ParameterVar(std::string n, ParameterFunctionPtr f, std::string d, std::string g = "") : name(n), function(f), description(d), group(g) { }
#endif
      };

      /** A variable taking string arguments returning a variable. */
      struct MetaVar {
        std::string name; /**< Unique identifier of the function, used as key. */
#ifndef __CINT__
        MetaFunctionPtr function; /**< Pointer to function. */
#endif
        std::string description; /**< Description of what this function does. */
        std::string group; /**< Associated group. */
#if defined(__CINT__) || defined(__ROOTCLING__) || defined(R__DICTIONARY_FILENAME)
        MetaVar() {} /**< default constructor for ROOT */
#else
        /** ctor */
        MetaVar(std::string n, MetaFunctionPtr f, std::string d, std::string g = "") : name(n), function(f), description(d), group(g) { }
#endif
      };

      /** get singleton instance. */
      static Manager& Instance();

      /** Get the variable belonging to the given key.
       *
       * Returns NULL if name not found.
       */
      const Var* getVariable(const std::string& name);

#ifndef __CINT__
      /** Return list of all variables (in order registered). */
      std::vector<const Var*> getVariables() const { return m_variablesInRegistrationOrder; }

      /** All variables registered after VARIABLE_GROUP(groupName) will be added to this group. */
      void setVariableGroup(const std::string& groupName);

      /** Register a variable. */
      void registerVariable(const std::string& name, Manager::FunctionPtr f, const std::string& description);
      /** Register a variable that takes floating-point arguments (see Variable::Manager::ParameterFunctionPtr). */
      void registerVariable(const std::string& name, Manager::ParameterFunctionPtr f, const std::string& description);
      /** Register a meta-variable that takes string arguments and returns a variable(see Variable::Manager::MetaFunctionPtr). */
      void registerVariable(const std::string& name, Manager::MetaFunctionPtr f, const std::string& description);

      /** Creates and registers a variable of the form func(varname) */
      bool createVariable(const std::string& name);
#endif

      /** evaluate variable 'varName' on given Particle.
       *
       * Mainly provided for the Python interface. For performance critical code, it is recommended to use getVariable() once and keep the Var* pointer around.
       *
       * Aborts with B2FATAL if variable isn't found. Assumes 'p' is != NULL.
       */
      double evaluate(const std::string& varName, const Particle* p);

      /** Return list of all variable names (in order registered). */
      std::vector<std::string> getNames() const;

      /** Print list of all variables with description (in order registered). */
      void printList() const;

    private:
      Manager() {};
      /** Copy constructor disabled (not defined). */
      Manager(const Manager&);
      ~Manager();

      /** Group last set via VARIABLE_GROUP(). */
      std::string m_currentGroup;
#ifndef __CINT__
      /** List of registered variables. */
      std::map<std::string, Var*> m_variables;
      /** List of registered variables. */
      std::map<std::string, ParameterVar*> m_parameter_variables;
      /** List of registered variables. */
      std::map<std::string, MetaVar*> m_meta_variables;

      /** List of variables in registration order. */
      std::vector<const Var*> m_variablesInRegistrationOrder;
#endif
    };

#ifndef __CINT__
    /** Internal class that registers a variable with Manager when constructed. */
    class Proxy {
    public:
      /** constructor. */
      Proxy(const std::string& name, Manager::FunctionPtr f, const std::string& description) {
        Manager::Instance().registerVariable(name, f, description);
      }
      /** constructor. */
      Proxy(const std::string& name, Manager::ParameterFunctionPtr f, const std::string& description) {
        Manager::Instance().registerVariable(name, f, description);
      }
      /** constructor. */
      Proxy(const std::string& name, Manager::MetaFunctionPtr f, const std::string& description) {
        Manager::Instance().registerVariable(name, f, description);
      }
    };

    /** Internal class that registers a variable group with Manager when constructed. */
    class GroupProxy {
    public:
      /** constructor. */
      GroupProxy(const std::string& groupName) {
        Manager::Instance().setVariableGroup(groupName);
      }
    };

    template<typename T>
    std::function<T> make_function(T* t)
    {
      return { t };
    }
#endif


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

    /** \def REGISTER_VARIABLE(name, function, description)
     *
     * Register a variable under the key 'name' with given function and description.
     * \sa Manager
     */
#define REGISTER_VARIABLE(name, function, description) \
  static Proxy VARMANAGER_MAKE_UNIQUE(_variableproxy)(std::string(name), Belle2::Variable::make_function(function), std::string(description));

    /** \def VARIABLE_GROUP(groupName)
     *
     * All variables registered after this will be added to this group, which mainly affects the output when printing the variable list.
     * \sa Manager
     */
#define VARIABLE_GROUP(groupName) \
  static GroupProxy VARMANAGER_MAKE_UNIQUE(_variablegroupproxy)(groupName);

  }
}
