/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Phillip Urquijo, Anze Zupanc           *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLETRACKTOOL_H
#define NTUPLETRACKTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /** NtupleTool to write out track impact parameters (d0 and z0) and track's fit pValue. */

  class NtupleTrackTool : public NtupleFlatTool {

  private:
    /** track's d0 : signed distance to the POCA in the r-phi plane */
    float* m_fD0;
    /** track's d0 error: signed distance to the POCA in the r-phi plane */
    float* m_ferrD0;
    /** track's z0 : z coordinate of the POCA */
    float* m_fZ0;
    /** track's z0 error: z coordinate of the POCA */
    float* m_ferrZ0;
    /** Track fit pValue. */
    float* m_fTrPval;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
    /** Delete 'new's */
    void deallocateMemory();
  public:
    /** Constuctor. */
    NtupleTrackTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Destructor. */
    ~NtupleTrackTool() {deallocateMemory();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLETRACKTOOL_H
