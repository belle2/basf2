/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLETOOLLIST_H
#define NTUPLETOOLLIST_H

#include<string>
#include "TTree.h"
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/NtupleTools/NtupleFlatTool.h>

namespace Belle2 {

  /** List of implemented NtupleTools used to create an NtupleTool object
  from a provided Name. If you implemented a new NtupleTool, the create() method
  has to be extended (see existing entries there). */
  class NtupleToolList {
  public:
    NtupleToolList() {};
    ~NtupleToolList() {};
    /** Create the NtupleTool specified by strName. The TTree* tree is
    the tree to which the NtupleTool will write. The DecayDescriptor d specifies
    which reconstructed particles the NtupleTool should act on. */
    static NtupleFlatTool* create(std::string strName, TTree* tree, DecayDescriptor& d);
  };
} // namespace Belle2

#endif // NTUPLETOOLLIST_H
