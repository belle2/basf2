/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Phillip Urquijo                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEEDETECTORSTATSRECTOOL_H
#define NTUPLEEDETECTORSTATSRECTOOL_H
#include <boost/function.hpp>
#include <analysis/dataobjects/Particle.h>
#include <framework/dataobjects/EventMetaData.h>
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

using namespace std;
using namespace Belle2;

namespace Belle2 {
  /** Tool for NtupleMaker to write out container summaries to flat ntuple. */
  class NtupleDetectorStatsRecTool : public NtupleFlatTool {
  private:
    /** N ARICHAeroHits */
    int m_iARICHAeroHits;
    /** N ARICHLikelihoods */
    int m_iARICHLikelihoods;
    /** N CDCHits */
    int m_iCDCHits;
    /** N DedxLikelihoods */
    int m_iDedxLikelihoods;
    /** N ECLDspss */
    int m_iECLDsps;
    /** N ECLHitAssignments */
    int m_iECLHitAssignments;
    /** N ECLHits */
    int m_iECLHits;
    /** N ECLShowers */
    int m_iECLShowers;
    /** N ECLTrigs */
    int m_iECLTrigs;
    /** N EKLMHit2ds */
    int m_iEKLMHit2ds;
    /** N EKLMK0Ls */
    int m_iEKLMK0Ls;
    // /** N EKLMStepHits */
    // int m_iEKLMStepHits;
    /** N genfit::Tracks */
    int m_iGFTracks;
    /** N PIDLikelihoods */
    int m_iPIDLikelihoods;
    /** N PXDClusters */
    int m_iPXDClusters;
    /** N SVDCluseters */
    int m_iSVDClusters;
    /** N TOPBarHits */
    int m_iTOPBarHits;
    /** N TOPLikelihoods */
    int m_iTOPLikelihoods;
    /** N TrackFitResults */
    int m_iTrackFitResults;

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constructor. */
    NtupleDetectorStatsRecTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };
} // namepspace Belle2

#endif // NTUPLEDETECTORSTATSRECTOOL_H
