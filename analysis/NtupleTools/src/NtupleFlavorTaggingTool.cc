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
#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Variables.h>
#include <analysis/utility/MCMatching.h>
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/dataobjects/FlavorTaggerInfo.h>

#include <cmath>
#include <TBranch.h>
#include <TLorentzVector.h>
#include <TVector3.h>

void NtupleFlavorTaggingTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;
  int nDecayProducts = strNames.size();

  qrCombinedTMVA = new float[nDecayProducts];
  qrCombinedFANN = new float[nDecayProducts];
  qrMC = new float[nDecayProducts];
  string method("");

  const char* categories[] = { "Muon",  "IntermediateMuon", "Electron", "IntermediateElectron", "KinLepton",
                               "IntermediateKinLepton", "Kaon", "SlowPion", "FastPion", "Lambda", "FSC", "MaximumPstar", "KaonPion"
                             };

  for (auto& category : categories) {
    m_qrCategories.insert(std::pair<std::string, float*>(category, new float[nDecayProducts]));
  }

  if (m_strOption.empty()) {
    m_useTMVA = true;
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

      if (optioni == "TMVA") {
        B2INFO("Flavor Tagger Output saved for TMVA Multivariate Method");
        m_useTMVA = true;
      } else if (optioni == "FANN") {
        B2INFO("Flavor Tagger Output saved for FANN Multivariate Method");
        m_useFANN = true;
      } else if (optioni == "qrCategories") {
        B2INFO("Flavor Tagger Output for each category saved");
        m_saveCategories = true;
      } else {
        B2FATAL("Invalid option used for Flavor Tagger ntuple tool: " << m_strOption <<
                ". Write 'TMVA' and/or 'FANN' to save the Flavor Tagger Output related to these combiner methods or leave the option empty to use the default TMVA Method"
                <<
                ". Write 'qrCategories' to save the qr output of all used categories");
      }
    }
  }

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {


    if (m_useTMVA == true) m_tree->Branch((strNames[iProduct] + "_TMVA_qrCombined").c_str(), &qrCombinedTMVA[iProduct],
                                            (strNames[iProduct] + "_TMVA_qrCombined/F").c_str());

    if (m_useFANN == true) m_tree->Branch((strNames[iProduct] + "_FANN_qrCombined").c_str(), &qrCombinedFANN[iProduct],
                                            (strNames[iProduct] + "_FANN_qrCombined/F").c_str());

    m_tree->Branch((strNames[iProduct] + "_qrMC").c_str(), &qrMC[iProduct],
                   (strNames[iProduct] + "_qrMC/F").c_str());

    if (m_saveCategories == true) {
      for (auto& categoryEntry : m_qrCategories) {
        m_tree->Branch((strNames[iProduct] + "_qr" + categoryEntry.first).c_str(), &categoryEntry.second[iProduct],
                       (strNames[iProduct] + "_qr" + categoryEntry.first + "/F").c_str());
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

    qrCombinedTMVA[iProduct] = -2;
    qrCombinedFANN[iProduct] = -2;
    qrMC[iProduct] = 0;

    FlavorTaggerInfo* flavorTaggerInfo = selparticles[iProduct]->getRelatedTo<FlavorTaggerInfo>();

    if (flavorTaggerInfo != nullptr) {
      if (flavorTaggerInfo->getUseModeFlavorTagger() != "Expert") continue;
//       method[iProduct] = flavTag->getMethod();
      if (Variable::hasRestOfEventTracks(selparticles[iProduct]) > 0) {

        if (m_useTMVA == true) qrCombinedTMVA[iProduct] = flavorTaggerInfo->getMethodMap("TMVA")->getQrCombined();
        if (m_useFANN == true) qrCombinedFANN[iProduct] = flavorTaggerInfo->getMethodMap("FANN")->getQrCombined();

        if (m_saveCategories == true) {
          for (auto& categoryEntry : m_qrCategories) {
            categoryEntry.second[iProduct] = 0;
          }
          std::map<std::string, float> iQrCategories = flavorTaggerInfo -> getMethodMap("TMVA")-> getQrCategory();

          for (auto& categoryEntry : iQrCategories) {
            m_qrCategories.at(categoryEntry.first)[iProduct] = categoryEntry.second;
          }

        }

        //  MC Flavor is saved only if mcparticles is not empty
        StoreArray<MCParticle> mcparticles;
        if (mcparticles.isValid()) {
          if ((mcparticles.getEntries()) > 0) {
            {
              float qrnormed = Variable::isRelatedRestOfEventB0Flavor(selparticles[iProduct]);
              if (qrnormed > -2) qrMC[iProduct] = 2 * (qrnormed - 0.5);
            }
          }
        }

      }
    }
  }
}