/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Chunhua LI                                               *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleHLTTagTool.h>
#include <mdst/dataobjects/HLTTag.h>
#include <TBranch.h>
#include <cmath>
using namespace Belle2;
using namespace std;

void NtupleHLTTagTool::setupTree()
{
//  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  m_hltsubtrgsummary = new int[16];
  m_tree->Branch("HLTsubtrgSummary", &m_hltsubtrgsummary[0], "m_hltsubtrgsummary[16]/I");
  m_hltsummary = 0;
  m_tree->Branch("HLTSummary",  &m_hltsummary, "HLTSummary/I");
}

void NtupleHLTTagTool::eval(const Particle*)
{
  StoreArray<HLTTag> hlttag;
  m_hltsummary = hlttag[0]->GetSummaryWord();
  m_hltsubtrgsummary[2] = hlttag[0]->GetAlgoInfo(HLTTag::Hadronic);
  m_hltsubtrgsummary[3] = hlttag[0]->GetAlgoInfo(HLTTag::Tautau);
  m_hltsubtrgsummary[4] = hlttag[0]->GetAlgoInfo(HLTTag::LowMulti);
  m_hltsubtrgsummary[5] = hlttag[0]->GetAlgoInfo(HLTTag::Bhabha);
  m_hltsubtrgsummary[6] = hlttag[0]->GetAlgoInfo(HLTTag::Mumu);
  m_hltsubtrgsummary[7] = hlttag[0]->GetAlgoInfo(HLTTag::Calib1);
  m_hltsubtrgsummary[0] = hlttag[0]->GetAlgoInfo(HLTTag::Global);
  m_hltsubtrgsummary[1] = hlttag[0]->GetAlgoInfo(HLTTag::Level3);


}
