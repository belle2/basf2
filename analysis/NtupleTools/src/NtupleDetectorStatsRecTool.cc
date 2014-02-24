/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Phillip Urquijo                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleDetectorStatsRecTool.h>
#include <framework/datastore/StoreObjPtr.h>
#include <TBranch.h>

#include <arich/dataobjects/ARICHAeroHit.h>
#include <arich/dataobjects/ARICHLikelihood.h>
#include <cdc/dataobjects/CDCHit.h>
#include <reconstruction/dataobjects/DedxLikelihood.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dataobjects/ECLHitAssignment.h>
#include <ecl/dataobjects/ECLHit.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLTrig.h>
#include <eklm/dataobjects/EKLMHit2d.h>
#include <eklm/dataobjects/EKLMK0L.h>

#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDCluster.h>
#include <top/dataobjects/TOPBarHit.h>
#include <top/dataobjects/TOPLikelihood.h>
#include <genfit/Track.h>

void NtupleDetectorStatsRecTool::setupTree()
{
  m_iARICHAeroHits = -1;
  m_iARICHLikelihoods = -1;
  m_iCDCHits = -1;
  m_iDedxLikelihoods = -1;
  m_iECLDsps = -1;
  m_iECLHitAssignments = -1;
  m_iECLHits = -1;
  m_iECLShowers = -1;
  m_iECLTrigs = -1;
  m_iEKLMHit2ds = -1;
  m_iEKLMK0Ls = -1;
  m_iGFTracks = -1;
  m_iPIDLikelihoods = -1;
  m_iPXDClusters = -1;
  m_iSVDClusters = -1;
  m_iTOPBarHits = -1;
  m_iTOPLikelihoods = -1;
  m_iTrackFitResults = -1;
  m_tree->Branch("nARICHAeroHits",      &m_iARICHAeroHits,  "nARICHAeroHits/I");
  m_tree->Branch("nARICHLikelihoods",   &m_iARICHLikelihoods,   "nARICHLikelihoods/I");
  m_tree->Branch("nCDCHits",        &m_iCDCHits,    "nCDCHits/I");
  m_tree->Branch("nDedxLikelihoods",    &m_iDedxLikelihoods,  "nDedxLikelihoods/I");
  m_tree->Branch("nECLDsps",            &m_iECLDsps,    "nECLDsps/I");
  m_tree->Branch("nECLHitAssignments",  &m_iECLHitAssignments,  "nECLHitAssignments/I");
  m_tree->Branch("nECLHits",    &m_iECLHits,    "nECLHits/I");
  m_tree->Branch("nECLShowers",   &m_iECLShowers,   "nECLShowers/I");
  m_tree->Branch("nECLTrigs",     &m_iECLTrigs,     "nECLTrigs/I");
  m_tree->Branch("nEKLMHit2ds",   &m_iEKLMHit2ds,   "nEKLMHit2ds/I");
  m_tree->Branch("nEKLMK0Ls",     &m_iEKLMK0Ls,     "nEKLMK0Ls/I");
  m_tree->Branch("nGFTracks",     &m_iGFTracks,     "nGFTracks/I");
  m_tree->Branch("nPIDLikelihoods",   &m_iPIDLikelihoods,   "nPIDLikelihoods/I");
  m_tree->Branch("nPXDClusters",  &m_iPXDClusters,  "nPXDClusters/I");
  m_tree->Branch("nSVDClusters",  &m_iSVDClusters,  "nSVDClusters/I");
  m_tree->Branch("nTOPBarHits",   &m_iTOPBarHits,   "nTOPBarHits/I");
  m_tree->Branch("nTOPLikelihoods",   &m_iTOPLikelihoods,   "nTOPLikelihoods/I");
  m_tree->Branch("nTrackFitResults",  &m_iTrackFitResults,  "nTrackFitResults/I");

}

void NtupleDetectorStatsRecTool::eval(const  Particle*)
{
  StoreArray<ARICHAeroHit>    mARICHAeroHits;
  m_iARICHAeroHits = (int) mARICHAeroHits.getEntries();

  StoreArray<ARICHLikelihood>    mARICHLikelihoods;
  m_iARICHLikelihoods = (int) mARICHLikelihoods.getEntries();

  StoreArray<CDCHit>    mCDCHits;
  m_iCDCHits = (int) mCDCHits.getEntries();

  StoreArray<DedxLikelihood>    mDedxLikelihoods;
  m_iDedxLikelihoods = (int) mDedxLikelihoods.getEntries();

  StoreArray<ECLDsp>    mECLDsps;
  m_iECLDsps = (int) mECLDsps.getEntries();

  StoreArray<ECLHitAssignment>    mECLHitAssignments;
  m_iECLHitAssignments = (int) mECLHitAssignments.getEntries();

  StoreArray<ECLHit>    mECLHits;
  m_iECLHits = (int) mECLHits.getEntries();

  StoreArray<ECLShower>    mECLShowers;
  m_iECLShowers = (int) mECLShowers.getEntries();

  StoreArray<ECLTrig>    mECLTrigs;
  m_iECLTrigs = (int) mECLTrigs.getEntries();

  StoreArray<EKLMHit2d>    mEKLMHit2ds;
  m_iEKLMHit2ds = (int) mEKLMHit2ds.getEntries();

  StoreArray<EKLMK0L>    mEKLMK0Ls;
  m_iEKLMK0Ls = (int) mEKLMK0Ls.getEntries();

  StoreArray<genfit::Track>    mGFTracks("");
  m_iGFTracks = (int) mGFTracks.getEntries();

  StoreArray<PIDLikelihood>    mPIDLikelihoods;
  m_iPIDLikelihoods = (int) mPIDLikelihoods.getEntries();

  StoreArray<PXDCluster>    mPXDClusters;
  m_iPXDClusters = (int) mPXDClusters.getEntries();

  StoreArray<SVDCluster>    mSVDClusters;
  m_iSVDClusters = (int) mSVDClusters.getEntries();

  StoreArray<TOPBarHit>    mTOPBarHits;
  m_iTOPBarHits = (int) mTOPBarHits.getEntries();

  StoreArray<TOPLikelihood>    mTOPLikelihoods;
  m_iTOPLikelihoods = (int) mTOPLikelihoods.getEntries();

  StoreArray<TrackFitResult>    mTrackFitResults;
  m_iTrackFitResults = (int) mTrackFitResults.getEntries();

}
