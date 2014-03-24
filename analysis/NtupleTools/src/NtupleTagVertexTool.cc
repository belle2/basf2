/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Luigi Li Gioi                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleTagVertexTool.h>
#include <analysis/utility/PSelectorFunctions.h>
#include <cmath>
#include <TBranch.h>
#include <TLorentzVector.h>
#include <TVector3.h>

void NtupleTagVertexTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;
  m_fTagVx = 0;
  m_fTagVy = 0;
  m_fTagVz = 0;
  m_fTagVex = 0;
  m_fTagVey = 0;
  m_fTagVez = 0;

  m_tree->Branch((strNames[0] + "_TagVx").c_str(), &m_fTagVx, (strNames[0] + "_TagVx/F").c_str());
  m_tree->Branch((strNames[0] + "_TagVy").c_str(), &m_fTagVy, (strNames[0] + "_TagVy/F").c_str());
  m_tree->Branch((strNames[0] + "_TagVz").c_str(), &m_fTagVz, (strNames[0] + "_TagVz/F").c_str());
  m_tree->Branch((strNames[0] + "_TagVex").c_str(), &m_fTagVex, (strNames[0] + "_TagVex/F").c_str());
  m_tree->Branch((strNames[0] + "_TagVey").c_str(), &m_fTagVey, (strNames[0] + "_TagVey/F").c_str());
  m_tree->Branch((strNames[0] + "_TagVez").c_str(), &m_fTagVez, (strNames[0] + "_TagVez/F").c_str());
}

void NtupleTagVertexTool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtupleTagVertexTool::eval - ERROR, no Particle found!\n");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty()) return;

  m_fTagVx = selparticles[0]->getExtraInfo("TagVx");
  m_fTagVy = selparticles[0]->getExtraInfo("TagVy");
  m_fTagVz = selparticles[0]->getExtraInfo("TagVz");
  m_fTagVex = TMath::Sqrt(selparticles[0]->getExtraInfo("TagVcovXX"));
  m_fTagVey = TMath::Sqrt(selparticles[0]->getExtraInfo("TagVcovYY"));
  m_fTagVez = TMath::Sqrt(selparticles[0]->getExtraInfo("TagVcovZZ"));

}




