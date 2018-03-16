/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Luigi Li Gioi                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEMCVERTEXTOOL_H
#define NTUPLEMCVERTEXTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /** NtupleTool to write Vertex information of an MCParticle matched to the reconstructed Particle
      to a flat ntuple. */
  class NtupleMCVertexTool : public NtupleFlatTool {
  private:
    /** number of decay products */
    int m_nDecayProducts;
    /** Truth Vertex position X. */
    float* m_fTruthX;
    /** Truth Vertex position Y. */
    float* m_fTruthY;
    /** Truth Vertex position Z. */
    float* m_fTruthZ;
    /** Truth Vertex position Sqrt(X^2+Y^2). */
    float* m_fTruthRho;
    /** Truth production vertex position */
    float** m_fTruthProdV;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
    /** delete all float** etc */
    void deallocateMemory();
  public:
    /** Constuctor. */
    NtupleMCVertexTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor), m_fTruthX(0),
      m_fTruthY(0), m_fTruthZ(0) , m_fTruthRho(0) {setupTree();}
    /** Destructor. */
    virtual ~NtupleMCVertexTool() {deallocateMemory();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEMCVERTEXTOOL_H
