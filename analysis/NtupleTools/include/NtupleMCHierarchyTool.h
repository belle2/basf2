/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald,Phillip Urquijo                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEMCHEIRARCHYTOOL_H
#define NTUPLEMCHEIRARCHYTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /** NtupleTool to write the  mother, grandmother, and greatgrandmother truth ID for a given reconstructed particle
  to a flat ntuple. */
  class NtupleMCHierarchyTool : public NtupleFlatTool {
  private:
    /** Mother ID. */
    int* m_iMotherID;
    /** Grand Mother ID. */
    int* m_iGDMotherID;
    /** Great Grand Mother ID. */
    int* m_iGDGDMotherID;

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constuctor. */
    NtupleMCHierarchyTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEMCHEIRARCHYTOOL_H
