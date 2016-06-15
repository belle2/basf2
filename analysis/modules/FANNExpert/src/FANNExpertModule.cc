/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Fernando Abudinen                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/modules/FANNExpert/FANNExpertModule.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <analysis/dataobjects/EventExtraInfo.h>
#include <framework/utilities/WorkingDirectoryManager.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <fstream>

#include <TFile.h>

namespace Belle2 {


  REG_MODULE(FANNExpert)

  FANNExpertModule::FANNExpertModule() : Module()
  {
    setDescription("Adds an ExtraInfo to the Particle objects in given ParticleLists. The ExtraInfo is calculated by a FANN method "
                   "and represents the SignalProbability of the Particle with respect to the training. "
                   "Requires existing training for the specified MLP via FANNTeacher. "
                   "See also https://belle2.cc.kek.jp/~twiki/bin/view/Software/FANN for detailed instructions.");
    setPropertyFlags(c_ParallelProcessingCertified);

    std::vector<std::string> empty;
    addParam("listNames", m_listNames,
             "Particles from these ParticleLists are used as input. If no name is given the expert is applied to every event once, and one can only use variables which accept nullptr as Particle*",
             empty);
    addParam("method", m_methodName, "Method name specified in the training via FANNTeacher.");
    addParam("prefix", m_methodPrefix, "Common prefix for the methods trained by FANNTeacher. "
             "The prefix is used by the FANNInterface to read its config-file $prefix.config "
             "and by FANN method itself to read the files weights/$prefix_$method.class.C "
             "and weights/$prefix_$method.weights.xml with additional information", std::string("FANN"));
    addParam("workingDirectory", m_workingDirectory,
             "Working directory in which the expert finds the config file and the weight file directory", std::string("."));
    addParam("expertOutputName", m_expertOutputName,
             "Name under which the output of the expert is stored in the ExtraInfo of the Particle object.");

  }

  void FANNExpertModule::initialize()
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

    m_filename = m_methodPrefix + std::string("_WeightFile.root");

    if (!load(m_filename, m_methodName)) {
      B2ERROR("FANN Method " << m_methodName << " not found!");
    } else B2INFO("Loaded FANN MLP " << m_methodName << " from File " << m_filename << ".");

      Variable::Manager& manager = Variable::Manager::Instance();
    m_variables =  manager.getVariables(m_MLP.getVariableNames());

  }

  void FANNExpertModule::terminate()
  {
  }

  void FANNExpertModule::event()
  {
    for (auto& listName : m_listNames) {
      StoreObjPtr<ParticleList> list(listName);
      // Calculate target Value for Particles
      for (unsigned i = 0; i < list->getListSize(); ++i) {
        Particle* particle = list->getParticle(i);
        float targetValue = analyse(particle);
        if (particle->hasExtraInfo(m_expertOutputName)) {
          B2WARNING("Extra Info with given name is already set! Overwriting old value!");
          particle->setExtraInfo(m_expertOutputName, targetValue);
        } else {
          particle->addExtraInfo(m_expertOutputName, targetValue);
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
        float targetValue = analyse(nullptr);
        eventExtraInfo->addExtraInfo(m_expertOutputName, targetValue);
      }
    }
  }

  bool FANNExpertModule::load(const std::string& filename, const std::string& arrayname)
  {
    WorkingDirectoryManager dummy(m_workingDirectory);
    TFile datafile(filename.c_str(), "READ");
    if (!datafile.IsOpen()) {
      B2WARNING("Could not open file " << filename);
      return false;
    }
    TObjArray* MLPs = (TObjArray*)datafile.Get(arrayname.c_str());
    if (!MLPs) {
      datafile.Close();
      B2WARNING("File " << filename << " does not contain key " << arrayname);
      return false;
    }

    bool foundMLP = false;
    for (int isector = 0; isector < MLPs->GetEntriesFast(); ++isector) {
      FANNMLP* expert = dynamic_cast<FANNMLP*>(MLPs->At(isector));
      if (expert) {
        FANNMLP mlp = *expert;
        if (mlp.getArrayName() == arrayname) { m_MLP = mlp; foundMLP = true;}
      } else B2WARNING("Wrong type " << MLPs->At(isector)->ClassName() << ", ignoring this entry.");
    }
    MLPs->Clear();
    delete MLPs;
    datafile.Close();
    if (!foundMLP) {
      B2ERROR("Could not find any MLP with the given name " << arrayname << ".");
      return false;
    }

    B2DEBUG(100, "loaded MLP with the name " << m_MLP.getArrayName() << ".");
    return true;
  }

  float FANNExpertModule::analyse(const Particle* particle)
  {

    std::vector<float> input;
    input.resize(m_variables.size(), 0);


    for (unsigned int i = 0; i < m_variables.size(); ++i) {
      if (m_variables[i] != nullptr)
        input[i] = m_variables[i]->function(particle);
      if (!std::isfinite(input[i])) {
        B2WARNING("Output of variable " << m_variables[i]->name << " is " << input[i] << ", please fix it. It will be set manually to -1.");
        input[i] = -1;
      }
    }
    std::vector<float> output = m_MLP.runMLP(input);

    return output[0];

  }

} // Belle2 namespace

