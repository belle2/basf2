#pragma once

#include <string>
#include <map>
#include <vector>

namespace Belle2 {
  class Particle;

  /** Global list of available variables.
   *
   *  Each variable is has an associated unique string key through
   *  which it can be accessed. The VariableManager can be used to get
   *  an instance of the Variable class for this key. All variables take a
   *  const Particle* and return a double.
   *
   *  Variables will automatically register themselves when
   *  the library that contains them is loaded. They only need to be put in a .cc file,
   *  with a call to REGISTER_VARIABLE() to set a name and description.
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
   *
   *  Variables can then be accessed using getVariable(name) and getVariables().
   */
  class VariableManager {
  public:
    /** functions stored take a const Particle* and return double. */
    typedef double(*FunctionPtr)(const Particle*);

    /** Struct containing the function used for calculation and the description. */
    struct Var {
      std::string name; /**< Unique identifier of the function, used as key. */
      FunctionPtr function; /**< Pointer to function. */
      std::string description; /**< Description of what this function does. */

      /** Constructor. */
      Var(std::string n, FunctionPtr f, std::string d) : name(n), function(f), description(d) { }
    };

    /** get singleton instance. */
    static VariableManager& Instance();


    /** Get the variable belonging to the given key.
     *
     * Returns NULL if name not found.
     */
    const Var* getVariable(const std::string& name) const;

    /** Return list of all variables (in order registered). */
    std::vector<const Var*> getVariables() const { return m_variablesInRegistrationOrder; }

    /** Register a variable. */
    void registerVariable(const std::string& name, VariableManager::FunctionPtr f, const std::string& description);

  private:
    VariableManager() {};
    /** Copy constructor disabled (not defined). */
    VariableManager(const VariableManager&);
    ~VariableManager();

    /** List of registered variables. */
    std::map<std::string, Var*> m_variables;

    /** List of variables in registration order. */
    std::vector<const Var*> m_variablesInRegistrationOrder;


  };

  /** Internal class that registers a variable with VariableManager when constructed. */
  class VariableProxy {
  public:
    /** constructor. */
    VariableProxy(const std::string& name, VariableManager::FunctionPtr f, const std::string& description) {
      VariableManager::Instance().registerVariable(name, f, description);
    }
  };


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
   * \sa VariableManager
   */
#define REGISTER_VARIABLE(name, function, description) \
  VariableProxy VARMANAGER_MAKE_UNIQUE(_variableproxy##function)(name, &function, description);

}
