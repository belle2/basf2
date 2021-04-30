/**************************************************************************
  * BASF2 (Belle Analysis Framework 2)                                     *
  * Copyright(C) 2020 - Belle II Collaboration                             *
  *                                                                        *
  * Author: The Belle II Collaboration                                     *
  * Contributors: Maximilian Welsch, Pascal Schmolz                        *
  *                                                                        *
  * This software is provided "as is" without any warranty.                *
  **************************************************************************/

#include <analysis/modules/InclusiveDstarReconstruction/InclusiveDstarReconstructionModule.h>
#include <analysis/utility/PCmsLabTransform.h>

#include <analysis/DecayDescriptor/ParticleListName.h>

#include <framework/gearbox/Const.h>

#include <TVector3.h>
#include <TDatabasePDG.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(InclusiveDstarReconstruction)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
InclusiveDstarReconstructionModule::InclusiveDstarReconstructionModule() : Module()
{
  // Set module properties
  setDescription("Inclusive Dstar reconstruction by estimating the four vector using slow pions");

  // Parameter definitions
  addParam("decayString", m_decayString, "Input DecayDescriptor string", std::string(""));
  addParam("slowPionCut", m_slowPionCut, "Cut for slow pions", std::string("useCMSFrame(p) < 0.2"));
  addParam("DstarCut", m_DstarCut, "Cut for Dstar", std::string(""));

  m_dstar_pdg_code = 0;
  m_dstar_pdg_mass = 0;
  m_d_pdg_mass = 0;
}

InclusiveDstarReconstructionModule::~InclusiveDstarReconstructionModule() = default;

void InclusiveDstarReconstructionModule::initialize()
{
  m_outputListName = "";
  m_outputAntiListName = "";
  m_pionListName = "";

  bool valid = m_decaydescriptor.init(m_decayString);
  if (!valid)
    B2ERROR("Invalid input DecayString: " << m_decayString);

  // mother particle: D*
  const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();
  m_dstar_pdg_code = mother->getPDGCode();

  // daughter particle: pion (is the only daughter)
  int pion_pdg_code = 0;
  const DecayDescriptorParticle* daughter = m_decaydescriptor.getDaughter(0)->getMother();
  m_pionListName = daughter->getFullName();
  m_inputPionList.isRequired(m_pionListName);
  pion_pdg_code = daughter->getPDGCode();

  if (!pionCompatibleWithDstar(pion_pdg_code))
    B2ERROR("Pion PDG code " << pion_pdg_code << " not compatible with D* PDG code " <<  m_dstar_pdg_code);

  // create and register output particle list
  m_outputListName = mother->getFullName();
  m_outputDstarList.registerInDataStore(m_outputListName);
  // create and register output antiparticle list
  m_outputAntiListName = ParticleListName::antiParticleListName(m_outputListName);
  m_outputAntiDstarList.registerInDataStore(m_outputAntiListName);

  m_cut_dstar = Variable::Cut::compile(m_DstarCut);
  m_cut_pion = Variable::Cut::compile(m_slowPionCut);

  m_dstar_pdg_mass = TDatabasePDG::Instance()->GetParticle(m_dstar_pdg_code)->Mass();

  // get the mass of daughter-D:
  /*
  decay 1. D*+ {413} -> pi+ {211} D0 {421}
  decay 2. D*+ {413} -> pi0 {111} D+ {411}
  decay 3. D*0 {423} -> pi0 {111} D0 {421}
  */
  int d_pdg_code = 0;
  if (abs(m_dstar_pdg_code) == 413) {
    d_pdg_code = (pion_pdg_code == Const::pi0.getPDGCode()) ? 411 : 421;
  } else {
    d_pdg_code = 421;
  }
  m_d_pdg_mass = TDatabasePDG::Instance()->GetParticle(d_pdg_code)->Mass();
}

void InclusiveDstarReconstructionModule::event()
{

  m_outputDstarList.create();
  m_outputDstarList->initialize(m_dstar_pdg_code, m_outputDstarList.getName());

  m_outputAntiDstarList.create();
  m_outputAntiDstarList->initialize(-m_dstar_pdg_code, m_outputAntiListName);
  m_outputDstarList->bindAntiParticleList(*m_outputAntiDstarList);

  unsigned int num_pions = m_inputPionList->getListSize();

  for (unsigned int pion_index = 0; pion_index < num_pions; pion_index++) {
    const Particle* pion = m_inputPionList->getParticle(pion_index);

    if (!m_cut_pion->check(pion)) continue;

    TLorentzVector dstar_four_vector = estimateDstarFourMomentum(pion);

    if (isnan(dstar_four_vector.P())) continue;

    /*
    decay 1:
    - pion-charge > 0: m_dstar_pdg_code
    - pion-charge < 0: -m_dstar_pdg_code
    decay 2 and 3:
    - pion-charge = 0: m_dstar_pdg_code as well as -m_dstar_pdg_code
    - for these cases we need to store the particleList and antiParticleList
      with the same candidates
    */

    int particle_properties = Particle::PropertyFlags::c_IsIgnoreRadiatedPhotons
                              + Particle::PropertyFlags::c_IsIgnoreIntermediate
                              + Particle::PropertyFlags::c_IsIgnoreMassive
                              + Particle::PropertyFlags::c_IsIgnoreNeutrino
                              + Particle::PropertyFlags::c_IsIgnoreGamma;

    // for decay 1 and decay 2/3 with positive flavor
    int output_dstar_pdg = getDstarOutputPDG(pion->getCharge(), m_dstar_pdg_code);
    Particle dstar = Particle(dstar_four_vector, output_dstar_pdg,
                              Particle::EFlavorType::c_Flavored, {pion->getArrayIndex()},
                              particle_properties, pion->getArrayPointer());

    Particle* new_dstar = m_particles.appendNew(dstar);
    if (!m_cut_dstar->check(new_dstar)) continue;
    m_outputDstarList->addParticle(new_dstar);

    // for decay 2/3 with negative flavor
    if (pion->getCharge() == 0) {
      Particle antidstar = Particle(dstar_four_vector, -m_dstar_pdg_code,
                                    Particle::EFlavorType::c_Flavored, {pion->getArrayIndex()},
                                    particle_properties, pion->getArrayPointer());

      Particle* new_antidstar = m_particles.appendNew(antidstar);
      if (!m_cut_dstar->check(new_antidstar)) continue;
      m_outputAntiDstarList->addParticle(new_antidstar);
    }
  }
}

TLorentzVector InclusiveDstarReconstructionModule::estimateDstarFourMomentum(const Particle* pion)
{
  // estimate D* energy and absolute momentum using the slow pion energy
  double energy_dstar = pion->getEnergy() * m_dstar_pdg_mass / (m_dstar_pdg_mass - m_d_pdg_mass);
  double abs_momentum_dstar = sqrt(energy_dstar * energy_dstar - m_dstar_pdg_mass * m_dstar_pdg_mass);

  // dstar momentum approximated collinear to pion direction
  TVector3 momentum_vector_pion =  pion->getMomentum();
  TVector3 momentum_vec_dstar = abs_momentum_dstar * momentum_vector_pion.Unit();

  return TLorentzVector(momentum_vec_dstar, energy_dstar);
}

bool InclusiveDstarReconstructionModule::pionCompatibleWithDstar(int pion_pdg_code)
{
  bool is_compatible = false;
  if (pion_pdg_code == Const::pi0.getPDGCode()) {
    is_compatible = true;
  } else if (pion_pdg_code == Const::pion.getPDGCode()) {
    is_compatible = (m_dstar_pdg_code == 413);
  } else if (pion_pdg_code == -Const::pion.getPDGCode()) {
    is_compatible = (m_dstar_pdg_code == -413);
  }
  return is_compatible;
}

int InclusiveDstarReconstructionModule::getDstarOutputPDG(int pion_charge, int input_dstar_pdg)
{
  // Helper function to get the correct D* PDG code depending on the input (DecayDescriptor)
  // and the charge of the reconstructed pion

  // DecayDescriptor: D*+ -> pi0 or D*0 -> pi0
  // the opposite (D*- -> pi0 or anti-D*0 -> pi0) are treated from line 150.
  if (pion_charge == 0) {
    return input_dstar_pdg;
  }

  else {
    // DecayDescriptor: D*+ -> pi+
    if (input_dstar_pdg > 0) {
      return (pion_charge > 0) ? input_dstar_pdg : -input_dstar_pdg;
    }
    // DecayDescriptor: D* -> pi-
    else {
      return (pion_charge < 0) ? input_dstar_pdg : -input_dstar_pdg;
    }
  }

}
