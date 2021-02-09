#pragma once

#include <framework/core/Module.h>

namespace Belle2 {

  /**
  * Energy bias correction
  */
  class EnergyBiasCorrectionModule : public Module {
  public:
    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    EnergyBiasCorrectionModule();

    /**
    * Function to be executed at each event
    */
    virtual void event() override;

  private:
    /** input particle lists */
    std::vector<std::string> m_ParticleLists;
    /** input momentum scale modifier */
    double m_scale;

  }; // EnergyBiasCorrectionModule

}; //namespace
