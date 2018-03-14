/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck and Fernando Abudinen                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <mva/modules/MVAExpert/MVAMultipleExpertsModule.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <analysis/dataobjects/EventExtraInfo.h>

#include <mva/interface/Interface.h>

#include <boost/algorithm/string/predicate.hpp>

#include <framework/logging/Logger.h>


namespace Belle2 {

  REG_MODULE(MVAMultipleExperts)

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

    for (unsigned int i = 0; i < m_identifiers.size(); ++i) {
      if (not(boost::ends_with(m_identifiers[i], ".root") or boost::ends_with(m_identifiers[i], ".xml"))) {
        m_weightfile_representations[i] = (std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>(new
                                           DBObjPtr<DatabaseRepresentationOfWeightfile>(MVA::makeSaveForDatabase(m_identifiers[i]))));
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
    m_datasets[i] = std::unique_ptr<MVA::SingleDataset>(new MVA::SingleDataset(general_options, dummy, 0));

  }

  std::vector<float> MVAMultipleExpertsModule::analyse(Particle* particle)
  {

    std::vector<float> targetValues;
    targetValues.resize(m_identifiers.size());
    for (auto const& iVariable : m_feature_variables) {
      m_feature_variables[iVariable.first] = iVariable.first ->function(particle);
    }

    for (unsigned int i = 0; i < m_identifiers.size(); ++i) {
      for (unsigned int j = 0; j < m_individual_feature_variables[i].size(); ++j) {
        m_datasets[i]->m_input[j] = m_feature_variables[m_individual_feature_variables[i][j]];
      }
      targetValues[i] = m_experts[i]->apply(*m_datasets[i])[0];
    }

    return targetValues;
  }


  void MVAMultipleExpertsModule::event()
  {
    for (auto& listName : m_listNames) {
      StoreObjPtr<ParticleList> list(listName);
      // Calculate target Value for Particles
      for (unsigned i = 0; i < list->getListSize(); ++i) {
        Particle* particle = list->getParticle(i);
        std::vector<float> targetValues = analyse(particle);

        for (unsigned int j = 0; j < m_identifiers.size(); ++j) {
          if (particle->hasExtraInfo(m_extraInfoNames[j])) {
            B2WARNING("Extra Info with given name is already set! Overwriting old value!");
            particle->setExtraInfo(m_extraInfoNames[j], targetValues[j]);
          } else {
            particle->addExtraInfo(m_extraInfoNames[j], targetValues[j]);
          }
        }
      }
    }
    if (m_listNames.empty()) {
      StoreObjPtr<EventExtraInfo> eventExtraInfo;
      if (not eventExtraInfo.isValid())
        eventExtraInfo.create();
      std::vector<float> targetValues = analyse(nullptr);
      for (unsigned int j = 0; j < m_identifiers.size(); ++j) {
        if (eventExtraInfo->hasExtraInfo(m_extraInfoNames[j])) {
          B2WARNING("Extra Info with given name is already set! I won't set it again!");
        } else {
          eventExtraInfo->addExtraInfo(m_extraInfoNames[j], targetValues[j]);
        }
      }
    }
  }

} // Belle2 namespace

