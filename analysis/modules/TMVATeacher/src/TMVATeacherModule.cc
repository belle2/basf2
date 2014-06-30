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
#include <framework/logging/Logger.h>

namespace Belle2 {


  REG_MODULE(TMVATeacher)

  TMVATeacherModule::TMVATeacherModule() : Module()
  {
    setDescription("Trains TMVA method with given particle lists as training samples. "
                   "The target variable has to be an integer valued variable which defines the clusters in the sample. "
                   "e.g. isSignal for signal and background cluster or abs_PDG to define different MC-PDGs as clusters. "
                   "The clusters are trained against each other. "
                   "See also https://belle2.cc.kek.jp/~twiki/bin/view/Software/TMVA for detailed instructions.");
    //setPropertyFlags(c_ParallelProcessingCertified);

    std::vector<std::string> empty;
    addParam("listNames", m_listNames, "Particles from these ParticleLists are used as input. If no name is given the teacher is applied to every event once, and one can only use variables which accept nullptr as Particle*", empty);
    addParam("methods", m_methods, "Vector of Tuples with (Name, Type, Config) of the methods. Valid types are: BDT, KNN, Fisher, Plugin. For type 'Plugin', the plugin matching the Name attribute will be loaded (e.g. NeuroBayes). The Config is passed to the TMVA Method and is documented in the TMVA UserGuide.");
    addParam("prefix", m_methodPrefix, "Prefix which is used by the TMVAInterface to store its configfile $prefix.config and by TMVA itself to write the files weights/$prefix_$method.class.C and weights/$prefix_$method.weights.xml with additional information", std::string("TMVA"));
    addParam("workingDirectory", m_workingDirectory, "Working directory in which the config file and the weight file directory is created", std::string("."));
    addParam("variables", m_variables, "Input variables used by the TMVA method");
    addParam("target", m_target, "Target used by the method, has to be an integer-valued variable which defines clusters in the sample.");
    addParam("factoryOption", m_factoryOption, "Option passed to TMVA::Factory", std::string("!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification"));
    addParam("prepareOption", m_prepareOption, "Option passed to TMVA::Factory::PrepareTrainingAndTestTree", std::string("SplitMode=random:!V"));
    addParam("createMVAPDFs", m_createMVAPDFs, "Creates the MVA PDFs for signal and background. This is needed to transform the output of the trained method to a probability.", true);
    addParam("useExistingData", m_useExistingData, "Use existing data which is already stored in the $prefix.root file.", false);
    addParam("doNotTrain", m_doNotTrain, "Do not train, create only datafile with samples. Useful if you want to train outside of basf2 with the externTeacher tool.", false);
    addParam("trainOncePerJob", m_trainOncePerJob, "If true, training is performed once per job (in the terminate method instead of in the endRun method)", true);

    m_teacher = nullptr;
  }

  TMVATeacherModule::~TMVATeacherModule()
  {
  }

  void TMVATeacherModule::initialize()
  {
    for (auto & name : m_listNames) {
      StoreObjPtr<ParticleList>::required(name);
    }

    if (m_trainOncePerJob)
      initTeacher();
  }

  void TMVATeacherModule::initTeacher()
  {
    std::vector<TMVAInterface::Method> methods;
    for (auto & x : m_methods) {
      std::string config = std::get<2>(x);
      if (m_createMVAPDFs)
        config = std::string("CreateMVAPdfs:") + config;
      methods.push_back(TMVAInterface::Method(std::get<0>(x), std::get<1>(x), config, m_variables));
    }
    m_teacher = new TMVAInterface::Teacher(m_methodPrefix, m_workingDirectory, m_target, methods, m_useExistingData);
  }

  void TMVATeacherModule::beginRun()
  {
    if (!m_trainOncePerJob)
      initTeacher();
  }

  void TMVATeacherModule::trainTeacher()
  {
    if (not m_doNotTrain) {
      m_teacher->train(m_factoryOption, m_prepareOption);
    }
    delete m_teacher;
    m_teacher = nullptr;
  }

  void TMVATeacherModule::endRun()
  {
    if (!m_trainOncePerJob)
      trainTeacher();
  }

  void TMVATeacherModule::terminate()
  {
    if (m_trainOncePerJob)
      trainTeacher();
  }

  void TMVATeacherModule::event()
  {
    for (auto & listName : m_listNames) {
      StoreObjPtr<ParticleList> list(listName);
      // Calculate Signal Probability for Particles
      for (unsigned i = 0; i < list->getListSize(); ++i) {
        const Particle* particle = list->getParticle(i);
        m_teacher->addSample(particle);
      }
    }

    if (m_listNames.empty()) {
      m_teacher->addSample(nullptr);
    }
  }

} // Belle2 namespace

