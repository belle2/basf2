#pragma once

#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <memory>

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
        print(variables.evaluate('E', someParticle))
        \endcode
     *
     *
     *  \note You should probably also update this page if you add a useful function
     *        https://confluence.desy.de/display/BI/Physics+ParticleSelectorFunctions
     */
    class Manager {

    public:
      /** functions stored take a const Particle* and return double. */
      typedef std::function<double(const Particle*)> FunctionPtr;
      /** parameter functions stored take a const Particle*, const std::vector<double>& and return double. */
      typedef std::function<double(const Particle*, const std::vector<double>&)> ParameterFunctionPtr;
      /** meta functions stored take a const std::vector<std::string>& and return a FunctionPtr. */
      typedef std::function<FunctionPtr(const std::vector<std::string>&)> MetaFunctionPtr;
      /** Typedef for the cut, that we use Particles as outr base objects. */
      typedef Particle Object;

      /** Base class for information common to all types of variables. */
      struct VarBase {
        std::string name; /**< Unique identifier of the function, used as key. */
        std::string description; /**< Description of what this function does. */
        std::string group; /**< Associated group. */
        /** ctor */
        VarBase(std::string n, std::string d, std::string g) : name(n), description(d), group(g) { }
      };

      /** A variable returning a floating-point value for a given Particle. */
      struct Var : public VarBase {
        FunctionPtr function; /**< Pointer to function. */
        /** ctor */
        Var(std::string n, FunctionPtr f, std::string d, std::string g = "") : VarBase(n, d, g), function(f) { }
      };

      /** A variable taking additional floating-point arguments to influence the behaviour. */
      struct ParameterVar : public VarBase {
        ParameterFunctionPtr function; /**< Pointer to function. */
        /** ctor */
        ParameterVar(std::string n, ParameterFunctionPtr f, std::string d, std::string g = "") : VarBase(n, d, g), function(f) { }
      };

      /** A variable taking string arguments returning a variable. */
      struct MetaVar : public VarBase {
        MetaFunctionPtr function; /**< Pointer to function. */
        /** ctor */
        MetaVar(std::string n, MetaFunctionPtr f, std::string d, std::string g = "") : VarBase(n, d, g), function(f) { }
      };

      /** get singleton instance. */
      static Manager& Instance();

      /** Get the variable belonging to the given key.
       *
       * Returns NULL if name not found.
       */
      const Var* getVariable(std::string name);

      /**
       * Get variables belonging to the given keys
       */
      std::vector<const Belle2::Variable::Manager::Var*> getVariables(const std::vector<std::string>& variables);

      /** Add alias
       * Return true if the alias was successfully added
       */
      bool addAlias(const std::string& alias, const std::string& variable);

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
      void registerVariable(const std::string& name, Manager::FunctionPtr f, const std::string& description);
      /** Register a variable that takes floating-point arguments (see Variable::Manager::ParameterFunctionPtr). */
      void registerVariable(const std::string& name, Manager::ParameterFunctionPtr f, const std::string& description);
      /** Register a meta-variable that takes string arguments and returns a variable(see Variable::Manager::MetaFunctionPtr). */
      void registerVariable(const std::string& name, Manager::MetaFunctionPtr f, const std::string& description);

      /** evaluate variable 'varName' on given Particle.
       *
       * Mainly provided for the Python interface. For performance critical code, it is recommended to use getVariable() once and keep the Var* pointer around.
       *
       * Throws exception if variable isn't found. Assumes 'p' is != NULL.
       */
      double evaluate(const std::string& varName, const Particle* p);

      /** Return list of all variable names (in order registered). */
      std::vector<std::string> getNames() const;

      /** Abort with B2FATAL if name is not a valid name for a variable. */
      void assertValidName(const std::string& name);

    private:
      Manager() {};
      /** Copy constructor disabled (not defined). */
      Manager(const Manager&);
      ~Manager();

      /** Creates and registers a concrete variable (Var) from a MetaVar, ParameterVar or numeric constant. */
      bool createVariable(const std::string& name);

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
    };

    /** Internal class that registers a variable with Manager when constructed. */
    class Proxy {
    public:
      /** constructor. */
      Proxy(const std::string& name, Manager::FunctionPtr f, const std::string& description)
      {
        Manager::Instance().registerVariable(name, f, description);
      }
      /** constructor. */
      Proxy(const std::string& name, Manager::ParameterFunctionPtr f, const std::string& description)
      {
        Manager::Instance().registerVariable(name, f, description);
      }
      /** constructor. */
      Proxy(const std::string& name, Manager::MetaFunctionPtr f, const std::string& description)
      {
        Manager::Instance().registerVariable(name, f, description);
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

    template<typename T>
    std::function<T> make_function(T* t)
    {
      return { t };
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
