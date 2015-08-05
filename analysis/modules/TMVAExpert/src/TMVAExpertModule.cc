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

#include <analysis/TMVAInterface/Config.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <analysis/dataobjects/EventExtraInfo.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <fstream>

namespace Belle2 {


  REG_MODULE(TMVAExpert)

  TMVAExpertModule::TMVAExpertModule() : Module(), m_splotPrior_func(nullptr)
  {
    setDescription("Adds an ExtraInfo to the Particle objects in given ParticleLists. The ExtraInfo is calculated by a TMVA method "
                   "and represents the SignalProbability of the Particle with respect to the training. "
                   "Requires existing training for the specified method via TMVATeacher. "
                   "See also https://belle2.cc.kek.jp/~twiki/bin/view/Software/TMVA for detailed instructions.");
    setPropertyFlags(c_ParallelProcessingCertified);

    std::vector<std::string> empty;
    addParam("listNames", m_listNames,
             "Particles from these ParticleLists are used as input. If no name is given the expert is applied to every event once, and one can only use variables which accept nullptr as Particle*",
             empty);
    addParam("method", m_methodName, "Method name specified in the training via TMVATeacher.");
    addParam("prefix", m_methodPrefix, "Common prefix for the methods trained by TMVATeacher. "
             "The prefix is used by the TMVAInterface to read its config-file $prefix.config "
             "and by TMVA method itself to read the files weights/$prefix_$method.class.C "
             "and weights/$prefix_$method.weights.xml with additional information", std::string("TMVA"));
    addParam("workingDirectory", m_workingDirectory,
             "Working directory in which the expert finds the config file and the weight file directory", std::string("."));
    addParam("expertOutputName", m_expertOutputName,
             "Name under which the output of the expert is stored in the ExtraInfo of the Particle object.");
    addParam("signalClass", m_signalClass,
             "Class which is considered as signal. e.g. the pdg of the Particle which is considered signal if "
             "you trained the method with pdg as target variable. Or 1 if you trained with isSignal as target.", 1);
    addParam("signalFraction", m_signalFraction,
             "signalFraction to calculate probability (if -1 the signalFraction of the training data is used)", -1.0f);
    addParam("transformToProbability", m_transformToProbability,
             "Transform classifier output to a probability using given signalFraction."
             "WARNING: If you want to use this feature you have to set the options createMVAPDFs and NbinsMVAPdf in the method config string provided to TMVATeacher to reasonable values!"
             "The default values of TMVA will result in a unusable classifier output!", true);
    addParam("sPlotPrior", m_splotPrior,
             "If set, in addition to the network output the prior calculated from the variable used during the sPlot training is also saved under the name $expertOutputName_$sPlotPrior",
             std::string(""));

  }

  void TMVAExpertModule::initialize()
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

    TMVAInterface::ExpertConfig config(m_methodPrefix, m_workingDirectory, m_methodName, m_signalClass, m_signalFraction);
    m_method = std::unique_ptr<TMVAInterface::Expert>(new TMVAInterface::Expert(config, m_transformToProbability));

    if (m_splotPrior != "") {
      Variable::Manager& manager = Variable::Manager::Instance();
      m_splotPrior_func = manager.getVariable(m_splotPrior);
      if (not config.hasExtraData("SPlotPriorBinning"))
        B2FATAL("Missing sPlotPrior binning in given config file, are you sure this was an sPlot training?");
      m_splotPrior_binning = config.getExtraData("SPlotPriorBinning");
      if (not config.hasExtraData("SPlotPriorValues"))
        B2FATAL("Missing sPlotPrior values in given config file, are you sure this was an sPlot training?");
      m_splotPrior_values = config.getExtraData("SPlotPriorValues");
    }

  }

  void TMVAExpertModule::terminate()
  {
    m_method.reset();
  }

  float TMVAExpertModule::sPlotPrior(const Particle* p) const
  {
    float v = m_splotPrior_func->function(p);
    if (v < m_splotPrior_binning[0] or v > m_splotPrior_binning[m_splotPrior_binning.size() - 1]) {
      return 0.0;
    }
    for (unsigned int i = 1; i < m_splotPrior_binning.size(); ++i) {
      if (v >= m_splotPrior_binning[i - 1] and v <= m_splotPrior_binning[i]) {
        return m_splotPrior_values[i - 1];
      }
    }
    return 0.0;
  }

  void TMVAExpertModule::event()
  {
    for (auto& listName : m_listNames) {
      StoreObjPtr<ParticleList> list(listName);
      // Calculate target Value for Particles
      for (unsigned i = 0; i < list->getListSize(); ++i) {
        Particle* particle = list->getParticle(i);
        float targetValue = m_method->analyse(particle);
        if (particle->hasExtraInfo(m_expertOutputName)) {
          B2WARNING("Extra Info with given name is already set! Overwriting old value!");
          particle->setExtraInfo(m_expertOutputName, targetValue);
        } else {
          particle->addExtraInfo(m_expertOutputName, targetValue);
        }
        if (m_splotPrior_func != nullptr) {
          std::string name = m_expertOutputName + std::string("_") + m_splotPrior;
          float prior = sPlotPrior(particle);
          if (particle->hasExtraInfo(name)) {
            B2WARNING("Extra Info with given name is already set! Overwriting old value!");
            particle->setExtraInfo(name, prior);
          } else {
            particle->addExtraInfo(name, prior);
          }
        }
      }
    }
    if (m_listNames.empty()) {
      StoreObjPtr<EventExtraInfo> eventExtraInfo;
      if (not eventExtraInfo.isValid())
        eventExtraInfo.create();
      if (eventExtraInfo->hasExtraInfo(m_expertOutputName)) {
        B2WARNING("Extra Info with given name is already set! I won't set it again!");
      } else {
        float targetValue = m_method->analyse(nullptr);
        eventExtraInfo->addExtraInfo(m_expertOutputName, targetValue);
      }
      if (m_splotPrior_func != nullptr) {
        std::string name = m_expertOutputName + std::string("_") + m_splotPrior;
        if (eventExtraInfo->hasExtraInfo(name)) {
          B2WARNING("Extra Info with given name is already set! I won't set it again!");
        } else {
          float prior = sPlotPrior(nullptr);
          eventExtraInfo->addExtraInfo(name, prior);
        }
      }
    }
  }

} // Belle2 namespace

