/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/TMVATeacher/TMVATeacherModule.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/logging/Logger.h>

namespace Belle2 {


  REG_MODULE(TMVATeacher)

  TMVATeacherModule::TMVATeacherModule() : Module(), m_target_var(nullptr)
  {
    setDescription("Trains TMVA method with given particle lists as training samples. "
                   "The target variable has to be an integer valued variable which defines the clusters in the sample. "
                   "e.g. isSignal for signal and background cluster or abs_PDG to define different MC-PDGs as clusters. "
                   "The clusters are trained against each other. "
                   "See also https://belle2.cc.kek.jp/~twiki/bin/view/Software/TMVA for detailed instructions.");

    setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

    std::vector<std::string> empty;
    addParam("listNames", m_listNames, "Particles from these ParticleLists are used as input. If no name is given the teacher is applied to every event once, and one can only use variables which accept nullptr as Particle*", empty);
    addParam("methods", m_methods, "Vector of Tuples with (Name, Type, Config) of the methods. Valid types are: BDT, KNN, Fisher, Plugin. For type 'Plugin', the plugin matching the Name attribute will be loaded (e.g. NeuroBayes). The Config is passed to the TMVA Method and is documented in the TMVA UserGuide.");
    addParam("prefix", m_methodPrefix, "Prefix which is used by the TMVAInterface to store its configfile $prefix.config and by TMVA itself to write the files weights/$prefix_$method.class.C and weights/$prefix_$method.weights.xml with additional information", std::string("TMVA"));
    addParam("workingDirectory", m_workingDirectory, "Working directory in which the config file and the weight file directory is created", std::string("."));
    addParam("variables", m_variables, "Input variables used by the TMVA method");
    addParam("spectators", m_spectators, "Input spectators used by the TMVA method. These variables are saved in the output file, but not used as training input.", empty);
    addParam("target", m_target, "Target used by the method, has to be an integer-valued variable which defines clusters in the sample.");
    addParam("factoryOption", m_factoryOption, "Option passed to TMVA::Factory", std::string("!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification"));
    addParam("prepareOption", m_prepareOption, "Option passed to TMVA::Factory::PrepareTrainingAndTestTree", std::string("SplitMode=random:!V"));
    addParam("createMVAPDFs", m_createMVAPDFs, "Creates the MVA PDFs for signal and background. This is needed to transform the output of the trained method to a probability.", true);
    addParam("useExistingData", m_useExistingData, "Use existing data which is already stored in the $prefix.root file.", false);
    addParam("doNotTrain", m_doNotTrain, "Do not train, create only datafile with samples. Useful if you want to train outside of basf2 with the externTeacher tool.", false);
    addParam("maxEventsPerClass", m_maxEventsPerClass, "Maximum number of events per class passed to TMVA. 0 means no limit.", static_cast<unsigned int>(0));
    std::map<int, unsigned int> defaultInverseSamplingRates;
    addParam("inverseSamplingRates", m_inverseSamplingRates, "Map of class id and inverse sampling rate for this class.", defaultInverseSamplingRates);

  }

  TMVATeacherModule::~TMVATeacherModule()
  {
  }

  void TMVATeacherModule::initialize()
  {
    for (auto & name : m_listNames) {
      StoreObjPtr<ParticleList>::required(name);
    }

    std::vector<TMVAInterface::Method> methods;
    for (auto & x : m_methods) {
      std::string config = std::get<2>(x);
      if (m_createMVAPDFs)
        config = std::string("CreateMVAPdfs:") + config;
      methods.push_back(TMVAInterface::Method(std::get<0>(x), std::get<1>(x), config, m_variables, m_spectators));
    }
    m_teacher = std::make_shared<TMVAInterface::Teacher>(m_methodPrefix, m_workingDirectory, m_target, methods, m_useExistingData);

    Variable::Manager& manager = Variable::Manager::Instance();
    m_target_var = manager.getVariable(m_target);

  }

  void TMVATeacherModule::terminate()
  {
    if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
      if (not m_doNotTrain) {
        m_teacher->train(m_factoryOption, m_prepareOption, m_maxEventsPerClass);
      }
      m_teacher->writeTree();
    }
    m_teacher.reset();
  }

  bool TMVATeacherModule::checkSampling(int target)
  {
    if (m_inverseSamplingRates.find(target) != m_inverseSamplingRates.end()) {
      if (m_iSamples.find(target) == m_iSamples.end()) {
        m_iSamples[target] = 0;
      }
      m_iSamples[target]++;
      if (m_iSamples[target] % m_inverseSamplingRates[target] != 0)
        return false;
      else
        m_iSamples[target] = 0;
    }
    return true;
  }

  void TMVATeacherModule::event()
  {
    for (auto & listName : m_listNames) {
      StoreObjPtr<ParticleList> list(listName);
      // Calculate Signal Probability for Particles
      for (unsigned i = 0; i < list->getListSize(); ++i) {
        const Particle* particle = list->getParticle(i);
        int target = int(m_target_var->function(particle) + 0.5);
        if (checkSampling(target)) {
          m_teacher->addClassSample(particle, target);
        }
      }
    }

    if (m_listNames.empty()) {
      int target = int(m_target_var->function(nullptr) + 0.5);
      if (checkSampling(target)) {
        m_teacher->addClassSample(nullptr, target);
      }
    }
  }

} // Belle2 namespace

