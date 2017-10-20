#pragma once

#include <analysis/VariableManager/Manager.h>
#include <framework/core/Module.h>
#include <string>

namespace Belle2 {

  /**

   */
  class MarkDuplicateVertexModule : public Module {

  public:

    /**
     * Constructor
     */
    MarkDuplicateVertexModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:

    std::string m_particleList;              /**< input ParticleList name */
    std::string m_extraInfoName;             /**< output extra-info name */
    bool m_prioritiseV0;                    /**< if one of the decay is a V0, prioritise that before checking vertex quality */

    const Variable::Manager::Var* m_targetVar; /**< Pointer to target variable stored in the variable manager */

  };

} // Belle2 namespace
