/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for cluster shape correction quality check                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <genfit/MeasurementOnPlane.h>
#include <tracking/modules/trackingDQM/TrackDQMModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>

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

  StoreArray<RecoTrack> recotracks(m_storeRecoTrackName);
  m_storeRecoTrackName = recotracks.getName();
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
  name = str(format("TrackMomentum"));
  title = str(format("Track Momentum"));
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
  StoreArray<RecoTrack> recotracks(m_storeRecoTrackName);
  int iTrack = 0;
  int iTrackVXD = 0;
  int iTrackCDC = 0;
  int iTrackVXDCDC = 0;
  for (auto& recoTrack : recotracks) {  // over recotracks
    if (!recoTrack.wasFitSuccessful())
      continue;
    if (!recoTrack.getTrackFitStatus())
      continue;
    auto& genfitTrack = RecoTrackGenfitAccess::getGenfitTrack(recoTrack);
    TString message = Form("TrackDQM: track %3i, Mom: %f, %f, %f, Pt: %f +- %f, Hits: ",
                           iTrack,
                           genfitTrack.getFittedState().getMom().X(),
                           genfitTrack.getFittedState().getMom().Y(),
                           genfitTrack.getFittedState().getMom().Z(),
                           genfitTrack.getFittedState().getMom().Perp(),
                           genfitTrack.getFittedState().getMomVar()
                          );
    int valAll = 0;
    int val = 0;
    if (recoTrack.getNumberOfPXDHits() > 0) {
      val = recoTrack.getNumberOfPXDHits();
    }
    message = Form("%sPXD %i ", message.Data(), val);
    valAll += val;
    val = 0;
    if (recoTrack.getNumberOfSVDHits() > 0) {
      val = recoTrack.getNumberOfSVDHits();
    }
    message = Form("%sSVD %i ", message.Data(), val);
    valAll += val;
    val = 0;
    if (recoTrack.getNumberOfCDCHits() > 0) {
      val = recoTrack.getNumberOfCDCHits();
    }
    message = Form("%sCDC %i ", message.Data(), val);
    valAll += val;
    message = Form("%sSuma %i (%i) ", message.Data(), valAll, genfitTrack.getNumPoints());
    B2DEBUG(230, message.Data());

    iTrack++;
    if (((recoTrack.getNumberOfPXDHits() > 0) || (recoTrack.getNumberOfSVDHits() > 0)) &&
        (recoTrack.getNumberOfCDCHits() > 0)) iTrackVXDCDC++;
    if (((recoTrack.getNumberOfPXDHits() > 0) || (recoTrack.getNumberOfSVDHits() > 0)) &&
        (recoTrack.getNumberOfCDCHits() == 0)) iTrackVXD++;
    if (((recoTrack.getNumberOfPXDHits() == 0) && (recoTrack.getNumberOfSVDHits() == 0)) &&
        (recoTrack.getNumberOfCDCHits() > 0)) iTrackCDC++;
    if (m_MomX != NULL) m_MomX->Fill(genfitTrack.getFittedState().getMom().X());
    if (m_MomY != NULL) m_MomY->Fill(genfitTrack.getFittedState().getMom().Y());
    if (m_MomZ != NULL) m_MomZ->Fill(genfitTrack.getFittedState().getMom().Z());
    if (m_Mom != NULL) m_Mom->Fill(genfitTrack.getFittedState().getMom().Perp());
    if (m_HitsPXD != NULL) m_HitsPXD->Fill(recoTrack.getNumberOfPXDHits());
    if (m_HitsSVD != NULL) m_HitsSVD->Fill(recoTrack.getNumberOfSVDHits());
    if (m_HitsCDC != NULL) m_HitsCDC->Fill(recoTrack.getNumberOfCDCHits());
    // if (m_Hits != NULL) m_Hits->Fill(genfitTrack.getNumPoints()); // not works for Phase2 geometry!!!
    if (m_Hits != NULL) m_Hits->Fill(recoTrack.getNumberOfPXDHits() +
                                       recoTrack.getNumberOfSVDHits() + recoTrack.getNumberOfCDCHits()
                                      );
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
