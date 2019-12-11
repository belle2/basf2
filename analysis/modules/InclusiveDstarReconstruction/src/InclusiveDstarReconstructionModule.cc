//
// Created by mwelsch on 12/6/19.
//


#include <analysis/modules/InclusiveDstarReconstruction/InclusiveDstarReconstructionModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/DecayDescriptor/ParticleListName.h>

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

}

void InclusiveDstarReconstructionModule::event()
{
  // loop over all pions in input list
  // if pion does not pass slow pion cut, continue
  // check if pion and dstar list fit to each other
  // estimate dstar four vector
  // create dstar particle and set slow pion as daughter
  int pdgCode  = 0;

  // check the validity of output ParticleList name
  bool valid = m_decaydescriptor.init(m_outputDstarListName);
  if (!valid)
    B2ERROR("InclusiveDstarReconstructionModule::initialize Invalid output ParticleList name: " << m_outputDstarListName);

  // Output particle
  const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();
  pdgCode  = mother->getPDGCode();

  StoreObjPtr<ParticleList> outputDstarList(m_outputDstarListName);
  outputDstarList.create();
  outputDstarList->initialize(pdgCode, outputDstarList.getName());

  StoreObjPtr<ParticleList> outputAntiDstarList(ParticleListName::antiParticleListName(m_outputDstarListName));
  outputAntiDstarList.create();
  outputAntiDstarList->initialize(-pdgCode, ParticleListName::antiParticleListName(m_outputDstarListName));

  outputDstarList->bindAntiParticleList(*outputAntiDstarList);






}