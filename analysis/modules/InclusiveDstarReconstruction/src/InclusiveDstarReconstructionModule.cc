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
  addParam("bsigListName", m_inputPionListName, "Name of the input pion ParticleList", std::string(""));
  addParam("DstarListName", m_outputDstarListName, "Name of the output D* ParticleList", std::string(""));
  addParam("slowPionCut", m_slowPionCut, "Cut for selecting slow pions", std::string("useCMSFrame(p) < 0.2"));

}

InclusiveDstarReconstructionModule::~InclusiveDstarReconstructionModule() = default;

void InclusiveDstarReconstructionModule::initialize()
{
  StoreObjPtr<ParticleList> inputPionList(m_inputPionListName);
  inputPionList.isRequired();

  StoreObjPtr<ParticleList> outputDstarList(m_outputDstarListName);
  outputDstarList.registerInDataStore();

  StoreObjPtr<ParticleList> outputAntiDstarList(ParticleListName::antiParticleListName(m_outputDstarListName));
  outputAntiDstarList.registerInDataStore();

  m_cut = Variable::Cut::compile(m_slowPionCut);

  // initialize Dstar and D masses from PDG
  // check the validity of output ParticleList name
  bool valid = m_decaydescriptor.init(m_outputDstarListName);
  if (!valid)
    B2ERROR("InclusiveDstarReconstructionModule::initialize Invalid output ParticleList name: " << m_outputDstarListName);

  const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();
  m_dstar_pdg_code  = mother->getPDGCode();
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

  // Create output particle and antiparticle list

  StoreArray<Particle> particles;

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
    auto pion = inputPionList->getParticle(pion_index);
    if (!m_cut->check(pion)) {
      continue;
    }

    TLorentzVector dstar_four_vector = estimateDstarFourMomentum(pion);
    Particle dstar = Particle(dstar_four_vector, m_dstar_pdg_code);
    dstar.appendDaughter(pion, false);

    Particle* new_dstar = particles.appendNew(dstar);

    outputDstarList->addParticle(new_dstar);

  }

}

TLorentzVector InclusiveDstarReconstructionModule::estimateDstarFourMomentum(const Particle* pion)
{
  float slow_pion_energy = pion->getEnergy();
  TVector3 slow_pion_momentum = pion ->getMomentum();
  float dstar_energy = slow_pion_energy * m_dstar_pdg_mass / (m_dstar_pdg_mass - m_d_pdg_mass);
  float dstar_momentum_mag = sqrt(dstar_energy * dstar_energy - m_dstar_pdg_mass * m_dstar_pdg_mass);
  TVector3 dstar_momentum = slow_pion_momentum * (dstar_momentum_mag / slow_pion_momentum.Mag());

  return TLorentzVector(dstar_momentum, dstar_energy);
}