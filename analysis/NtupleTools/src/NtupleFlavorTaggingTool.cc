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

#include <analysis/dataobjects/RestOfEvent.h>
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

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_B0Probability").c_str(), &B0Probability[iProduct],
                   (strNames[iProduct] + "_B0Probability/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_B0barProbability").c_str(), &B0barProbability[iProduct],
                   (strNames[iProduct] + "_B0barProbability/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_qrCombined").c_str(), &qrCombined[iProduct], (strNames[iProduct] + "_qrCombined/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_qrMC").c_str(), &qrMC[iProduct], (strNames[iProduct] + "_qrMC/F").c_str());
  }
}

void NtupleFlavorTaggingTool::eval(const Particle* particle)
{
  if (!particle) {
//     B2ERROR("NtupleFlavorTaggingTool::eval - no Particle found!");
    printf("NtupleFlavorTaggingTool::eval - no Particle found!");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty()) return;

  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {

    if (selparticles[iProduct]->getExtraInfo("ModeCode") != 1) return;

    B0Probability[iProduct]     = -2;
    B0barProbability[iProduct] = -2;
    qrCombined[iProduct] = -2;
    qrMC[iProduct] = -2;

    const RestOfEvent* roe = selparticles[iProduct]->getRelatedTo<RestOfEvent>();

    if (roe) {
      B0Probability[iProduct] = selparticles[iProduct]->getExtraInfo("B0Probability");
      B0barProbability[iProduct] = selparticles[iProduct]->getExtraInfo("B0barProbability");
      qrCombined[iProduct] = selparticles[iProduct]->getExtraInfo("qrCombined");
      qrMC[iProduct] = selparticles[iProduct]->getExtraInfo("qrMC");
    }
  }
}