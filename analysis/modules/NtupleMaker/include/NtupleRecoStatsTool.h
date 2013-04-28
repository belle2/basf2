/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Phillip Urquijo                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEERECOSTATSTOOL_H
#define NTUPLEERECOSTATSTOOL_H
#include <boost/function.hpp>
#include <analysis/dataobjects/Particle.h>
#include <framework/dataobjects/EventMetaData.h>
#include <analysis/modules/NtupleMaker/NtupleFlatTool.h>
#include <analysis/modules/NtupleMaker/DecayDescriptor.h>

using namespace std;
using namespace Belle2;

namespace Belle2 {
  /** Tool for NtupleMaker to write out container summaries to flat ntuple. */
  class NtupleRecoStatsTool : public NtupleFlatTool {
  private:
    /** N ECL gammas */
    int m_iNPhotons;
    /** N ECL showers */
    int m_iNShowers;
    /** N ECL pi0s */
    int m_iNPi0s;
    /** N Tracks*/
    int m_iNTracks;
    /** N MCParticles*/
    int m_iNMCParticles;
    /** N Particles*/
    int m_iNParticles;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constructor. */
    NtupleRecoStatsTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };
} // namepspace Belle2

#endif // NTUPLERECOSTATSTOOL_H
