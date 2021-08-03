/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/FlavorTaggerInfoFiller/FlavorTaggerInfoFillerModule.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/FlavorTaggerInfo.h>
#include <analysis/VariableManager/Manager.h>

#include <iostream>

using namespace std;
using namespace Belle2;

// Register module in the framework
REG_MODULE(FlavorTaggerInfoFiller)

FlavorTaggerInfoFillerModule::FlavorTaggerInfoFillerModule() : Module()
{
  //Set module properties
  setDescription("Creates a new flavorTaggerInfoMap DataObject for the specific methods. Saves there all the relevant information of the flavorTagger.");
  setPropertyFlags(c_ParallelProcessingCertified);
  //Parameter definition
  addParam("trackLevelParticleLists", m_trackLevelParticleLists, "Used Flavor Tagger trackLevel Categories of the lists ",
           vector<tuple<string, string>>());
  addParam("eventLevelParticleLists", m_eventLevelParticleLists, "Used Flavor Tagger eventLevel Categories of the lists ",
           vector<tuple<string, string, string>>());
  addParam("FANNmlp", m_FANNmlp, "Sets if FANN Combiner output will be saved or not", false);
  addParam("TMVAfbdt", m_TMVAfbdt, "Sets if FANN Combiner output will be saved or not", false);
  addParam("qpCategories", m_qpCategories, "Sets if individual categories output will be saved or not", false);
  addParam("istrueCategories", m_istrueCategories, "Sets if individual MC truth for each category is saved or not", false);
  addParam("targetProb", m_targetProb, "Sets if individual Categories output will be saved or not", false);
  addParam("trackPointers", m_trackPointers, "Sets if track pointers to target tracks are saved or not", false);

}

void FlavorTaggerInfoFillerModule::initialize()
{
}

void FlavorTaggerInfoFillerModule::event()
{
  if (!m_roe->isBuiltWithMostLikely()) {
    B2ERROR("The ROE was not created with most-likely particle lists."
            "The flavor tagger will not work properly.");
  }
  auto* flavorTaggerInfo = m_roe->getRelatedTo<FlavorTaggerInfo>();

  Variable::Manager& manager = Variable::Manager::Instance();


  if (flavorTaggerInfo == nullptr) {
    B2ERROR("flavorTaggerInfoFiller: FlavorTaggerInfo does not exist");
    return;
  }

  flavorTaggerInfo -> setUseModeFlavorTagger("Expert");

  if (m_FANNmlp) {
    flavorTaggerInfo -> addMethodMap("FANN");
    FlavorTaggerInfoMap* infoMapsFANN = flavorTaggerInfo -> getMethodMap("FANN");
    // float qrCombined = 2 * (eventExtraInfo->getExtraInfo("qrCombinedFANN") - 0.5);
    float qrCombined = m_eventExtraInfo->getExtraInfo("qrCombinedFANN");
    if (qrCombined < 1.1 && qrCombined > 1.0) qrCombined = 1.0;
    if (qrCombined > - 1.1 && qrCombined < -1.0) qrCombined = -1.0;
    float B0Probability = m_eventExtraInfo->getExtraInfo("qrCombinedFANN");
    float B0barProbability = 1 - m_eventExtraInfo->getExtraInfo("qrCombinedFANN");
    infoMapsFANN->setQrCombined(qrCombined);
    infoMapsFANN->setB0Probability(B0Probability);
    infoMapsFANN->setB0barProbability(B0barProbability);
  }

  flavorTaggerInfo -> addMethodMap("FBDT");
  FlavorTaggerInfoMap* infoMapsFBDT = flavorTaggerInfo -> getMethodMap("FBDT");

  if (m_TMVAfbdt) {
    float qrCombined = 2 * (m_eventExtraInfo->getExtraInfo("qrCombinedFBDT") - 0.5);
    if (qrCombined < 1.1 && qrCombined > 1.0) qrCombined = 1.0;
    if (qrCombined > - 1.1 && qrCombined < -1.0) qrCombined = -1.0;
    float B0Probability = m_eventExtraInfo->getExtraInfo("qrCombinedFBDT");
    float B0barProbability = 1 - m_eventExtraInfo->getExtraInfo("qrCombinedFBDT");
    infoMapsFBDT->setQrCombined(qrCombined);
    infoMapsFBDT->setB0Probability(B0Probability);
    infoMapsFBDT->setB0barProbability(B0barProbability);
  }

  if (m_targetProb) {
    for (auto& iTuple : m_trackLevelParticleLists) {
      string particleListName = get<0>(iTuple);
      string category = get<1>(iTuple);
      StoreObjPtr<ParticleList> particleList(particleListName);


      if (!particleList.isValid()) {
        B2INFO("ParticleList " << particleListName << " not found");
      } else {
        if (particleList -> getListSize() == 0) {
          infoMapsFBDT -> setProbTrackLevel(category, 0);
          if (m_trackPointers) infoMapsFBDT -> setTargetTrackLevel(category, nullptr);
        } else {

          for (unsigned int i = 0; i < particleList->getListSize(); ++i) {
            Particle* iParticle =  particleList ->getParticle(i);
            bool hasMaxProb = std::get<bool>(manager.getVariable("hasHighestProbInCat(" + particleListName + "," + "isRightTrack(" + category +
                                                                 "))")->function(iParticle));
            if (hasMaxProb == 1) {
              float targetProb = iParticle -> getExtraInfo("isRightTrack(" + category + ")");
              infoMapsFBDT->setProbTrackLevel(category, targetProb);
              if (m_trackPointers) {
                infoMapsFBDT-> setTargetTrackLevel(category, iParticle -> getTrack());
              }
              break;
            }
          }
        }
      }
    }
  }

  if (m_qpCategories) {

    for (auto& iTuple : m_eventLevelParticleLists) {
      string particleListName = get<0>(iTuple);
      string category = get<1>(iTuple);
      string qpCategoryVariable = get<2>(iTuple);
      StoreObjPtr<ParticleList> particleList(particleListName);

      if (!particleList.isValid()) {
        B2INFO("ParticleList " << particleListName << " not found");
      } else {
        if (particleList -> getListSize() == 0) {
          infoMapsFBDT -> setProbEventLevel(category, 0);
          infoMapsFBDT -> setQpCategory(category, 0);
          if (m_istrueCategories and m_mcparticles.isValid()) {
            infoMapsFBDT -> setHasTrueTarget(category, 0);
            infoMapsFBDT -> setIsTrueCategory(category, 0);
          }
          if (m_trackPointers) infoMapsFBDT -> setTargetEventLevel(category, nullptr);
        } else {

          for (unsigned int i = 0; i < particleList->getListSize(); ++i) {
            Particle* iParticle =  particleList ->getParticle(i);
            bool hasMaxProb = std::get<bool>(manager.getVariable("hasHighestProbInCat(" + particleListName + "," + "isRightCategory(" + category
                                                                 + "))")-> function(iParticle));
            if (hasMaxProb == 1) {
              float categoryProb = iParticle -> getExtraInfo("isRightCategory(" + category + ")");
              float qpCategory = std::get<double>(manager.getVariable(qpCategoryVariable)-> function(iParticle));
              infoMapsFBDT->setProbEventLevel(category, categoryProb);
              infoMapsFBDT -> setQpCategory(category, qpCategory);
              if (m_istrueCategories and m_mcparticles.isValid()) {
                float isTrueTarget = std::get<bool>(manager.getVariable("hasTrueTarget(" + category + ")")-> function(nullptr));
                infoMapsFBDT -> setHasTrueTarget(category, isTrueTarget);
                float isTrueCategory = std::get<bool>(manager.getVariable("isTrueCategory(" + category + ")")-> function(nullptr));
                infoMapsFBDT -> setIsTrueCategory(category, isTrueCategory);
              }
              if (m_trackPointers) {
                infoMapsFBDT-> setTargetEventLevel(category, iParticle -> getTrack());
              }
              break;
            }
          }
        }
      }
    }
  }

}

void FlavorTaggerInfoFillerModule::terminate()
{
}

