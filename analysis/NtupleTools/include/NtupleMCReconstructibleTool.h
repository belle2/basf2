/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Phillip Urquijo, Sam Cunliffe          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /** NtupleTool to write the MC information of reconstructed Particles
  to a flat ntuple. */
  class NtupleMCReconstructibleTool : public NtupleFlatTool {
  private:
    /** Reconstructible based on particle ID */
    int* m_iReconstructible;
    /** Flags for subdetectors */
    /** Particle seen in the PXD **/
    int* m_iSeenInPXD;
    /** Particle seen in the SVD **/
    int* m_iSeenInSVD;
    /** Particle seen in the CDC **/
    int* m_iSeenInCDC;
    /** Particle seen in the TOP **/
    int* m_iSeenInTOP;
    /** Particle seen in the ARICH **/
    int* m_iSeenInARICH;
    /** Particle seen in the ECL **/
    int* m_iSeenInECL;
    /** Particle seen in the KLM **/
    int* m_iSeenInKLM;

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
    /** delete 'new's */
    void deallocateMemory();
  public:
    /** Constuctor. */
    NtupleMCReconstructibleTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Destructor. */
    ~NtupleMCReconstructibleTool() {deallocateMemory();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2
