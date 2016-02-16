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
#include <analysis/TMVAInterface/Config.h>
#include <analysis/TMVAInterface/SPlot.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

#include <analysis/VariableManager/Utility.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/logging/Logger.h>

#include <memory>

namespace Belle2 {


  REG_MODULE(TMVATeacher)

  TMVATeacherModule::TMVATeacherModule() : Module(), m_sample_var(nullptr), m_maxSamples(0), m_nSamples(0)
  {
    setDescription("Trains TMVA method with given particle lists as training samples. "
                   "The target variable has to be an integer valued variable which defines the clusters in the sample. "
                   "e.g. isSignal for signal and background cluster or abs_PDG to define different MC-PDGs as clusters. "
                   "The clusters are trained against each other. "
                   "See also https://belle2.cc.kek.jp/~twiki/bin/view/Software/TMVA for detailed instructions.");

    setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

    std::vector<std::string> empty;
    addParam("listNames", m_listNames,
             "Particles from these ParticleLists are used as input. If no name is given the teacher is applied to every event once, and one can only use variables which accept nullptr as Particle*",
             empty);
    addParam("prefix", m_prefix,
             "Prefix which is used by the TMVAInterface to store its config file $prefix.config and by TMVA itself to write the files weights/$prefix_$method.class.C and weights/$prefix_$method.weights.xml with additional information",
             std::string("TMVA"));
    addParam("workingDirectory", m_workingDirectory,
             "Working directory in which the config file and the weight file directory is created", std::string("."));
    addParam("variables", m_variables, "Input variables used by the TMVA method");
    addParam("spectators", m_spectators,
             "Input spectators used by the TMVA method. These variables are saved in the output file, but not used as training input.", empty);
    addParam("treeName", m_treeName, "Tree name used in output file, default is prefix + '_tree'", std::string(""));
    addParam("maxSamples", m_maxSamples, "Maximum number of samples. 0 means no limit.", 0lu);
    addParam("sample", m_sample,
             "Variable used for inverse sampling rates. Usually this is the same as the target", std::string(""));
    std::map<int, unsigned int> defaultInverseSamplingRates;
    addParam("inverseSamplingRates", m_inverseSamplingRates, "Map of class id and inverse sampling rate for this class.",
             defaultInverseSamplingRates);
    addParam("lowMemoryProfile", m_lowMemoryProfile,
             "Enables low memory footprint. The internal TTree uses usually O(10MB) of memory, sometimes this is too much, this option tries to reduce the memory footprint of the TMVATeacher module by reducing the basket and cache size of the TTree",
             false);

  }

  void TMVATeacherModule::initialize()
  {
    for (auto& name : m_listNames) {
      StoreObjPtr<ParticleList>::required(name);
    }

    if (m_sample == "" and not m_inverseSamplingRates.empty()) {
      B2FATAL("Received inverseSamplingRates but no sampling variable was given");
    }

    if (m_treeName == "") {
      m_treeName = m_prefix + "_tree";
    }

    Variable::Manager& manager = Variable::Manager::Instance();
    if (m_sample != "") {
      m_sample_var = manager.getVariable(m_sample);
      if (m_sample_var == nullptr) {
        B2ERROR("Couldn't find sample variable " << m_sample << " via the Variable::Manager. Check the name!");
      }

      if (m_sample != "" and std::find(m_spectators.begin(), m_spectators.end(), m_sample) == m_spectators.end()) {
        m_spectators.push_back(m_sample);
      }
    } else {
      m_sample_var = nullptr;
    }
    m_nSamples = 0;

    std::vector<TMVAInterface::Method> methods;
    for (auto& tuple : m_methods)
      methods.push_back(TMVAInterface::Method(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple)));

    TMVAInterface::TeacherConfig config(m_prefix, m_treeName, m_workingDirectory, m_variables, m_spectators, methods);
    m_teacher = std::unique_ptr<TMVAInterface::Teacher>(new TMVAInterface::Teacher(config, false));
    if (m_lowMemoryProfile) {
      m_teacher->enableLowMemoryProfile();
    }

  }

  void TMVATeacherModule::terminate()
  {
    if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
      m_teacher->writeTree();
    }
    m_teacher.reset();
  }

  float TMVATeacherModule::getInverseSamplingRateWeight(const Particle* particle)
  {
    m_nSamples++;

    if (m_maxSamples != 0 and m_nSamples > m_maxSamples) {
      return 0.0;
    }

    if (m_sample_var == nullptr)
      return 1.0;

    long target = std::lround(m_sample_var->function(particle));
    if (m_inverseSamplingRates.find(target) != m_inverseSamplingRates.end()) {
      if (m_iSamples.find(target) == m_iSamples.end()) {
        m_iSamples[target] = 0;
      }
      m_iSamples[target]++;
      if (m_iSamples[target] % m_inverseSamplingRates[target] != 0)
        return 0;
      else {
        m_iSamples[target] = 0;
        return m_inverseSamplingRates[target];
      }
    }
    return 1.0;
  }


  void TMVATeacherModule::event()
  {
    for (auto& listName : m_listNames) {
      StoreObjPtr<ParticleList> list(listName);
      // Calculate Signal Probability for Particles
      for (unsigned i = 0; i < list->getListSize(); ++i) {
        const Particle* particle = list->getParticle(i);
        float weight = getInverseSamplingRateWeight(particle);
        if (weight > 0)
          m_teacher->addSample(particle, weight);
      }
    }

    if (m_listNames.empty()) {
      float weight = getInverseSamplingRateWeight(nullptr);
      if (weight > 0)
        m_teacher->addSample(nullptr, weight);
    }
  }

} // Belle2 namespace

