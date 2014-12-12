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
#include <analysis/modules/ParticleSelector/ParticleSelectorModule.h>
#include <cmath>
#include <TBranch.h>
#include <TLorentzVector.h>
#include <TVector3.h>

void NtupleFlavorTaggingTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;
  float B0Probability = 0;
  float B0barProbability = 0;
  float qrCombined = 0;
  float qrMC = 0;
  float NTracksInROE = 0;
  float NECLClustersInROE = 0;
  float NKLMClustersInROE = 0;

  m_tree->Branch((strNames[0] + "_B0Probability").c_str(), &B0Probability, (strNames[0] + "_B0Probability/F").c_str());
  m_tree->Branch((strNames[0] + "_B0barProbability").c_str(), &B0barProbability, (strNames[0] + "_B0barProbability/F").c_str());
  m_tree->Branch((strNames[0] + "_qrCombined").c_str(), &qrCombined, (strNames[0] + "_qrCombined/F").c_str());
  m_tree->Branch((strNames[0] + "_qrMC").c_str(), &qrMC, (strNames[0] + "_qrMC/F").c_str());
  m_tree->Branch((strNames[0] + "_NTracksInROE").c_str(), &NTracksInROE, (strNames[0] + "_NTracksInROE/F").c_str());
  m_tree->Branch((strNames[0] + "_NECLClustersInROE").c_str(), &NECLClustersInROE, (strNames[0] + "_NECLClustersInROE/F").c_str());
  m_tree->Branch((strNames[0] + "_NKLMClustersInROE").c_str(), &NKLMClustersInROE, (strNames[0] + "_NKLMClustersInROE/F").c_str());

}

void NtupleFlavorTaggingTool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtupleFlavorTaggingTool::eval - ERROR, no Particle found!\n");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty()) return;

  B0Probability = selparticles[0]->getExtraInfo("B0Probability");
  B0barProbability = selparticles[0]->getExtraInfo("B0barProbability");
  qrCombined = selparticles[0]->getExtraInfo("qrCombined");
  qrMC = selparticles[0]->getExtraInfo("qrMC");
  NTracksInROE = selparticles[0]->getExtraInfo("NTracksInROE");
  NECLClustersInROE = selparticles[0]->getExtraInfo("NECLClustersInROE");
  NKLMClustersInROE = selparticles[0]->getExtraInfo("NKLMClustersInROE");

}