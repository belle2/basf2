/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Chunhua LI                                               *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleLETRGTool.h>
#include <analysis/VariableManager/Variables.h>
#include <analysis/VariableManager/L1EmulatorVariables.h>
#include <skim/L1Emulator/dataobjects/L1EmulationInformation.h>
#include <TBranch.h>
#include <cmath>
using namespace Belle2;
using namespace std;

void NtupleLETRGTool::setupTree()
{
//  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  m_GDL = new int[50];
  m_tree->Branch("Summary",  &m_GDL[0], "Summary[50]/I");
}

void NtupleLETRGTool::eval(const Particle*)
{
//vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
//  if (selparticles.empty()){cout<<"noParticle"<<endl;selparticles.push_back(NULL);// return;
//}
//const Particle* selparticles = NULL;
// Particle* selparticles = NULL;
  StoreArray<L1EmulationInformation> LEInfos;
  std::vector<int> sum = LEInfos[0]->getSummary();
  for (int i = 0; i < 50; i++)
    m_GDL[i] = sum[i];


}
