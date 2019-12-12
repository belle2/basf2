//
// Created by mwelsch on 12/6/19.
//


#pragma once

#include <framework/core/Module.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/VariableManager/Utility.h>

#include <TLorentzVector.h>

#include <string>
#include <memory>


namespace Belle2 {
  /**
   * Inclusively reconstructs a D* particle list  from an input pion particle list.
   *
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

    /** Exstimates the D* four momentum given a slow pion */
    TLorentzVector estimateDstarFourMomentum(const Particle* pion);

    /** Checks if the given pion is list if compatible with the charge of the D* particle */
    bool pionCompatibleWithDstar(int pion_pdg_code);


    std::string m_inputPionListName;  /**< Name of the input pion particle list */
    std::string m_outputDstarListName;  /**< Name of the output D* particle list */
    std::string m_slowPionCut;  /**< Cut used to identify slow pions */
    std::unique_ptr<Variable::Cut> m_cut; /**< cut object which performs the cuts */

    DecayDescriptor m_decaydescriptor; /**< Decay descriptor for parsing the user specifed DecayString */

    int m_dstar_pdg_code;
    float m_dstar_pdg_mass;
    float m_d_pdg_mass;
  };

}
