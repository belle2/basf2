//
// Created by mwelsch on 12/6/19.
//


#include <analysis/modules/InclusiveDstarReconstruction/InclusiveDstarReconstructionModule.h>
#include <analysis/utility/PCmsLabTransform.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/DecayDescriptor/ParticleListName.h>

#include <TLorentzVector.h>
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
  addParam("pionListName", m_inputPionListName, "Name of the input pion ParticleList", std::string(""));
  addParam("DstarListName", m_outputDstarListName, "Name of the output D* ParticleList", std::string(""));
  addParam("slowPionCut", m_slowPionCut, "Cut for selecting slow pions", std::string("useCMSFrame(p) < 0.2"));

  m_dstar_pdg_code = 0;
  m_dstar_pdg_mass = 0;
  m_d_pdg_mass = 0;

}

InclusiveDstarReconstructionModule::~InclusiveDstarReconstructionModule() = default;

void InclusiveDstarReconstructionModule::initialize()
{

  // check the validity of output ParticleList name
  bool valid = m_decaydescriptor.init(m_outputDstarListName);
  if (!valid)
    B2ERROR("InclusiveDstarReconstructionModule::initialize Invalid Output ParticleList Name: " << m_outputDstarListName);

  // get output particle pdg code
  const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();
  m_dstar_pdg_code  = mother->getPDGCode();

  valid = m_decaydescriptor.init(m_inputPionListName);
  if (!valid)
    B2ERROR("InclusiveDstarReconstructionModule::initialize Invalid Input ParticleList Name: " << m_outputDstarListName);

  // get output particle pdg code
  mother = m_decaydescriptor.getMother();
  int pion_pdg_code  = mother->getPDGCode();

  // check compatibility of particle lists like (D*+ needs either pi+ or pi-, D*0 only compatible with pi0)??
  if (!pionCompatibleWithDstar(pion_pdg_code))
    B2ERROR("Pion PDG code " << pion_pdg_code << " not compatible with D* PDG code " << m_dstar_pdg_code);

  // ensure that input pion list has been registered in data store previously
  StoreObjPtr<ParticleList> inputPionList(m_inputPionListName);
  inputPionList.isRequired();

  // create and register output particle list
  StoreObjPtr<ParticleList> outputDstarList(m_outputDstarListName);
  outputDstarList.registerInDataStore();
  StoreObjPtr<ParticleList> outputAntiDstarList(ParticleListName::antiParticleListName(m_outputDstarListName));
  outputAntiDstarList.registerInDataStore();

  m_cut = Variable::Cut::compile(m_slowPionCut);

  m_dstar_pdg_mass = TDatabasePDG::Instance()->GetParticle(m_dstar_pdg_code)->Mass();

  if (abs(m_dstar_pdg_code) == 413) {
    m_d_pdg_mass = (pion_pdg_code == 111) ? TDatabasePDG::Instance()->GetParticle(421)->Mass() : TDatabasePDG::Instance()->GetParticle(
                     411)->Mass();
  } else {
    m_d_pdg_mass = TDatabasePDG::Instance()->GetParticle(421)->Mass();
  }

}

void InclusiveDstarReconstructionModule::event()
{
  StoreArray<Particle> particles;

  // create output particle lists and bind anti-particle list
  StoreObjPtr<ParticleList> outputDstarList(m_outputDstarListName);
  outputDstarList.create();
  outputDstarList->initialize(m_dstar_pdg_code, outputDstarList.getName());

  StoreObjPtr<ParticleList> outputAntiDstarList(ParticleListName::antiParticleListName(m_outputDstarListName));
  outputAntiDstarList.create();
  outputAntiDstarList->initialize(-m_dstar_pdg_code, ParticleListName::antiParticleListName(m_outputDstarListName));
  outputDstarList->bindAntiParticleList(*outputAntiDstarList);

  // loop over all pions and cut on slow pion cut
  StoreObjPtr<ParticleList> inputPionList(m_inputPionListName);
  unsigned int num_pions = inputPionList->getListSize();

  for (unsigned int pion_index = 0; pion_index < num_pions; pion_index++) {
    const Particle* pion = inputPionList->getParticle(pion_index);

    if (!m_cut->check(pion)) {
      continue;
    };

    TLorentzVector dstar_four_vector = estimateDstarFourMomentum(pion);

    if (isnan(dstar_four_vector.P())) continue;

    Particle dstar = Particle(dstar_four_vector, m_dstar_pdg_code, Particle::EFlavorType::c_Flavored, {pion->getArrayIndex()},
                              pion->getArrayPointer());
    Particle* new_dstar = particles.appendNew(dstar);
    outputDstarList->addParticle(new_dstar);

  }
}

TLorentzVector InclusiveDstarReconstructionModule::estimateDstarFourMomentum(const Particle* pion)
{
//  // estimate D* energy and absolute momentum using the slow pion energy
  double energy_dstar = pion->getEnergy() * m_dstar_pdg_mass / (m_dstar_pdg_mass - m_d_pdg_mass);
  double abs_momentum_dstar = sqrt(energy_dstar * energy_dstar - m_dstar_pdg_mass * m_dstar_pdg_mass);

  // dstar momentum approximated colinear to pion direction
  TVector3 momentum_vector_pion =  pion->getMomentum();
  TVector3 momentum_vec_dstar = momentum_vector_pion * (abs_momentum_dstar / momentum_vector_pion.Mag());

  return TLorentzVector(momentum_vec_dstar, energy_dstar);
}

bool InclusiveDstarReconstructionModule::pionCompatibleWithDstar(int pion_pdg_code)
{
  bool is_compatible = false;
  if (pion_pdg_code == 111) {
    is_compatible = true;
  } else if (pion_pdg_code == 211) {
    is_compatible = (m_dstar_pdg_code == 413);
  } else if (pion_pdg_code == -211) {
    is_compatible = (m_dstar_pdg_code == -413);
  }

  return is_compatible;
}
