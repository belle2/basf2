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
  class NtupleToolList {
  public:
    NtupleToolList() {};
    ~NtupleToolList() {};
    /** Create calls the respective analysis tools specified in strName to build the analysis root tree */
    /** The decay descriptor provides steering within the respective analysis tools*/
    static NtupleFlatTool* create(std::string strName, TTree* tree, DecayDescriptor& d);
  };
} // namespace Belle2

#endif // NTUPLETOOLLIST_H
