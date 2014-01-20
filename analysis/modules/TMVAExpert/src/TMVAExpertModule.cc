/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/ModuleManager.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <analysis/dataobjects/ParticleInfo.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <framework/logging/Logger.h>

#include <analysis/TMVAInterface/TMVAExpert.h>
#include <analysis/utility/VariableManager.h>
#include <analysis/modules/TMVAExpert/TMVAExpertModule.h>

#include <fstream>

namespace Belle2 {


  REG_MODULE(TMVAExpert)

  TMVAExpertModule::TMVAExpertModule() : Module()
  {
    setDescription("Fills ParticleInfo with calculated TMVAExpert output.");
    // We use root TFile and TMVA so, this is propably not usable for parallel processsing
    //setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    std::vector<std::string> defaultList;
    addParam("listNames", m_listNames, "Input particle list names as list", defaultList);
    addParam("method", m_methodName, "Method which is used to calculate the target variable. The name has to start with a valid method. Valid methods are: BDT, KNN, NeuroBayes, Fisher. Valid names are therefore BDT, BDTWithGradientBoost, BDT_MySecondBDTWhichDoesntOverwriteMyFirstOne,...");
    addParam("identifier", m_identifier, "Identifier which is used by the TMVA method to read the files weights/$identifier_$method.class.C and weights/$identifier_$method.weights.xml with additional information");
    addParam("target", m_targetName, "Name of the target variable, which is stored in the related ParticleInfo of the particle");

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
    StoreArray<ParticleInfo>::required();

    m_method = new TMVAExpert(m_identifier, m_methodName);
    VariableManager::Instance().registerParticleInfoVariable(m_targetName, "TMVA Expert TargetVariable", true);
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
        const Particle* particle = list->getParticle(i);
        float targetValue = m_method->analyse(particle);
        ParticleInfo* particleInfo = DataStore::getRelated<ParticleInfo>(particle);
        particleInfo->addVariable(m_targetName, targetValue);
      }
    }
  }

  void TMVAExpertModule::printModuleParams() const
  {
  }

} // Belle2 namespace

