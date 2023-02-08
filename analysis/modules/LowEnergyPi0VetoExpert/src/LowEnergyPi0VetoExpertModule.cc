/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <analysis/modules/LowEnergyPi0VetoExpert/LowEnergyPi0VetoExpertModule.h>

/* Basf2 headers. */
#include <analysis/variables/ECLVariables.h>
#include <analysis/variables/HelicityVariables.h>
#include <mva/interface/Interface.h>
#include <boost/algorithm/string/predicate.hpp>

/* ROOT headers. */
#include <Math/Vector3D.h>
#include <Math/Vector4D.h>
#include <Math/VectorUtil.h>

using namespace Belle2;

REG_MODULE(LowEnergyPi0VetoExpert);

LowEnergyPi0VetoExpertModule::LowEnergyPi0VetoExpertModule(): Module()
{
  setDescription("Low-energy pi0 veto.");
  addParam(
    "VetoPi0Daughters", m_VetoPi0Daughters,
    "Veto for pi0 daughters (maximum over all pairs excluding this pi0).",
    false);
  addParam("GammaListName", m_GammaListName, "Gamma particle list name.",
           std::string("gamma"));
  addParam("Pi0ListName", m_Pi0ListName, "Pi0 particle list name.",
           std::string("pi0"));
  addParam("Belle1", m_Belle1, "Belle 1 data analysis.", false);
  addParam("identifier", m_identifier,
           "Database identifier or file used to load the weights.",
           m_identifier);
  setPropertyFlags(c_ParallelProcessingCertified);
}

LowEnergyPi0VetoExpertModule::~LowEnergyPi0VetoExpertModule()
{
}

void LowEnergyPi0VetoExpertModule::initialize()
{
  m_ListGamma.isRequired(m_GammaListName);
  if (m_VetoPi0Daughters)
    m_ListPi0.isRequired(m_Pi0ListName);
  if (not(boost::ends_with(m_identifier, ".root") or boost::ends_with(m_identifier, ".xml"))) {
    m_weightfile_representation = std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>(new
                                  DBObjPtr<DatabaseRepresentationOfWeightfile>(m_identifier));
  }
  MVA::AbstractInterface::initSupportedInterfaces();
}

void LowEnergyPi0VetoExpertModule::terminate()
{
  m_expert.reset();
  m_dataset.reset();
}

void LowEnergyPi0VetoExpertModule::beginRun()
{
  if (m_weightfile_representation) {
    if (m_weightfile_representation->hasChanged()) {
      std::stringstream ss((*m_weightfile_representation)->m_data);
      auto weightfile = MVA::Weightfile::loadFromStream(ss);
      init_mva(weightfile);
    }
  } else {
    auto weightfile = MVA::Weightfile::loadFromFile(m_identifier);
    init_mva(weightfile);
  }
}

void LowEnergyPi0VetoExpertModule::endRun()
{
}

void LowEnergyPi0VetoExpertModule::init_mva(MVA::Weightfile& weightfile)
{
  auto supported_interfaces = MVA::AbstractInterface::getSupportedInterfaces();
  MVA::GeneralOptions general_options;
  weightfile.getOptions(general_options);
  weightfile.addSignalFraction(0.5);
  m_expert = supported_interfaces[general_options.m_method]->getExpert();
  m_expert->load(weightfile);
  std::vector<float> dummy;
  /* The number of input variables depends on the experiment. */
  int nInputVariables;
  if (m_Belle1)
    nInputVariables = 7;
  else
    nInputVariables = 11;
  dummy.resize(nInputVariables, 0);
  m_dataset = std::unique_ptr<MVA::SingleDataset>(new MVA::SingleDataset(general_options, std::move(dummy), 0));
}

float LowEnergyPi0VetoExpertModule::getMaximumVeto(const Particle* gamma1,
                                                   const Particle* pi0Gamma)
{
  float maxVeto = 0;
  int n = m_ListGamma->getListSize();
  for (int i = 0; i < n; ++i) {
    Particle* gamma2 = m_ListGamma->getParticle(i);
    if (gamma1 == gamma2)
      continue;
    if (pi0Gamma != nullptr) {
      if (pi0Gamma == gamma2)
        continue;
    }
    double pi0Mass = (gamma1->get4Vector() + gamma2->get4Vector()).M();
    if (pi0Mass < 0.07 || pi0Mass > 0.20)
      continue;
    const Particle* gammaLowEnergy, *gammaHighEnergy;
    if (gamma1->getEnergy() > gamma2->getEnergy()) {
      gammaLowEnergy = gamma2;
      gammaHighEnergy = gamma1;
    } else {
      gammaLowEnergy = gamma1;
      gammaHighEnergy = gamma2;
    }
    double gammaLowEnergyEnergy, gammaHighEnergyEnergy;
    double gammaLowEnergyE9E21, gammaHighEnergyE9E21;
    double gammaLowEnergyClusterTheta, gammaHighEnergyClusterTheta;
    double gammaLowEnergyZernikeMVA, gammaHighEnergyZernikeMVA;
    double gammaLowEnergyIsolation, gammaHighEnergyIsolation;
    double cosHelicityAngleMomentum;
    gammaLowEnergyEnergy = gammaLowEnergy->getEnergy();
    gammaHighEnergyEnergy = gammaHighEnergy->getEnergy();
    ROOT::Math::PxPyPzEVector gammaHighEnergyMomentum(
      gammaHighEnergy->getPx(), gammaHighEnergy->getPy(),
      gammaHighEnergy->getPz(), gammaHighEnergyEnergy);
    ROOT::Math::PxPyPzEVector gammaLowEnergyMomentum(
      gammaLowEnergy->getPx(), gammaLowEnergy->getPy(),
      gammaLowEnergy->getPz(), gammaLowEnergyEnergy);
    ROOT::Math::PxPyPzEVector momentum = gammaHighEnergyMomentum +
                                         gammaLowEnergyMomentum;
    ROOT::Math::XYZVector boost = momentum.BoostToCM();
    gammaHighEnergyMomentum =
      ROOT::Math::VectorUtil::boost(gammaHighEnergyMomentum, boost);
    cosHelicityAngleMomentum =
      fabs(ROOT::Math::VectorUtil::CosTheta(momentum.Vect(),
                                            gammaHighEnergyMomentum.Vect()));
    gammaLowEnergyE9E21 = Variable::eclClusterE9E21(gammaLowEnergy);
    gammaHighEnergyE9E21 = Variable::eclClusterE9E21(gammaHighEnergy);
    gammaLowEnergyClusterTheta = Variable::eclClusterTheta(gammaLowEnergy);
    gammaHighEnergyClusterTheta = Variable::eclClusterTheta(gammaHighEnergy);
    if (!m_Belle1) {
      gammaLowEnergyZernikeMVA =
        Variable::eclClusterZernikeMVA(gammaLowEnergy);
      gammaHighEnergyZernikeMVA =
        Variable::eclClusterZernikeMVA(gammaHighEnergy);
      gammaLowEnergyIsolation = Variable::eclClusterIsolation(gammaLowEnergy);
      gammaHighEnergyIsolation =
        Variable::eclClusterIsolation(gammaHighEnergy);
    }
    m_dataset->m_input[0] = gammaLowEnergyEnergy;
    m_dataset->m_input[1] = pi0Mass;
    m_dataset->m_input[2] = cosHelicityAngleMomentum;
    m_dataset->m_input[3] = gammaLowEnergyE9E21;
    m_dataset->m_input[4] = gammaHighEnergyE9E21;
    m_dataset->m_input[5] = gammaLowEnergyClusterTheta;
    m_dataset->m_input[6] = gammaHighEnergyClusterTheta;
    if (!m_Belle1) {
      m_dataset->m_input[7] = gammaLowEnergyZernikeMVA;
      m_dataset->m_input[8] = gammaHighEnergyZernikeMVA;
      m_dataset->m_input[9] = gammaLowEnergyIsolation;
      m_dataset->m_input[10] = gammaHighEnergyIsolation;
    }
    float veto = m_expert->apply(*m_dataset)[0];
    if (veto > maxVeto)
      maxVeto = veto;
  }
  return maxVeto;
}

void LowEnergyPi0VetoExpertModule::event()
{
  if (m_VetoPi0Daughters) {
    int n = m_ListPi0->getListSize();
    for (int i = 0; i < n; ++i) {
      Particle* pi0 = m_ListPi0->getParticle(i);
      const Particle* gamma1 = pi0->getDaughter(0);
      const Particle* gamma2 = pi0->getDaughter(1);
      const Particle* gammaLowEnergy, *gammaHighEnergy;
      if (gamma1->getEnergy() > gamma2->getEnergy()) {
        gammaLowEnergy = gamma2;
        gammaHighEnergy = gamma1;
      } else {
        gammaLowEnergy = gamma1;
        gammaHighEnergy = gamma2;
      }
      float maxVeto = getMaximumVeto(gammaLowEnergy, gammaHighEnergy);
      pi0->addExtraInfo("lowEnergyPi0VetoGammaLowEnergy", maxVeto);
      maxVeto = getMaximumVeto(gammaHighEnergy, gammaLowEnergy);
      pi0->addExtraInfo("lowEnergyPi0VetoGammaHighEnergy", maxVeto);
    }
  } else {
    int n = m_ListGamma->getListSize();
    for (int i = 0; i < n; ++i) {
      Particle* gamma = m_ListGamma->getParticle(i);
      float maxVeto = getMaximumVeto(gamma, nullptr);
      gamma->addExtraInfo("lowEnergyPi0Veto", maxVeto);
    }
  }
}
