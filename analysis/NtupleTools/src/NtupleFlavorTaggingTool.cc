/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                      *
* Copyright(C) 2014 - Belle II Collaboration                              *
*                                                                         *
* Author: The Belle II Collaboration                                      *
* Contributors: Fernando Abudinen and Moritz Gelb                         *
*                                                                         *
* This software is provided "as is" without any warranty.                 *
**************************************************************************/

#include <analysis/NtupleTools/NtupleFlavorTaggingTool.h>
#include <analysis/VariableManager/FlavorTaggingVariables.h>
#include <analysis/VariableManager/Variables.h>
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/dataobjects/FlavorTaggerInfo.h>

#include <cmath>
#include <TBranch.h>
#include <TLorentzVector.h>
#include <TVector3.h>

using namespace Belle2;
using namespace std;

void NtupleFlavorTaggingTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;
  int nDecayProducts = strNames.size();

  qrCombinedFBDT = new float[nDecayProducts];
  qrCombinedFANN = new float[nDecayProducts];
  qrMC = new float[nDecayProducts];
  //string method("");

  const char* categories[] = { "Muon",  "IntermediateMuon", "Electron", "IntermediateElectron", "KinLepton",
                               "IntermediateKinLepton", "Kaon", "SlowPion", "FastHadron", "Lambda", "FSC", "MaximumPstar", "KaonPion"
                             };

  for (auto& category : categories) {
    m_qpCategories.insert(std::pair<std::string, float*>(category, new float[nDecayProducts]));
    m_hasTrueTargets.insert(std::pair<std::string, float*>(category, new float[nDecayProducts]));
    m_isTrueCategories.insert(std::pair<std::string, float*>(category, new float[nDecayProducts]));
  }

  if (m_strOption.empty()) {
    m_useFBDT = true;
    m_useFANN = true;
    B2INFO("Flavor Tagger Output saved for default Multivariate Method: No arguments given.");
  } else {


    std::stringstream options(m_strOption);
    std::string option;
    std::vector<std::string> optionsVector;
    while (std::getline(options, option, ',')) {
      option.erase(std::remove(option.begin(), option.end(), ' '), option.end());
      optionsVector.push_back(option);
    }

    for (auto& optioni : optionsVector) {

      if (optioni == "TMVA-FBDT") {
        B2INFO("Flavor Tagger Output saved for FBDT Multivariate Method");
        m_useFBDT = true;
      } else if (optioni == "FANN-MLP") {
        B2INFO("Flavor Tagger Output saved for FANN Multivariate Method");
        m_useFANN = true;
      } else if (optioni == "qpCategories") {
        B2INFO("Flavor Tagger Output for each category saved");
        m_saveCategories = true;
      } else {
        B2FATAL("Invalid option used for Flavor Tagger ntuple tool: " << m_strOption <<
                ". Write 'FBDT' and/or 'FANN' to save the Flavor Tagger Output related to these combiner methods or leave the option empty to use the default FBDT Method"
                <<
                ". Write 'qpCategories' to save the qp output of all used categories");
      }
    }
  }

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {


    if (m_useFBDT == true) m_tree->Branch((strNames[iProduct] + "_FBDT_qrCombined").c_str(), &qrCombinedFBDT[iProduct],
                                            (strNames[iProduct] + "_FBDT_qrCombined/F").c_str());

    if (m_useFANN == true) m_tree->Branch((strNames[iProduct] + "_FANN_qrCombined").c_str(), &qrCombinedFANN[iProduct],
                                            (strNames[iProduct] + "_FANN_qrCombined/F").c_str());

    m_tree->Branch((strNames[iProduct] + "_qrMC").c_str(), &qrMC[iProduct],
                   (strNames[iProduct] + "_qrMC/F").c_str());

    if (m_saveCategories == true) {
      for (auto& categoryEntry : m_qpCategories) {
        m_tree->Branch((strNames[iProduct] + "_qp" + categoryEntry.first).c_str(), &categoryEntry.second[iProduct],
                       (strNames[iProduct] + "_qp" + categoryEntry.first + "/F").c_str());
      }
      for (auto& categoryEntry : m_hasTrueTargets) {
        m_tree->Branch((strNames[iProduct] + "_hasTrueTarget" + categoryEntry.first).c_str(), &categoryEntry.second[iProduct],
                       (strNames[iProduct] + "_hasTrueTarget" + categoryEntry.first + "/F").c_str());
      }
      for (auto& categoryEntry : m_isTrueCategories) {
        m_tree->Branch((strNames[iProduct] + "_isRightCategory" + categoryEntry.first).c_str(), &categoryEntry.second[iProduct],
                       (strNames[iProduct] + "_isRightCategory" + categoryEntry.first + "/F").c_str());
      }
    }

  }
}

void NtupleFlavorTaggingTool::eval(const Particle* particle)
{
  if (!particle) {
    B2INFO("NtupleFlavorTaggingTool::eval - no Particle found!");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty()) return;

  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {

    qrCombinedFBDT[iProduct] = -2;
    qrCombinedFANN[iProduct] = -2;
    qrMC[iProduct] = 0;

    if (m_saveCategories == true and (m_useFBDT == true or m_useFANN == true)) {
      for (auto& categoryEntry : m_qpCategories) {
        categoryEntry.second[iProduct] = -2;
      }

      for (auto& categoryEntry : m_hasTrueTargets) {
        categoryEntry.second[iProduct] = -2;
      }

      for (auto& categoryEntry : m_isTrueCategories) {
        categoryEntry.second[iProduct] = -2;
      }
    }

    FlavorTaggerInfo* flavorTaggerInfo = selparticles[iProduct]->getRelatedTo<FlavorTaggerInfo>();

    if (flavorTaggerInfo != nullptr) {
      if (flavorTaggerInfo->getUseModeFlavorTagger() != "Expert") continue;
//       method[iProduct] = flavTag->getMethod();
      if (Variable::hasRestOfEventTracks(selparticles[iProduct]) > 0) {

        if (m_useFBDT == true) qrCombinedFBDT[iProduct] = flavorTaggerInfo->getMethodMap("FBDT")->getQrCombined();
        if (m_useFANN == true) qrCombinedFANN[iProduct] = flavorTaggerInfo->getMethodMap("FANN")->getQrCombined();

        if (m_saveCategories == true and (m_useFBDT == true or m_useFANN == true)) {

          std::map<std::string, float> iHasTrueTargets = flavorTaggerInfo -> getMethodMap("FBDT")-> getHasTrueTarget();
          std::map<std::string, float> iIsTrueCategories = flavorTaggerInfo -> getMethodMap("FBDT")-> getIsTrueCategory();
          std::map<std::string, float> iQpCategories = flavorTaggerInfo -> getMethodMap("FBDT")-> getQpCategory();
          for (auto& categoryEntry : iQpCategories) {
            m_qpCategories.at(categoryEntry.first)[iProduct] = categoryEntry.second;
          }

          for (auto& categoryEntry : iHasTrueTargets) {
            m_hasTrueTargets.at(categoryEntry.first)[iProduct] = categoryEntry.second;
          }

          for (auto& categoryEntry : iIsTrueCategories) {
            m_isTrueCategories.at(categoryEntry.first)[iProduct] = categoryEntry.second;
          }
        }

        //  MC Flavor is saved only if mcparticles is not empty
        StoreArray<MCParticle> mcparticles;
        if (mcparticles.isValid()) {
          if ((mcparticles.getEntries()) > 0) {
            {
              qrMC[iProduct] = Variable::isRelatedRestOfEventB0Flavor(selparticles[iProduct]);
            }
          }
        }
      }
    }
  }
}
