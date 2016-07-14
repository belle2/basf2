/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/TMVATeacher/TMVAOnTheFlyTeacherModule.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/VariableManager/Utility.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/MakeROOTCompatible.h>

#include <memory>

namespace Belle2 {


  REG_MODULE(TMVAOnTheFlyTeacher)

  TMVAOnTheFlyTeacherModule::TMVAOnTheFlyTeacherModule() : TMVATeacherModule()
  {
    setDescription("Trains TMVA method with given particle lists as training samples. "
                   "The target variable has to be an integer valued variable which defines the clusters in the sample. "
                   "e.g. isSignal for signal and background cluster or abs_PDG to define different MC-PDGs as clusters. "
                   "The clusters are trained against each other. "
                   "See also https://belle2.cc.kek.jp/~twiki/bin/view/Software/TMVA for detailed instructions.");

    setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

    addParam("methods", m_methods,
             "Vector of Tuples with (Name, Type, Config) of the methods. Valid types are: BDT, KNN, Fisher, Plugin. For type 'Plugin', the plugin matching the Name attribute will be loaded (e.g. NeuroBayes). The Config is passed to the TMVA Method and is documented in the TMVA UserGuide.");

    addParam("target", m_target,
             "Target used by the method, has to be an integer-valued variable which defines clusters in the sample.", std::string(""));
    addParam("weight", m_weight, "Weight used by the method, has to be a variable defined in the variable manager.", std::string(""));

    addParam("factoryOption", m_factoryOption, "Option passed to TMVA::Factory",
             std::string("!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification"));
    addParam("prepareOption", m_prepareOption, "Option passed to TMVA::Factory::PrepareTrainingAndTestTree",
             std::string("SplitMode=random:!V"));

    std::vector<std::string> empty;
    addParam("discriminatingVariables", m_discriminatingVariables,
             "The discriminating variables used by sPlot to determine the weights.", empty);
    addParam("modelFileName", m_modelFileName,
             "Path to the Root file containing the model which describes signal and background in the discriminating variable. This file will only be opened readonly.",
             std::string(""));

  }

  void TMVAOnTheFlyTeacherModule::initialize()
  {

    if (m_target == "") {
      if (m_discriminatingVariables.size() > 0 and m_modelFileName != "") {
        B2INFO("No target given: TMVA Teacher uses sPlot.");
      } else {
        B2FATAL("No target given: However for sPlot mode either the discriminating variables or the modelFileName is missing!");
      }
    } else {
      if (m_discriminatingVariables.size() > 0 or m_modelFileName != "") {
        B2ERROR("Target variable is given: However discriminating variables or a sPlot model is given in addition, but these won't be used!");
      } else {
        B2INFO("TMVA Teacher uses normal classification mode.");
      }
    }

    // If weight is not empty and not in the spectators: add it to the spectators
    if (m_weight != "" and std::find(m_spectators.begin(), m_spectators.end(), m_weight) == m_spectators.end()) {
      m_spectators.push_back(m_weight);
    }

    // If target not in the spectators: add it to the spectators
    if (m_target != "" and std::find(m_spectators.begin(), m_spectators.end(), m_target) == m_spectators.end()) {
      m_spectators.push_back(m_target);
    }

    for (auto& var : m_discriminatingVariables) {
      if (std::find(m_spectators.begin(), m_spectators.end(), var) == m_spectators.end()) {
        m_spectators.push_back(var);
      }
    }

    TMVATeacherModule::initialize();


  }

  void TMVAOnTheFlyTeacherModule::terminate()
  {
    if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {

      m_target = makeROOTCompatible(m_target);
      m_weight = makeROOTCompatible(m_weight);

      if (m_target == "") {
        m_teacher->trainSPlot(m_modelFileName, m_discriminatingVariables, m_weight);
      } else {
        m_teacher->trainClassification(m_factoryOption, m_prepareOption, m_target, m_weight);
      }

    }

    TMVATeacherModule::terminate();
  }


} // Belle2 namespace

