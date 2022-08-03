/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <mva/modules/MVAExpert/MVAMultipleExpertsModule.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <framework/dataobjects/EventExtraInfo.h>

#include <mva/interface/Interface.h>

#include <boost/algorithm/string/predicate.hpp>

#include <framework/logging/Logger.h>


using namespace Belle2;

REG_MODULE(MVAMultipleExperts);

MVAMultipleExpertsModule::MVAMultipleExpertsModule() : Module()
{
  setDescription("Adds ExtraInfos to the Particle objects in given ParticleLists which is calcuated by multiple experts defined by the given weightfiles.");
  setPropertyFlags(c_ParallelProcessingCertified);

  std::vector<std::string> empty;
  addParam("listNames", m_listNames,
           "Particles from these ParticleLists are used as input. If no name is given the experts are applied to every event once, and one can only use variables which accept nullptr as Particle*",
           empty);
  addParam("extraInfoNames", m_extraInfoNames,
           "Names under which the output of the experts is stored in the ExtraInfo of the Particle object.");
  addParam("identifiers", m_identifiers, "The database identifiers which is used to load the weights during the training.");
  addParam("signalFraction", m_signal_fraction_override,
           "signalFraction to calculate probability (if -1 the signalFraction of the training data is used)", -1.0);
  std::vector<int> empty_vec;
  addParam("overwriteExistingExtraInfo", m_overwriteExistingExtraInfo,
           "If true, when the given extraInfo has already defined, the old extraInfo value is overwritten. If false, the original value is kept.",
           empty_vec);
}

void MVAMultipleExpertsModule::initialize()
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

  if (m_extraInfoNames.size() != m_identifiers.size()) {
    B2FATAL("The number of given m_extraInfoNames is not equal to the number of m_identifiers. The output the ith method in m_identifiers is saved as extraInfo under the ith name in m_extraInfoNames! Set also different names for each method!");
  }

  m_weightfile_representations.resize(m_identifiers.size());
  m_experts.resize(m_identifiers.size());
  m_individual_feature_variables.resize(m_identifiers.size());
  m_datasets.resize(m_identifiers.size());
  m_nClasses.resize(m_identifiers.size());
  // if the size of m_overwriteExistingExtraInfo is smaller than that of m_identifiers, 2 will be filled.
  m_overwriteExistingExtraInfo.resize(m_identifiers.size(), 2);
  m_existGivenExtraInfo.resize(m_identifiers.size(), false);

  for (unsigned int i = 0; i < m_identifiers.size(); ++i) {
    if (not(boost::ends_with(m_identifiers[i], ".root") or boost::ends_with(m_identifiers[i], ".xml"))) {
      m_weightfile_representations[i] = std::make_unique<DBObjPtr<DatabaseRepresentationOfWeightfile>>(
                                          MVA::makeSaveForDatabase(m_identifiers[i]));
    }
  }

  MVA::AbstractInterface::initSupportedInterfaces();
}

void MVAMultipleExpertsModule::beginRun()
{

  if (!m_weightfile_representations.empty()) {
    for (unsigned int i = 0; i < m_weightfile_representations.size(); ++i) {
      if (m_weightfile_representations[i]) {
        if (m_weightfile_representations[i]->hasChanged()) {
          std::stringstream ss((*m_weightfile_representations[i])->m_data);
          auto weightfile = MVA::Weightfile::loadFromStream(ss);
          init_mva(weightfile, i);
        }
      } else {
        auto weightfile = MVA::Weightfile::loadFromFile(m_identifiers[i]);
        init_mva(weightfile, i);
      }
    }

  } else B2FATAL("No m_identifiers given. At least one is needed!");
}

void MVAMultipleExpertsModule::init_mva(MVA::Weightfile& weightfile, unsigned int i)
{

  auto supported_interfaces = MVA::AbstractInterface::getSupportedInterfaces();
  Variable::Manager& manager = Variable::Manager::Instance();


  MVA::GeneralOptions general_options;
  weightfile.getOptions(general_options);

  // Overwrite signal fraction from training
  if (m_signal_fraction_override > 0)
    weightfile.addSignalFraction(m_signal_fraction_override);

  m_experts[i] = supported_interfaces[general_options.m_method]->getExpert();
  m_experts[i]->load(weightfile);


  m_individual_feature_variables[i] =  manager.getVariables(general_options.m_variables);
  if (m_individual_feature_variables[i].size() != general_options.m_variables.size()) {
    B2FATAL("One or more feature variables could not be loaded via the Variable::Manager. Check the names!");
  }

  for (auto& iVariable : m_individual_feature_variables[i]) {
    if (m_feature_variables.find(iVariable) == m_feature_variables.end()) {
      m_feature_variables.insert(std::pair<const Variable::Manager::Var*, float>(iVariable, 0));
    }
  }

  std::vector<float> dummy;
  dummy.resize(m_individual_feature_variables[i].size(), 0);
  m_datasets[i] = std::make_unique<MVA::SingleDataset>(general_options, dummy, 0);

  m_nClasses[i] = general_options.m_nClasses;

}

void MVAMultipleExpertsModule::fillDatasets(Particle* particle)
{
  for (auto const& iVariable : m_feature_variables) {
    if (iVariable.first->variabletype == Variable::Manager::VariableDataType::c_double) {
      m_feature_variables[iVariable.first] = std::get<double>(iVariable.first->function(particle));
    } else if (iVariable.first->variabletype == Variable::Manager::VariableDataType::c_int) {
      m_feature_variables[iVariable.first] = std::get<int>(iVariable.first->function(particle));
    } else if (iVariable.first->variabletype == Variable::Manager::VariableDataType::c_bool) {
      m_feature_variables[iVariable.first] = std::get<bool>(iVariable.first->function(particle));
    }
  }

  for (unsigned int i = 0; i < m_identifiers.size(); ++i) {
    for (unsigned int j = 0; j < m_individual_feature_variables[i].size(); ++j) {
      m_datasets[i]->m_input[j] = m_feature_variables[m_individual_feature_variables[i][j]];
    }
  }
}

std::vector<std::vector<float>> MVAMultipleExpertsModule::analyse(Particle* particle)
{
  std::vector<std::vector<float>> responseValues;
  fillDatasets(particle);

  for (unsigned int i = 0; i < m_identifiers.size(); ++i) {
    if (m_nClasses[i] == 2) {
      responseValues.push_back({m_experts[i]->apply(*m_datasets[i])[0],});
    } else if (m_nClasses[i] > 2) {
      responseValues.push_back(m_experts[i]->applyMulticlass(*m_datasets[i])[0]);
    } else {
      B2ERROR("Received a value of " << m_nClasses[i] <<
              " for the number of classes considered by the MVA Expert. This value should be >=2.");
    }
  }
  return responseValues;
}

void MVAMultipleExpertsModule::setExtraInfoField(Particle* particle, std::string extraInfoName, float responseValue, unsigned int i)
{
  if (particle->hasExtraInfo(extraInfoName)) {
    if (particle->getExtraInfo(extraInfoName) != responseValue) {
      m_existGivenExtraInfo[i] = true;
      double current = particle->getExtraInfo(extraInfoName);
      if (m_overwriteExistingExtraInfo[i] == -1) {
        if (responseValue < current) particle->setExtraInfo(extraInfoName, responseValue);
      } else if (m_overwriteExistingExtraInfo[i] == 0) {
        // don't overwrite!
      } else if (m_overwriteExistingExtraInfo[i] == 1) {
        if (responseValue > current) particle->setExtraInfo(extraInfoName, responseValue);
      } else if (m_overwriteExistingExtraInfo[i] == 2) {
        particle->setExtraInfo(extraInfoName, responseValue);
      } else {
        B2FATAL("m_overwriteExistingExtraInfo must be one of {-1,0,1,2}. Received '" << m_overwriteExistingExtraInfo[i] << "'.");
      }
    }
  } else {
    particle->addExtraInfo(extraInfoName, responseValue);
  }
}

void MVAMultipleExpertsModule::setEventExtraInfoField(StoreObjPtr<EventExtraInfo> eventExtraInfo, std::string extraInfoName,
                                                      float responseValue, unsigned int i)
{
  if (eventExtraInfo->hasExtraInfo(extraInfoName)) {
    m_existGivenExtraInfo[i] = true;
    double current = eventExtraInfo->getExtraInfo(extraInfoName);
    if (m_overwriteExistingExtraInfo[i] == -1) {
      if (responseValue < current) eventExtraInfo->setExtraInfo(extraInfoName, responseValue);
    } else if (m_overwriteExistingExtraInfo[i] == 0) {
      // don't overwrite!
    } else if (m_overwriteExistingExtraInfo[i] == 1) {
      if (responseValue > current) eventExtraInfo->setExtraInfo(extraInfoName, responseValue);
    } else if (m_overwriteExistingExtraInfo[i] == 2) {
      eventExtraInfo->setExtraInfo(extraInfoName, responseValue);
    } else {
      B2FATAL("m_overwriteExistingExtraInfo must be one of {-1,0,1,2}. Received '" << m_overwriteExistingExtraInfo[i] << "'.");
    }
  } else {
    eventExtraInfo->addExtraInfo(extraInfoName, responseValue);
  }
}

void MVAMultipleExpertsModule::event()
{
  for (auto& listName : m_listNames) {
    StoreObjPtr<ParticleList> list(listName);
    // Calculate target Value for Particles
    for (unsigned i = 0; i < list->getListSize(); ++i) {
      Particle* particle = list->getParticle(i);
      std::vector<std::vector<float>> responseValues = analyse(particle);
      for (unsigned int j = 0; j < m_identifiers.size(); ++j) {
        if (m_nClasses[j] == 2) {
          setExtraInfoField(particle, m_extraInfoNames[j], responseValues[j][0], j);
        } else if (m_nClasses[j] > 2) {
          if (responseValues[j].size() != m_nClasses[j]) {
            B2ERROR("Size of results returned by MVA Expert applyMulticlass (" << responseValues[j].size() <<
                    ") does not match the declared number of classes (" << m_nClasses[j] << ").");
          }
          for (unsigned int iClass = 0; iClass < m_nClasses[j]; iClass++) {
            setExtraInfoField(particle, m_extraInfoNames[j] + "_" + std::to_string(iClass), responseValues[j][iClass], j);
          }
        } else {
          B2ERROR("Received a value of " << m_nClasses[j] <<
                  " for the number of classes considered by the MVA Expert. This value should be >=2.");
        }
      } //identifiers
    }
  } // listnames
  if (m_listNames.empty()) {
    StoreObjPtr<EventExtraInfo> eventExtraInfo;
    if (not eventExtraInfo.isValid())
      eventExtraInfo.create();
    std::vector<std::vector<float>> responseValues = analyse(nullptr);
    for (unsigned int j = 0; j < m_identifiers.size(); ++j) {
      if (m_nClasses[j] == 2) {
        setEventExtraInfoField(eventExtraInfo, m_extraInfoNames[j], responseValues[j][0], j);
      } else if (m_nClasses[j] > 2) {
        if (responseValues[j].size() != m_nClasses[j]) {
          B2ERROR("Size of results returned by MVA Expert applyMulticlass (" << responseValues[j].size() <<
                  ") does not match the declared number of classes (" << m_nClasses[j] << ").");
        }
        for (unsigned int iClass = 0; iClass < m_nClasses[j]; iClass++) {
          setEventExtraInfoField(eventExtraInfo, m_extraInfoNames[j] + "_" + std::to_string(iClass), responseValues[j][iClass], j);
        }
      } else {
        B2ERROR("Received a value of " << m_nClasses[j] <<
                " for the number of classes considered by the MVA Expert. This value should be >=2.");
      }
    } //identifiers
  }
}

void MVAMultipleExpertsModule::terminate()
{
  for (unsigned int i = 0; i < m_identifiers.size(); ++i) {
    m_experts[i].reset();
    m_datasets[i].reset();

    if (m_existGivenExtraInfo[i]) {
      if (m_overwriteExistingExtraInfo[i] == -1) {
        B2WARNING("The extraInfo " << m_extraInfoNames[i] <<
                  " has already been set! It was overwritten by this module if the new value was lower than the previous!");
      } else if (m_overwriteExistingExtraInfo[i] == 0) {
        B2WARNING("The extraInfo " << m_extraInfoNames[i] <<
                  " has already been set! The original value was kept and this module did not overwrite it!");
      } else if (m_overwriteExistingExtraInfo[i] == 1) {
        B2WARNING("The extraInfo " << m_extraInfoNames[i] <<
                  " has already been set! It was overwritten by this module if the new value was higher than the previous!");
      } else if (m_overwriteExistingExtraInfo[i] == 2) {
        B2WARNING("The extraInfo " << m_extraInfoNames[i] << " has already been set! It was overwritten by this module!");
      }
    }
  }
}
