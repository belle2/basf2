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
    setDescription("Trains multivariate analysis method with given particle lists as training samples. See https://belle2.cc.kek.jp/~twiki/bin/view/Software/TMVA for detailed instructions.");
    //setPropertyFlags(c_ParallelProcessingCertified);

    addParam("listNames", m_listNames, "Input particle list names as list");
    addParam("methods", m_methods, "Vector of Tuples with (Name, Type, Config) of the methods. Valid types are: BDT, KNN, NeuroBayes, Fisher. The Config is passed to the TMVA Method and is documented in the TMVA UserGuide.");
    addParam("prefix", m_identifier, "Prefix which is used by the TMVAInterface to store its configfile $prefix.config and by TMVA itself to write the files weights/$prefix_$method.class.C and weights/$prefix_$method.weights.xml with additional information", std::string("TMVA"));
    addParam("workingDirectory", m_workingDirectory, "Working directory in which the config file and the weight file directory is created", std::string("."));
    addParam("variables", m_variables, "Input variables used by the TMVA method");
    addParam("target", m_target, "Target used by the method");
    addParam("factoryOption", m_factoryOption, "Option passed to TMVA::Factory", std::string("!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification"));
    addParam("prepareOption", m_prepareOption, "Option passed to TMVA::Factory::PrepareTrainingAndTestTree", std::string("SplitMode=random:!V"));

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
  }


  void TMVATeacherModule::beginRun()
  {
    std::vector<TMVAInterface::Method> methods;
    for (auto & x : m_methods) {
      methods.push_back(TMVAInterface::Method(std::get<0>(x), std::get<1>(x), std::get<2>(x), m_variables));
    }
    m_teacher = new TMVAInterface::Teacher(m_identifier, m_workingDirectory, m_target, methods);
  }

  void TMVATeacherModule::endRun()
  {
    m_teacher->train(m_factoryOption, m_prepareOption);
    delete m_teacher;
    m_teacher = nullptr;
  }

  void TMVATeacherModule::terminate()
  {

  }

  void TMVATeacherModule::event()
  {
    for (auto & listName : m_listNames) {
      StoreObjPtr<ParticleList> list(listName);
      // Calculate Signal Probability for Particles [0, N_PARTICLE] and AntiParticles [N_PARTICLE, N_PARTICLE + N_ANTIPARTICLE]
      for (unsigned i = 0; i < list->getNumofParticles() + list->getNumofAntiParticles(); ++i) {
        const Particle* particle = list->getParticle(i);
        m_teacher->addSample(particle);
      }
    }
  }

} // Belle2 namespace

