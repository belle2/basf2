/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/modules/TMVAExpert/TMVAExpertModule.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/utility/VariableManager.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <fstream>

namespace Belle2 {


  REG_MODULE(TMVAExpert)

  TMVAExpertModule::TMVAExpertModule() : Module()
  {
    setDescription("Fills ExtraInfo of Particle object with calculated TMVAExpert output. Requires existing training from running TMVATeacher.");
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("listNames", m_listNames, "Input particle list names as list");
    addParam("method", m_methodName, "Method which is used to calculate the target variable. For Plugin methods this name has to be identical to the method name");
    addParam("identifier", m_identifier, "Identifier which is used by the TMVAInterface to read its configfile $identifier.config and by TMVA method itself to read the files weights/$identifier_$method.class.C and weights/$identifier_$method.weights.xml with additional information", std::string("TMVA"));
    addParam("workingDirectory", m_workingDirectory, "Working directory in which the expert finds the config file and the weight file directory", std::string("."));
    addParam("signalProbabilityName", m_signalProbabilityName, "Name under which the signal probability is stored in the ExtraInfo of the Particle object.");
    addParam("signalCluster", m_signalCluster, "Number of the cluster which is considered as signal", 1);

    m_method = nullptr;
  }

  TMVAExpertModule::~TMVAExpertModule()
  {
  }

  void TMVAExpertModule::initialize()
  {
    for (auto & name : m_listNames) {
      StoreObjPtr<ParticleList>::required(name);
    }

    m_method = new TMVAInterface::Expert(m_identifier, m_workingDirectory, m_methodName, m_signalCluster);
    VariableManager::Instance().registerParticleExtraInfoVariable(m_signalProbabilityName, "TMVA Expert SignalProbability Variable");
  }


  void TMVAExpertModule::beginRun()
  {
  }

  void TMVAExpertModule::endRun()
  {
  }

  void TMVAExpertModule::terminate()
  {
    if (m_method !=  nullptr) {
      delete m_method;
      m_method = nullptr;
    }

  }

  void TMVAExpertModule::event()
  {
    for (auto & listName : m_listNames) {
      StoreObjPtr<ParticleList> list(listName);
      // Calculate target Value for Particles [0, N_PARTICLE] and AntiParticles [N_PARTICLE, N_PARTICLE + N_ANTIPARTICLE]
      for (unsigned i = 0; i < list->getNumofParticles() + list->getNumofAntiParticles(); ++i) {
        Particle* particle = list->getParticle(i);
        float targetValue = m_method->analyse(particle);
        particle->addExtraInfo(m_signalProbabilityName, targetValue);
      }
    }
  }

  void TMVAExpertModule::printModuleParams() const
  {
  }

} // Belle2 namespace

