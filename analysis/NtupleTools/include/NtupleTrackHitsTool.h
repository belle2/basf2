/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLETRACKHITSTOOL_H
#define NTUPLETRACKHITSTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /** NtupleTool to write out track impact parameters (d0 and z0) and track's fit pValue. */

  class NtupleTrackHitsTool : public NtupleFlatTool {

  private:
    /** number of CDC hits associated to the track */
    int* m_iNCDCHits;
    /** number of SVD hits associated to the track */
    int* m_iNSVDHits;
    /** number of PXD hits associated to the track */
    int* m_iNPXDHits;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
    /** Delete 'new's */
    void deallocateMemory();
  public:
    /** Constuctor. */
    NtupleTrackHitsTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Destructor. */
    ~NtupleTrackHitsTool() {deallocateMemory();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLETRACKHITSTOOL_H
