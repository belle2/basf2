/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald,Phillip Urquijo                         *
*               Vishal Bhardwaj                                          *
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

    /** Mother ID. 1 */
    int* m_iMotherID1;
    /** Grand Mother ID. 1 */
    int* m_iGDMotherID1;
    /** Great Grand Mother ID. 1 */
    int* m_iGDGDMotherID1;
    /** MC Info */
    int* m_iMCINFO;

    bool m_InterMediate;

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constuctor. */
    NtupleMCHierarchyTool(TTree* tree, DecayDescriptor& decaydescriptor,
                          const std::string& strOptions)
      : NtupleFlatTool(tree,  decaydescriptor, strOptions)
    {
      m_InterMediate = false;
      setupTree();
    }
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEMCHEIRARCHYTOOL_H
