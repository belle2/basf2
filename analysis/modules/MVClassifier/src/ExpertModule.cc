/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/modules/MVClassifier/ExpertModule.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <analysis/dataobjects/EventExtraInfo.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <analysis/VariableManager/Utility.h>

#include <analysis/utility/WorkingDirectoryManager.h>
#include <framework/logging/Logger.h>

#include <TSystem.h>
#include <TMVA/Tools.h>
#include <TMVA/Types.h>
#include <TPluginManager.h>

#include <fstream>

namespace Belle2 {


  REG_MODULE(Expert)

  ExpertModule::ExpertModule() : Module(), m_signal_fraction(0.0), m_expert_type(UnkownType)
  {
    setDescription("Adds an ExtraInfo to the Particle objects in given ParticleLists which is calcuated by an expert defined by a weightfile.");
    setPropertyFlags(c_ParallelProcessingCertified);

    std::vector<std::string> empty;
    addParam("listNames", m_listNames,
             "Particles from these ParticleLists are used as input. If no name is given the expert is applied to every event once, and one can only use variables which accept nullptr as Particle*",
             empty);
    addParam("extraInfoName", m_extraInfoName,
             "Name under which the output of the expert is stored in the ExtraInfo of the Particle object.");
    addParam("weightfile", m_weightfile, "The weightfile which is used.");
    addParam("signalFraction", m_signal_fraction_override,
             "signalFraction to calculate probability (if -1 the signalFraction of the training data is used)", -1.0);
    addParam("transformToProbability", m_transform2probability,
             "Transform classifier output to a probability using given signalFraction."
             "WARNING: If you want to use this feature you have to set the options createMVAPDFs and NbinsMVAPdf in the method config string provided to TMVATeacher to reasonable values!"
             "The default values of TMVA will result in a unusable classifier output!", true);
  }

  void ExpertModule::initialize()
  {
    // All specified ParticleLists are required to exist
    for (auto& name : m_listNames) {
      StoreObjPtr<ParticleList>::required(name);
    }

    if (m_listNames.empty()) {
      StoreObjPtr<EventExtraInfo>::registerPersistent("", DataStore::c_Event, false);
    } else {
      StoreObjPtr<ParticleExtraInfoMap>::registerPersistent("", DataStore::c_Event, false); //allow re-registration
    }

    switch (m_expert_type) {
      case TMVAExpert: return initializeTMVA();
      case TensorflowExpert: return initializeTensorflow();
      case FastBDTExpert: return initializeFastBDT();
      case NeuroBayesExpert: return initializeNeuroBayes();
      case UnkownType: throw std::runtime_error("Unkown expert type");
    }

  }

  void ExpertModule::initializeTMVA()
  {

    TMVA::Tools::Instance();

  }

  void ExpertModule::initializeFastBDT()
  {
    B2FATAL("FastBDT is not implemented yet");
  }

  void ExpertModule::initializeTensorflow()
  {
    B2FATAL("Tensorflow is not implemented yet");
  }

  void ExpertModule::initializeNeuroBayes()
  {
    B2FATAL("NeuroBayes is not implemented yet");
  }

  void ExpertModule::beginRun()
  {

    std::ifstream configstream(m_weightfile);
    if (not configstream.good()) {
      B2FATAL("Couldn't open Expert weightfile " << m_weightfile);
    }

    boost::property_tree::ptree pt;
    boost::property_tree::xml_parser::read_xml(configstream, pt);

    for (const auto& f : pt.get_child("Features")) {
      if (f.first.data() != std::string("Feature"))
        continue;
      m_feature_names.push_back(f.second.get<std::string>("Name"));
    }

    for (const auto& f : pt.get_child("Spectators")) {
      if (f.first.data() != std::string("Spectator"))
        continue;
      m_spectator_names.push_back(f.second.get<std::string>("Name"));
    }

    Variable::Manager& manager = Variable::Manager::Instance();

    m_expert_type = static_cast<Type>(pt.get<int>("ExpertType"));

    if (m_signal_fraction_override < 0) {
      m_signal_fraction = pt.get<double>("SignalFraction");
    } else {
      m_signal_fraction = m_signal_fraction_override;
    }

    m_feature_variables =  manager.getVariables(m_feature_names);
    if (m_feature_variables.size() != m_feature_names.size()) {
      B2FATAL("One or more feature variables could not be loaded via the Variable::Manager. Check the names!");
    }

    m_spectator_variables =  manager.getVariables(m_feature_names);
    if (m_spectator_variables.size() != m_spectator_names.size()) {
      B2ERROR("One or more spectator variables could not be loaded via the Variable::Manager. Check the names!");
    }

    m_feature_input.resize(m_feature_names.size(), 0);
    m_spectator_input.resize(m_spectator_names.size(), 0);

    switch (m_expert_type) {
      case TMVAExpert: return beginRunTMVA(pt);
      case TensorflowExpert: return beginRunTensorflow(pt);
      case FastBDTExpert: return beginRunFastBDT(pt);
      case NeuroBayesExpert: return beginRunNeuroBayes(pt);
      case UnkownType: throw std::runtime_error("Unkown expert type");
    }

  }

  void ExpertModule::beginRunTMVA(const boost::property_tree::ptree& pt)
  {

    m_tmva_type = pt.get<std::string>("TMVAType");
    m_tmva_method = pt.get<std::string>("TMVAMethod");
    m_tmva_weightfile = pt.get<std::string>("TMVAWeightfile");
    m_tmva_working_directory = pt.get<std::string>("TMVAWorkingDirectory");

    WorkingDirectoryManager dummy(m_tmva_working_directory);

    // Automatically load Plugin if necessary. The given name has to correspond to the method name
    if (m_tmva_type == "Plugins" or m_tmva_type == "Plugin") {
      m_tmva_type = "Plugins";
      auto base = std::string("TMVA@@MethodBase");
      auto regexp1 = std::string(".*_") + m_tmva_method + std::string(".*");
      auto regexp2 = std::string(".*") + m_tmva_method + std::string(".*");
      auto className = std::string("TMVA::Method") + m_tmva_method;
      auto ctor1 = std::string("Method") + m_tmva_method + std::string("(TMVA::DataSetInfo&,TString)");
      auto ctor2 = std::string("Method") + m_tmva_method + std::string("(TString&,TString&,TMVA::DataSetInfo&,TString&)");
      auto pluginName = (m_tmva_method == "MockPlugin") ? std::string("analysis_TMVA") + m_tmva_method : std::string("TMVA") +
                        m_tmva_method;

      gPluginMgr->AddHandler(base.c_str(), regexp1.c_str(), className.c_str(), pluginName.c_str(), ctor1.c_str());
      gPluginMgr->AddHandler(base.c_str(), regexp2.c_str(), className.c_str(), pluginName.c_str(), ctor2.c_str());
      B2INFO("Loaded plugin " << m_tmva_method);
    }

    m_tmva_reader = std::unique_ptr<TMVA::Reader>(new TMVA::Reader("!Color:!Silent"));

    for (unsigned int i = 0; i < m_feature_names.size(); ++i) {
      m_tmva_reader->AddVariable(Variable::makeROOTCompatible(m_feature_names[i]), &m_feature_input[i]);
    }
    for (unsigned int i = 0; i < m_spectator_names.size(); ++i) {
      m_tmva_reader->AddSpectator(Variable::makeROOTCompatible(m_spectator_names[i]), &m_spectator_input[i]);
    }

    if (!m_tmva_reader->BookMVA(m_tmva_method, m_tmva_weightfile)) {
      B2FATAL("Could not set up expert! Please see preceding error message from TMVA!");
    }
  }

  void ExpertModule::beginRunFastBDT(const boost::property_tree::ptree&)
  {
    B2FATAL("FastBDT is not implemented yet");
  }

  void ExpertModule::beginRunTensorflow(const boost::property_tree::ptree&)
  {
    B2FATAL("Tensorflow is not implemented yet");
  }

  void ExpertModule::beginRunNeuroBayes(const boost::property_tree::ptree&)
  {
    B2FATAL("NeuroBayes is not implemented yet");
  }

  float ExpertModule::analyse(Particle* particle)
  {

    for (unsigned int i = 0; i < m_feature_variables.size(); ++i) {
      m_feature_input[i] = m_feature_variables[i]->function(particle);
    }

    for (unsigned int i = 0; i < m_spectator_variables.size(); ++i) {
      m_spectator_input[i] = m_spectator_variables[i]->function(particle);
    }

    switch (m_expert_type) {
      case TMVAExpert: return analyseTMVA();
      case TensorflowExpert: return analyseTensorflow();
      case FastBDTExpert: return analyseFastBDT();
      case NeuroBayesExpert: return analyseNeuroBayes();
      case UnkownType: throw std::runtime_error("Unkown expert type");
    }
    return 0.0;

  }

  float ExpertModule::analyseTMVA()
  {

    double result = 0;
    if (m_transform2probability)
      result = m_tmva_reader->GetProba(m_tmva_method, m_signal_fraction);
    else
      result = m_tmva_reader->EvaluateMVA(m_tmva_method);

    if (result == -999) {
      B2WARNING("TMVA returned -999, which indicates that something went wrong while applying the MVA method, check previous messages for more information. In consequence the returned probability is not constrained to [0,1]!");
    }

    return result;
  }

  float ExpertModule::analyseFastBDT()
  {
    B2FATAL("FastBDT is not implemented yet");
    return 0.0;
  }

  float ExpertModule::analyseTensorflow()
  {
    B2FATAL("Tensorflow is not implemented yet");
    return 0.0;
  }

  float ExpertModule::analyseNeuroBayes()
  {
    B2FATAL("NeuroBayes is not implemented yet");
    return 0.0;
  }

  void ExpertModule::endRun()
  {
  }

  void ExpertModule::terminate()
  {
  }


  void ExpertModule::event()
  {
    for (auto& listName : m_listNames) {
      StoreObjPtr<ParticleList> list(listName);
      // Calculate target Value for Particles
      for (unsigned i = 0; i < list->getListSize(); ++i) {
        Particle* particle = list->getParticle(i);
        float targetValue = analyse(particle);
        if (particle->hasExtraInfo(m_extraInfoName)) {
          B2WARNING("Extra Info with given name is already set! Overwriting old value!");
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
      if (eventExtraInfo->hasExtraInfo(m_extraInfoName)) {
        B2WARNING("Extra Info with given name is already set! I won't set it again!");
      } else {
        float targetValue = analyse(nullptr);
        eventExtraInfo->addExtraInfo(m_extraInfoName, targetValue);
      }
    }
  }

} // Belle2 namespace

