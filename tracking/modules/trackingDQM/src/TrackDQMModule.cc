/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for Tracking DQM                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <genfit/MeasurementOnPlane.h>
#include <tracking/modules/trackingDQM/TrackDQMModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>

#include <framework/database/DBObjPtr.h>

#include <TDirectory.h>
#include <TVectorD.h>

using namespace Belle2;
using namespace std;
using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TrackDQM)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrackDQMModule::TrackDQMModule() : HistoModule()
{
  //Set module properties
  setDescription("DQM of finding tracks, their momentum, "
                 "Number of hits in tracks, "
                 "Number of tracks. "
                );
  setPropertyFlags(c_ParallelProcessingCertified);

}


TrackDQMModule::~TrackDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void TrackDQMModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

}

void TrackDQMModule::defineHisto()
{
  // Create a separate histogram directories and cd into it.
  TDirectory* oldDir = gDirectory;
  TDirectory* DirTracks = NULL;
  DirTracks = oldDir->mkdir("TracksDQM");
  DirTracks->cd();

  m_MomX = NULL;
  m_MomY = NULL;
  m_MomZ = NULL;
  m_Mom = NULL;
  m_HitsPXD = NULL;
  m_HitsSVD = NULL;
  m_HitsCDC = NULL;
  m_Hits = NULL;
  m_TracksVXD = NULL;
  m_TracksCDC = NULL;
  m_TracksVXDCDC = NULL;
  m_Tracks = NULL;

  int iHitsInPXD = 10;
  int iHitsInSVD = 20;
  int iHitsInCDC = 200;
  int iHits = 200;
  int iTracks = 30;
  int iMomRange = 600;
  float fMomRange = 3.0;
  string name = str(format("TrackMomentumX"));
  string title = str(format("Track Momentum X"));
  m_MomX = new TH1F(name.c_str(), title.c_str(), 2 * iMomRange, -fMomRange, fMomRange);
  m_MomX->GetXaxis()->SetTitle("Momentum");
  m_MomX->GetYaxis()->SetTitle("counts");
  name = str(format("TrackMomentumY"));
  title = str(format("Track Momentum Y"));
  m_MomY = new TH1F(name.c_str(), title.c_str(), 2 * iMomRange, -fMomRange, fMomRange);
  m_MomY->GetXaxis()->SetTitle("Momentum");
  m_MomY->GetYaxis()->SetTitle("counts");
  name = str(format("TrackMomentumZ"));
  title = str(format("Track Momentum Z"));
  m_MomZ = new TH1F(name.c_str(), title.c_str(), 2 * iMomRange, -fMomRange, fMomRange);
  m_MomZ->GetXaxis()->SetTitle("Momentum");
  m_MomZ->GetYaxis()->SetTitle("counts");
  name = str(format("TrackMomentumPt"));
  title = str(format("Track Momentum pT"));
  m_MomPt = new TH1F(name.c_str(), title.c_str(), 2 * iMomRange, 0.0, fMomRange);
  m_MomPt->GetXaxis()->SetTitle("Momentum");
  m_MomPt->GetYaxis()->SetTitle("counts");
  name = str(format("TrackMomentumMag"));
  title = str(format("Track Momentum Magnitude"));
  m_Mom = new TH1F(name.c_str(), title.c_str(), 2 * iMomRange, 0.0, fMomRange);
  m_Mom->GetXaxis()->SetTitle("Momentum");
  m_Mom->GetYaxis()->SetTitle("counts");

  name = str(format("NoOfHitsInTrack_PXD"));
  title = str(format("No Of Hits In Track - PXD"));
  m_HitsPXD = new TH1F(name.c_str(), title.c_str(), iHitsInPXD, 0, iHitsInPXD);
  m_HitsPXD->GetXaxis()->SetTitle("# hits");
  m_HitsPXD->GetYaxis()->SetTitle("counts");
  name = str(format("NoOfHitsInTrack_SVD"));
  title = str(format("No Of Hits In Track - SVD"));
  m_HitsSVD = new TH1F(name.c_str(), title.c_str(), iHitsInSVD, 0, iHitsInSVD);
  m_HitsSVD->GetXaxis()->SetTitle("# hits");
  m_HitsSVD->GetYaxis()->SetTitle("counts");
  name = str(format("NoOfHitsInTrack_CDC"));
  title = str(format("No Of Hits In Track - CDC"));
  m_HitsCDC = new TH1F(name.c_str(), title.c_str(), iHitsInCDC, 0, iHitsInCDC);
  m_HitsCDC->GetXaxis()->SetTitle("# hits");
  m_HitsCDC->GetYaxis()->SetTitle("counts");
  name = str(format("NoOfHitsInTrack"));
  title = str(format("No Of Hits In Track"));
  m_Hits = new TH1F(name.c_str(), title.c_str(), iHits, 0, iHits);
  m_Hits->GetXaxis()->SetTitle("# hits");
  m_Hits->GetYaxis()->SetTitle("counts");

  name = str(format("NoOfTracksInVXDOnly"));
  title = str(format("No Of Tracks Per Event, Only In VXD"));
  m_TracksVXD = new TH1F(name.c_str(), title.c_str(), iTracks, 0, iTracks);
  m_TracksVXD->GetXaxis()->SetTitle("# tracks");
  m_TracksVXD->GetYaxis()->SetTitle("counts");
  name = str(format("NoOfTracksInCDCOnly"));
  title = str(format("No Of Tracks Per Event, Only In CDC"));
  m_TracksCDC = new TH1F(name.c_str(), title.c_str(), iTracks, 0, iTracks);
  m_TracksCDC->GetXaxis()->SetTitle("# tracks");
  m_TracksCDC->GetYaxis()->SetTitle("counts");
  name = str(format("NoOfTracksInVXDCDC"));
  title = str(format("No Of Tracks Per Event, In VXD+CDC"));
  m_TracksVXDCDC = new TH1F(name.c_str(), title.c_str(), iTracks, 0, iTracks);
  m_TracksVXDCDC->GetXaxis()->SetTitle("# tracks");
  m_TracksVXDCDC->GetYaxis()->SetTitle("counts");
  name = str(format("NoOfTracks"));
  title = str(format("No Of All Tracks Per Event"));
  m_Tracks = new TH1F(name.c_str(), title.c_str(), iTracks, 0, iTracks);
  m_Tracks->GetXaxis()->SetTitle("# tracks");
  m_Tracks->GetYaxis()->SetTitle("counts");

  oldDir->cd();

}

void TrackDQMModule::beginRun()
{
  if (m_MomX != NULL) m_MomX->Reset();
  if (m_MomY != NULL) m_MomY->Reset();
  if (m_MomZ != NULL) m_MomZ->Reset();
  if (m_Mom != NULL) m_Mom->Reset();
  if (m_HitsPXD != NULL) m_HitsPXD->Reset();
  if (m_HitsSVD != NULL) m_HitsSVD->Reset();
  if (m_HitsCDC != NULL) m_HitsCDC->Reset();
  if (m_Hits != NULL) m_Hits->Reset();
  if (m_TracksVXD != NULL) m_TracksVXD->Reset();
  if (m_TracksCDC != NULL) m_TracksCDC->Reset();
  if (m_TracksVXDCDC != NULL) m_TracksVXDCDC->Reset();
  if (m_Tracks != NULL) m_Tracks->Reset();
}


void TrackDQMModule::event()
{
  int iTrack = 0;
  int iTrackVXD = 0;
  int iTrackCDC = 0;
  int iTrackVXDCDC = 0;

  StoreArray<Track> tracks;
  for (const Track& track : tracks) {
    RelationVector<RecoTrack> theRC = DataStore::getRelationsWithObj<RecoTrack>(&track);
    RelationVector<PXDCluster> pxdClustersTrack = DataStore::getRelationsWithObj<PXDCluster>(theRC[0]);
    int nPXD = (int)pxdClustersTrack.size();
    RelationVector<SVDCluster> svdClustersTrack = DataStore::getRelationsWithObj<SVDCluster>(theRC[0]);
    int nSVD = (int)svdClustersTrack.size();
    RelationVector<CDCHit> cdcHitTrack = DataStore::getRelationsWithObj<CDCHit>(theRC[0]);
    int nCDC = (int)cdcHitTrack.size();
    const TrackFitResult* tfr = track.getTrackFitResult(Const::pion);
    if (tfr == nullptr) continue;
    TString message = Form("TrackDQM: track %3i, Mom: %f, %f, %f, Pt: %f, Mag: %f, Hits: PXD %i SVD %i CDC %i Suma %i\n",
                           iTrack,
                           (float)tfr->getMomentum().Px(),
                           (float)tfr->getMomentum().Py(),
                           (float)tfr->getMomentum().Pz(),
                           (float)tfr->getMomentum().Pt(),
                           (float)tfr->getMomentum().Mag(),
                           nPXD, nSVD, nCDC, nPXD + nSVD + nCDC
                          );
    B2DEBUG(230, message.Data());
    iTrack++;
    if (((nPXD > 0) || (nSVD > 0)) && (nCDC > 0)) iTrackVXDCDC++;
    if (((nPXD > 0) || (nSVD > 0)) && (nCDC == 0)) iTrackVXD++;
    if (((nPXD == 0) && (nSVD == 0)) && (nCDC > 0)) iTrackCDC++;
    if (m_MomX != NULL) m_MomX->Fill(tfr->getMomentum().Px());
    if (m_MomY != NULL) m_MomY->Fill(tfr->getMomentum().Py());
    if (m_MomZ != NULL) m_MomZ->Fill(tfr->getMomentum().Pz());
    if (m_MomPt != NULL) m_MomPt->Fill(tfr->getMomentum().Pt());
    if (m_Mom != NULL) m_Mom->Fill(tfr->getMomentum().Mag());
    if (m_HitsPXD != NULL) m_HitsPXD->Fill(nPXD);
    if (m_HitsSVD != NULL) m_HitsSVD->Fill(nSVD);
    if (m_HitsCDC != NULL) m_HitsCDC->Fill(nCDC);
    if (m_Hits != NULL) m_Hits->Fill(nPXD + nSVD + nCDC);
  }
  if (m_TracksVXD != NULL) m_TracksVXD->Fill(iTrackVXD);
  if (m_TracksCDC != NULL) m_TracksCDC->Fill(iTrackCDC);
  if (m_TracksVXDCDC != NULL) m_TracksVXDCDC->Fill(iTrackVXDCDC);
  if (m_Tracks != NULL) m_Tracks->Fill(iTrack);
}


void TrackDQMModule::endRun()
{
}


void TrackDQMModule::terminate()
{
}
