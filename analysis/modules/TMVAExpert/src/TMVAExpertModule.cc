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

    // Add parameters
    std::vector<std::string> defaultList;
    addParam("listNames", m_listNames, "Input particle list names as list", defaultList);
    addParam("method", m_methodName, "Method which is used to calculate the target variable. The name has to start with a valid method. Valid methods are: BDT, KNN, NeuroBayes, Fisher. Valid names are therefore BDT, BDTWithGradientBoost, BDT_MySecondBDTWhichDoesntOverwriteMyFirstOne,...");
    addParam("identifier", m_identifier, "Identifier which is used by the TMVA method to read the files weights/$identifier_$method.class.C and weights/$identifier_$method.weights.xml with additional information");
    addParam("weightfile", m_weightfile, "Path to weightfile. If you don't specify method and identifier, you have to specify the weightfile instead.", std::string(""));
    addParam("target", m_targetName, "Name of the target variable, which is stored in the ExtraInfo of the Particle object");

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

    if (m_weightfile.empty())
      m_weightfile = std::string("weights/") + m_identifier + std::string("_") + m_methodName + std::string(".weights.xml");
    m_method = new TMVAInterface::Expert(m_weightfile);
    VariableManager::Instance().registerParticleExtraInfoVariable(m_targetName, "TMVA Expert TargetVariable");
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
        particle->addExtraInfo(m_targetName, targetValue);
      }
    }
  }

  void TMVAExpertModule::printModuleParams() const
  {
  }

} // Belle2 namespace

