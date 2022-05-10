/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <mva/modules/MVAExpert/MVAExpertModule.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <analysis/dataobjects/EventExtraInfo.h>

#include <mva/interface/Interface.h>

#include <boost/algorithm/string/predicate.hpp>
#include <memory>

#include <framework/logging/Logger.h>


using namespace Belle2;

REG_MODULE(MVAExpert);

MVAExpertModule::MVAExpertModule() : Module()
{
  setDescription("Adds an ExtraInfo to the Particle objects in given ParticleLists which is calcuated by an expert defined by a weightfile.");
  setPropertyFlags(c_ParallelProcessingCertified);

  std::vector<std::string> empty;
  addParam("listNames", m_listNames,
           "Particles from these ParticleLists are used as input. If no name is given the expert is applied to every event once, and one can only use variables which accept nullptr as Particle*",
           empty);
  addParam("extraInfoName", m_extraInfoName,
           "Name under which the output of the expert is stored in the ExtraInfo of the Particle object.");
  addParam("identifier", m_identifier, "The database identifier which is used to load the weights during the training.");
  addParam("signalFraction", m_signal_fraction_override,
           "signalFraction to calculate probability (if -1 the signalFraction of the training data is used)", -1.0);
  addParam("overwriteExistingExtraInfo", m_overwriteExistingExtraInfo,
           "If true, when the given extraInfo has already defined, the old extraInfo value is overwritten. If false, the original value is kept.",
           true);
}

void MVAExpertModule::initialize()
{
  // All specified ParticleLists are required to exist
  for (auto& name : m_listNames) {
    StoreObjPtr<ParticleList> list(name);
    list.isRequired();
  }

  if (m_listNames.empty()) {
    StoreObjPtr<EventExtraInfo> extraInfo("", DataStore::c_Event);
    extraInfo.registerInDataStore();
  } else {
    StoreObjPtr<ParticleExtraInfoMap> extraInfo("", DataStore::c_Event);
    extraInfo.registerInDataStore();
  }

  if (not(boost::ends_with(m_identifier, ".root") or boost::ends_with(m_identifier, ".xml"))) {
    m_weightfile_representation = std::make_unique<DBObjPtr<DatabaseRepresentationOfWeightfile>>(
                                    MVA::makeSaveForDatabase(m_identifier));
  }
  MVA::AbstractInterface::initSupportedInterfaces();

  m_existGivenExtraInfo = false;
}

void MVAExpertModule::beginRun()
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

void MVAExpertModule::init_mva(MVA::Weightfile& weightfile)
{

  auto supported_interfaces = MVA::AbstractInterface::getSupportedInterfaces();
  MVA::GeneralOptions general_options;
  weightfile.getOptions(general_options);

  // Overwrite signal fraction from training
  if (m_signal_fraction_override > 0)
    weightfile.addSignalFraction(m_signal_fraction_override);

  m_expert = supported_interfaces[general_options.m_method]->getExpert();
  m_expert->load(weightfile);

  Variable::Manager& manager = Variable::Manager::Instance();
  m_feature_variables =  manager.getVariables(general_options.m_variables);
  if (m_feature_variables.size() != general_options.m_variables.size()) {
    B2FATAL("One or more feature variables could not be loaded via the Variable::Manager. Check the names!");
  }

  std::vector<float> dummy;
  dummy.resize(m_feature_variables.size(), 0);
  m_dataset = std::make_unique<MVA::SingleDataset>(general_options, dummy, 0);

}

float MVAExpertModule::analyse(Particle* particle)
{
  if (not m_expert) {
    B2ERROR("MVA Expert is not loaded! I will return 0");
    return 0.0;
  }
  for (unsigned int i = 0; i < m_feature_variables.size(); ++i) {
    auto var_result = m_feature_variables[i]->function(particle);
    if (std::holds_alternative<double>(var_result)) {
      m_dataset->m_input[i] = std::get<double>(var_result);
    } else if (std::holds_alternative<int>(var_result)) {
      m_dataset->m_input[i] = std::get<int>(var_result);
    } else if (std::holds_alternative<bool>(var_result)) {
      m_dataset->m_input[i] = std::get<bool>(var_result);
    }
  }
  return m_expert->apply(*m_dataset)[0];
}


void MVAExpertModule::event()
{
  for (auto& listName : m_listNames) {
    StoreObjPtr<ParticleList> list(listName);
    // Calculate target Value for Particles
    for (unsigned i = 0; i < list->getListSize(); ++i) {
      Particle* particle = list->getParticle(i);
      float targetValue = analyse(particle);
      if (particle->hasExtraInfo(m_extraInfoName)) {
        m_existGivenExtraInfo = true;
        if (m_overwriteExistingExtraInfo)
          particle->setExtraInfo(m_extraInfoName, targetValue);
      } else {
        particle->addExtraInfo(m_extraInfoName, targetValue);
      }
    }
  }
  if (m_listNames.empty()) {
    StoreObjPtr<EventExtraInfo> eventExtraInfo;
    if (not eventExtraInfo.isValid())
      eventExtraInfo.create();

    float targetValue = analyse(nullptr);
    if (eventExtraInfo->hasExtraInfo(m_extraInfoName)) {
      m_existGivenExtraInfo = true;
      if (m_overwriteExistingExtraInfo)
        eventExtraInfo->setExtraInfo(m_extraInfoName, targetValue);
    } else {
      eventExtraInfo->addExtraInfo(m_extraInfoName, targetValue);
    }
  }
}

void MVAExpertModule::terminate()
{
  m_expert.reset();
  m_dataset.reset();

  if (m_existGivenExtraInfo) {
    if (m_overwriteExistingExtraInfo)
      B2WARNING("The extraInfo " << m_extraInfoName << " has already been set! It was overwritten by this module!");
    else
      B2WARNING("The extraInfo " << m_extraInfoName << " has already been set! "
                << "The original value was kept and this module did not overwrite it!");
  }
}
