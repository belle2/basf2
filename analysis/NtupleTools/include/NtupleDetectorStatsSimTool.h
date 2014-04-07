/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Phillip Urquijo                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEEDETECTORSTATSSIMTOOL_H
#define NTUPLEEDETECTORSTATSSIMTOOL_H
#include <boost/function.hpp>
#include <analysis/dataobjects/Particle.h>
#include <framework/dataobjects/EventMetaData.h>
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

namespace Belle2 {
  /** Tool for NtupleMaker to write out container summaries to flat ntuple. */
  class NtupleDetectorStatsSimTool : public NtupleFlatTool {
  private:
    /** N ARICHDigits */
    int m_iARICHDigits;
    /** N ARICHSimHits */
    int m_iARICHSimHits;
    /** N CDCSimHits */
    int m_iCDCSimHits;
    /** N ECLDigits */
    int m_iECLDigits;
    /** N ECLSimHits */
    int m_iECLSimHits;
    /** N EKLMDigits */
    int m_iEKLMDigits;
    /** N EKLMSimHits */
    int m_iEKLMSimHits;
    /** N PXDDigits */
    int m_iPXDDigits;
    /** N PXDSimHits */
    int m_iPXDSimHits;
    /** N PXDTrueHits */
    int m_iPXDTrueHits;
    /** N SVDDigits */
    int m_iSVDDigits;
    /** N SVDSimHits */
    int m_iSVDSimHits;
    /** N SVDTrueHits */
    int m_iSVDTrueHits;
    /** N TOPDigits */
    int m_iTOPDigits;
    /** N TOPSimHits */
    int m_iTOPSimHits;
    /** N TOPSimPhotons */
    int m_iTOPSimPhotons;

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constructor. */
    NtupleDetectorStatsSimTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };
} // namepspace Belle2

#endif // NTUPLEDETECTORSTATSSIMTOOL_H
