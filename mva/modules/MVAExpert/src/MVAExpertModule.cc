/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <mva/modules/MVAExpert/MVAExpertModule.h>

#include <analysis/DecayDescriptor/DecayDescriptorParticle.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <framework/dataobjects/EventExtraInfo.h>

#include <mva/interface/Interface.h>

#include <boost/algorithm/string/predicate.hpp>

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
           "Name under which the output of the expert is stored in the ExtraInfo of the Particle object. If the expert returns multiple values, the index of the value is appended to the name in the form '_0', '_1', ...");
  addParam("identifier", m_identifier, "The database identifier which is used to load the weights during the training.");
  addParam("signalFraction", m_signal_fraction_override,
           "signalFraction to calculate probability (if -1 the signalFraction of the training data is used)", -1.0);
  addParam("overwriteExistingExtraInfo", m_overwriteExistingExtraInfo,
           "-1/0/1/2: Overwrite if lower / don't overwrite / overwrite if higher / always overwrite, in case the extra info with given name already exists",
           2);
}

void MVAExpertModule::initialize()
{
  // All specified ParticleLists are required to exist
  for (auto& name : m_listNames) {
    DecayDescriptor dd;
    bool valid = dd.init(name);
    if (!valid) {
      B2ERROR("Decay string " << name << " is invalid.");
    }
    const DecayDescriptorParticle* mother = dd.getMother();
    uint nSelectedDaughters = dd.getSelectionNames().size();
    if (nSelectedDaughters > 1) {
      B2ERROR("More than one daughter is selected in the decay string " << name << ".");
    }
    StoreObjPtr<ParticleList> list(mother->getFullName());
    list.isRequired();
    m_targetListNames.push_back(mother->getFullName());
    m_decaydescriptors.insert(std::make_pair(mother->getFullName(), dd));
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
  m_nClasses = general_options.m_nClasses;
}

void MVAExpertModule::fillDataset(const Particle* particle)
{
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
}

float MVAExpertModule::analyse(const Particle* particle)
{
  if (not m_expert) {
    B2ERROR("MVA Expert is not loaded! I will return 0");
    return 0.0;
  }
  fillDataset(particle);
  return m_expert->apply(*m_dataset)[0];
}

std::vector<float> MVAExpertModule::analyseMulticlass(const Particle* particle)
{
  if (not m_expert) {
    B2ERROR("MVA Expert is not loaded! I will return 0");
    return std::vector<float>(m_nClasses, 0.0);
  }
  fillDataset(particle);
  return m_expert->applyMulticlass(*m_dataset)[0];
}

void MVAExpertModule::setExtraInfoField(Particle* particle, std::string extraInfoName, float responseValue)
{
  if (particle->hasExtraInfo(extraInfoName)) {
    if (particle->getExtraInfo(extraInfoName) != responseValue) {
      m_existGivenExtraInfo = true;
      double current = particle->getExtraInfo(extraInfoName);
      if (m_overwriteExistingExtraInfo == -1) {
        if (responseValue < current) particle->setExtraInfo(extraInfoName, responseValue);
      } else if (m_overwriteExistingExtraInfo == 0) {
        // don't overwrite!
      } else if (m_overwriteExistingExtraInfo == 1) {
        if (responseValue > current) particle->setExtraInfo(extraInfoName, responseValue);
      } else if (m_overwriteExistingExtraInfo == 2) {
        particle->setExtraInfo(extraInfoName, responseValue);
      } else {
        B2FATAL("m_overwriteExistingExtraInfo must be one of {-1,0,1,2}. Received '" << m_overwriteExistingExtraInfo << "'.");
      }
    }
  } else {
    particle->addExtraInfo(extraInfoName, responseValue);
  }
}

void MVAExpertModule::setEventExtraInfoField(StoreObjPtr<EventExtraInfo> eventExtraInfo, std::string extraInfoName,
                                             float responseValue)
{
  if (eventExtraInfo->hasExtraInfo(extraInfoName)) {
    m_existGivenExtraInfo = true;
    double current = eventExtraInfo->getExtraInfo(extraInfoName);
    if (m_overwriteExistingExtraInfo == -1) {
      if (responseValue < current) eventExtraInfo->setExtraInfo(extraInfoName, responseValue);
    } else if (m_overwriteExistingExtraInfo == 0) {
      // don't overwrite!
    } else if (m_overwriteExistingExtraInfo == 1) {
      if (responseValue > current) eventExtraInfo->setExtraInfo(extraInfoName, responseValue);
    } else if (m_overwriteExistingExtraInfo == 2) {
      eventExtraInfo->setExtraInfo(extraInfoName, responseValue);
    } else {
      B2FATAL("m_overwriteExistingExtraInfo must be one of {-1,0,1,2}. Received '" << m_overwriteExistingExtraInfo << "'.");
    }
  } else {
    eventExtraInfo->addExtraInfo(extraInfoName, responseValue);
  }
}

void MVAExpertModule::event()
{
  for (auto& listName : m_targetListNames) {
    StoreObjPtr<ParticleList> list(listName);
    // Calculate target Value for Particles
    for (unsigned i = 0; i < list->getListSize(); ++i) {
      auto dd = m_decaydescriptors[listName];
      uint nSelectedDaughters = dd.getSelectionNames().size();
      const Particle* particle = (nSelectedDaughters > 0) ? dd.getSelectionParticles(list->getParticle(i))[0] : list->getParticle(i);
      if (m_nClasses == 2) {
        float responseValue = analyse(particle);
        setExtraInfoField(m_particles[particle->getArrayIndex()], m_extraInfoName, responseValue);
      } else if (m_nClasses > 2) {
        std::vector<float> responseValues = analyseMulticlass(particle);
        if (responseValues.size() != m_nClasses) {
          B2ERROR("Size of results returned by MVA Expert applyMulticlass (" << responseValues.size() <<
                  ") does not match the declared number of classes (" << m_nClasses << ").");
        }
        for (unsigned int iClass = 0; iClass < m_nClasses; iClass++) {
          setExtraInfoField(m_particles[particle->getArrayIndex()], m_extraInfoName + "_" + std::to_string(iClass), responseValues[iClass]);
        }
      } else {
        B2ERROR("Received a value of " << m_nClasses <<
                " for the number of classes considered by the MVA Expert. This value should be >=2.");
      }
    }
  }
  if (m_listNames.empty()) {
    StoreObjPtr<EventExtraInfo> eventExtraInfo;
    if (not eventExtraInfo.isValid())
      eventExtraInfo.create();

    if (m_nClasses == 2) {
      float responseValue = analyse(nullptr);
      setEventExtraInfoField(eventExtraInfo, m_extraInfoName, responseValue);
    } else if (m_nClasses > 2) {
      std::vector<float> responseValues = analyseMulticlass(nullptr);
      if (responseValues.size() != m_nClasses) {
        B2ERROR("Size of results returned by MVA Expert applyMulticlass (" << responseValues.size() <<
                ") does not match the declared number of classes (" << m_nClasses << ").");
      }
      for (unsigned int iClass = 0; iClass < m_nClasses; iClass++) {
        setEventExtraInfoField(eventExtraInfo, m_extraInfoName + "_" + std::to_string(iClass), responseValues[iClass]);
      }
    } else {
      B2ERROR("Received a value of " << m_nClasses <<
              " for the number of classes considered by the MVA Expert. This value should be >=2.");
    }
  }
}

void MVAExpertModule::terminate()
{
  m_expert.reset();
  m_dataset.reset();

  if (m_existGivenExtraInfo) {
    if (m_overwriteExistingExtraInfo == -1) {
      B2WARNING("The extraInfo " << m_extraInfoName <<
                " has already been set! It was overwritten by this module if the new value was lower than the previous!");
    } else if (m_overwriteExistingExtraInfo == 0) {
      B2WARNING("The extraInfo " << m_extraInfoName <<
                " has already been set! The original value was kept and this module did not overwrite it!");
    } else if (m_overwriteExistingExtraInfo == 1) {
      B2WARNING("The extraInfo " << m_extraInfoName <<
                " has already been set! It was overwritten by this module if the new value was higher than the previous!");
    } else if (m_overwriteExistingExtraInfo == 2) {
      B2WARNING("The extraInfo " << m_extraInfoName << " has already been set! It was overwritten by this module!");
    }
  }
}