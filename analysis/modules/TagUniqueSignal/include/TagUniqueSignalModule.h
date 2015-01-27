#pragma once

#include <analysis/VariableManager/Manager.h>
#include <framework/core/Module.h>
#include <string>

namespace Belle2 {

  /**

   */
  class TagUniqueSignalModule : public Module {

  public:

    /**
     * Constructor
     */
    TagUniqueSignalModule();

    /**
     * Destructor
     */
    virtual ~TagUniqueSignalModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();


    /**
     * Event processor.
     */
    virtual void event();

  private:

    std::string m_particleList;              /**< input ParticleList name */
    std::string m_extraInfoName;              /**< output extra-info name */

    std::string m_targetVariable; /**< Variable which defines ginal and background */
    const Variable::Manager::Var* m_targetVar; /**< Pointer to target variable stored in the variable manager */

  };

} // Belle2 namespace
