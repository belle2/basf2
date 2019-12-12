//
// Created by mwelsch on 12/6/19.
//


#include <analysis/modules/InclusiveDstarReconstruction/InclusiveDstarReconstructionModule.h>

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
  // if output list is a charged (neutral) D*, assign charged (neutral) D mass
  m_d_pdg_mass = (abs(m_dstar_pdg_code) == 413) ? TDatabasePDG::Instance()->GetParticle(411)->Mass() :
                 TDatabasePDG::Instance()->GetParticle(421)->Mass();


}

void InclusiveDstarReconstructionModule::event()
{
  // loop over all pions in input list
  // if pion does not pass slow pion cut, continue
  // check if pion and dstar list fit to each other
  // estimate dstar four vector
  // create dstar particle and set slow pion as daughter

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

  // debugging purpose at the moment
  unsigned int num_slow_pions = 0;

  for (unsigned int pion_index = 0; pion_index < num_pions; pion_index++) {
    const Particle* pion = inputPionList->getParticle(pion_index);

    if (!m_cut->check(pion)) {
      continue;
    };

    num_slow_pions++;
    TLorentzVector dstar_four_vector = estimateDstarFourMomentum(pion);
    Particle dstar = Particle(
                       dstar_four_vector, m_dstar_pdg_code, Particle::EFlavorType::c_Flavored,
    {pion->getArrayIndex()}, pion->getArrayPointer());
    Particle* new_dstar = particles.appendNew(dstar);

    outputDstarList->addParticle(new_dstar);

  }

  B2DEBUG(9, "Number of slow pions found: " << num_slow_pions);
  B2DEBUG(9, "Number of pions: " << num_pions);
}

TLorentzVector InclusiveDstarReconstructionModule::estimateDstarFourMomentum(const Particle* pion)
{
  float dstar_energy = pion->getEnergy() * m_dstar_pdg_mass / (m_dstar_pdg_mass - m_d_pdg_mass);
  TVector3 slow_pion_momentum = pion->getMomentum();
  float dstar_momentum_mag = sqrt(dstar_energy * dstar_energy - m_dstar_pdg_mass * m_dstar_pdg_mass);
  TVector3 dstar_momentum = slow_pion_momentum * (dstar_momentum_mag / slow_pion_momentum.Mag());

  return TLorentzVector(dstar_momentum, dstar_energy);
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