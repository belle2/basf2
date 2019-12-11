//
// Created by mwelsch on 12/6/19.
//


#pragma once

#include <framework/core/Module.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/VariableManager/Utility.h>

#include <string>


namespace Belle2 {
  /**
   * Inclusively reconstructs `anti-B:tag` from input ParticleLists for given `B:sig`.
   *
   * `upsilonListName` ParticleList is filled with `upsilonListName -> bsigListName DstarListName`,
   * where `anti-B:tag` Particle is inclusively reconstructed from particles in input ParticleLists,
   * which do not share any final state particles (mdstSource) with `B:sig`.
   *
   * Input ParticleLists are passed as std::vector containing their names (`inputListsNames`).
   */
  class InclusiveDstarReconstructionModule : public Module {

  public:

    /** Constructor */
    InclusiveDstarReconstructionModule();
    /** Destructor */
    virtual ~InclusiveDstarReconstructionModule();
    /** initialize the module (setup the data store) */
    virtual void initialize() override;
    /** process event */
    virtual void event() override;


  private:

    std::string m_inputPionListName;  /**< Name of the input pion particle list */
    std::string m_outputDstarListName;  /**< Name of the output D* particle list */
    std::string m_slowPionCut;  /**< Cut used to identify slow pions */
    std::unique_ptr<Variable::Cut> m_cut; /**< cut object which performs the cuts */

    DecayDescriptor m_decaydescriptor; /**< Decay descriptor for parsing the user specifed DecayString */
  };

}
