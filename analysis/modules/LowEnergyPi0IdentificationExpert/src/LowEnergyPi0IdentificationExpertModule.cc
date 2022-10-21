/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <analysis/modules/LowEnergyPi0IdentificationExpert/LowEnergyPi0IdentificationExpertModule.h>

/* Belle 2 headers. */
#include <analysis/variables/ECLVariables.h>
#include <analysis/variables/HelicityVariables.h>
#include <mva/interface/Interface.h>
#include <boost/algorithm/string/predicate.hpp>

using namespace Belle2;

REG_MODULE(LowEnergyPi0IdentificationExpert);

LowEnergyPi0IdentificationExpertModule::LowEnergyPi0IdentificationExpertModule(): Module()
{
  setDescription("Low-energy pi0 identification.");
  addParam("Pi0ListName", m_Pi0ListName, "Pi0 particle list name.",
           std::string("pi0"));
  addParam("Belle1", m_Belle1, "Belle 1 data analysis.", false);
  addParam("identifier", m_identifier,
           "Database identifier or file used to load the weights.",
           m_identifier);
  setPropertyFlags(c_ParallelProcessingCertified);
}

LowEnergyPi0IdentificationExpertModule::~LowEnergyPi0IdentificationExpertModule()
{
}

void LowEnergyPi0IdentificationExpertModule::initialize()
{
  m_ListPi0.isRequired(m_Pi0ListName);
  if (not(boost::ends_with(m_identifier, ".root") or boost::ends_with(m_identifier, ".xml"))) {
    m_weightfile_representation = std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>(new
                                  DBObjPtr<DatabaseRepresentationOfWeightfile>(m_identifier));
  }
  MVA::AbstractInterface::initSupportedInterfaces();
}

void LowEnergyPi0IdentificationExpertModule::terminate()
{
  m_expert.reset();
  m_dataset.reset();
}

void LowEnergyPi0IdentificationExpertModule::beginRun()
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

void LowEnergyPi0IdentificationExpertModule::endRun()
{
}

void LowEnergyPi0IdentificationExpertModule::init_mva(MVA::Weightfile& weightfile)
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
    nInputVariables = 6;
  else
    nInputVariables = 10;
  dummy.resize(nInputVariables, 0);
  m_dataset = std::unique_ptr<MVA::SingleDataset>(new MVA::SingleDataset(general_options, std::move(dummy), 0));
}

void LowEnergyPi0IdentificationExpertModule::event()
{
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
    double gammaLowEnergyPi0Veto, gammaHighEnergyPi0Veto;
    double gammaLowEnergyE9E21, gammaHighEnergyE9E21;
    double gammaLowEnergyClusterTheta, gammaHighEnergyClusterTheta;
    double gammaLowEnergyZernikeMVA, gammaHighEnergyZernikeMVA;
    double gammaLowEnergyIsolation, gammaHighEnergyIsolation;
    gammaLowEnergyPi0Veto = pi0->getExtraInfo("lowEnergyPi0VetoGammaLowEnergy");
    gammaHighEnergyPi0Veto =
      pi0->getExtraInfo("lowEnergyPi0VetoGammaHighEnergy");
    gammaLowEnergyE9E21 = Variable::eclClusterE9E21(gammaLowEnergy);
    gammaHighEnergyE9E21 = Variable::eclClusterE9E21(gammaHighEnergy);
    gammaLowEnergyClusterTheta = Variable::eclClusterTheta(gammaLowEnergy);
    gammaHighEnergyClusterTheta = Variable::eclClusterTheta(gammaHighEnergy);
    if (!m_Belle1) {
      gammaLowEnergyZernikeMVA = Variable::eclClusterZernikeMVA(gammaLowEnergy);
      gammaHighEnergyZernikeMVA =
        Variable::eclClusterZernikeMVA(gammaHighEnergy);
      gammaLowEnergyIsolation = Variable::eclClusterIsolation(gammaLowEnergy);
      gammaHighEnergyIsolation = Variable::eclClusterIsolation(gammaHighEnergy);
    }
    m_dataset->m_input[0] = gammaLowEnergyPi0Veto;
    m_dataset->m_input[1] = gammaHighEnergyPi0Veto;
    m_dataset->m_input[2] = gammaLowEnergyE9E21;
    m_dataset->m_input[3] = gammaHighEnergyE9E21;
    m_dataset->m_input[4] = gammaLowEnergyClusterTheta;
    m_dataset->m_input[5] = gammaHighEnergyClusterTheta;
    if (!m_Belle1) {
      m_dataset->m_input[6] = gammaLowEnergyZernikeMVA;
      m_dataset->m_input[7] = gammaHighEnergyZernikeMVA;
      m_dataset->m_input[8] = gammaLowEnergyIsolation;
      m_dataset->m_input[9] = gammaHighEnergyIsolation;
    }
    float identification = m_expert->apply(*m_dataset)[0];
    pi0->addExtraInfo("lowEnergyPi0Identification", identification);
  }
}
