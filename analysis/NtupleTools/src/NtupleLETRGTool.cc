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
#include <analysis/VariableManager/PhysicsTriggerVariables.h>
#include <hlt/L1Emulator/dataobjects/L1EmulationInformation.h>
#include <TBranch.h>
#include <cmath>
using namespace Belle2;
using namespace std;

void NtupleLETRGTool::setupTree()
{
//  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  m_totWeight = 0.;
  m_tree->Branch("totWeight", &m_totWeight, "totWeight/F");
  m_GDL = new float[50];
  m_tree->Branch("Summary",  &m_GDL[0], "Summary[50]/F");
}

void NtupleLETRGTool::eval(const Particle*)
{
  StoreArray<L1EmulationInformation> LEInfos;
  m_totWeight = LEInfos[0]->gettotWeight();
  for (int i = 0; i < 50; i++)
    m_GDL[i] = LEInfos[0]->getSummary(i);


}
