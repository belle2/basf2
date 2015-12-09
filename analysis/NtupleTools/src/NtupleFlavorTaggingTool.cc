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
#include <analysis/dataobjects/FlavorTagInfo.h>

#include <cmath>
#include <TBranch.h>
#include <TLorentzVector.h>
#include <TVector3.h>

void NtupleFlavorTaggingTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;
  int nDecayProducts = strNames.size();

  B0Probability = new float[nDecayProducts];
  B0barProbability = new float[nDecayProducts];
  qrCombined = new float[nDecayProducts];
  qrMC = new float[nDecayProducts];
  string method("");

  if (m_strOption.empty()) {
    method = "TMVA";
    B2INFO("Flavor Tagger Output saved for default Multivariate Method: No arguments given.");
  } else {
    if (m_strOption == "FANN") {
      B2INFO("Flavor Tagger Output saved for FANN Multivariate Method");
      method = "FANN";
      m_useFANN = true;
    } else {
      B2FATAL("Invalid option used for Flavor Tagger ntuple tool: " << m_strOption <<
              ". Set to 'TMVA' or 'FANN' to save the Flavor Tagger Output related to these methods or leave the option empty to use the default multivariate Method.");
    }
  }

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_" + method + "_B0Probability").c_str(), &B0Probability[iProduct],
                   (strNames[iProduct] + "_" + method + "_B0Probability/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_" + method + "_B0barProbability").c_str(), &B0barProbability[iProduct],
                   (strNames[iProduct] + "_" + method + "_B0barProbability/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_" + method + "_qrCombined").c_str(), &qrCombined[iProduct],
                   (strNames[iProduct] + "_" + method + "_qrCombined/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_" + method + "_qrMC").c_str(), &qrMC[iProduct],
                   (strNames[iProduct] + "_" + method + "_qrMC/F").c_str());
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

    B0Probability[iProduct]     = -2;
    B0barProbability[iProduct] = -2;
    qrCombined[iProduct] = -2;
    qrMC[iProduct] = -2;

    FlavorTagInfo* flavTag = selparticles[iProduct]->getRelatedTo<FlavorTagInfo>();

    if (flavTag != nullptr) {
      if (flavTag->getUseModeFlavorTagger() != "Expert") continue;
//       method[iProduct] = flavTag->getMethod();
      if (Variable::isRestOfEventEmpty(selparticles[iProduct]) != -2) {
        B0Probability[iProduct] = flavTag->getB0Probability();
        B0barProbability[iProduct] = flavTag->getB0barProbability();
        qrCombined[iProduct] = flavTag->getQrCombined();

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