/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleFlatTool.h>

using namespace Belle2;
NtupleFlatTool::NtupleFlatTool(TTree* tree, DecayDescriptor& decaydescriptor)
{
  m_tree = tree;
  m_decaydescriptor = decaydescriptor;
}
