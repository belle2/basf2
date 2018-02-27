/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/svdROIFinder/SVDROIFinderAnalysisDataModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/RelationArray.h>

#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <list>
#include <iostream>
#include <TVector3.h>

#include <vxd/geometry/GeoCache.h>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDROIFinderAnalysisData)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDROIFinderAnalysisDataModule::SVDROIFinderAnalysisDataModule() : Module()
  , m_recoTrackListName()
  , m_SVDInterceptListName()
  , m_ROIListName()
  , m_rootFilePtr(NULL)
  , m_rootFileName("")
  , m_writeToRoot(false)
  , m_rootEvent(-1)
  //all tracks
  , m_h1Track(NULL)
  , m_h1Track_pt(NULL)
  , m_h1Track_phi(NULL)
  , m_h1Track_lambda(NULL)
  , m_h1Track_cosTheta(NULL)
  , m_h1Track_pVal(NULL)
  , m_h1Track_nSVDhits(NULL)
  , m_h1Track_nCDChits(NULL)
  //tracks with an attached ROI
  , m_h1ROItrack_pt(NULL)
  , m_h1ROItrack_phi(NULL)
  , m_h1ROItrack_lambda(NULL)
  , m_h1ROItrack_cosTheta(NULL)
  , m_h1ROItrack_pVal(NULL)
  , m_h1ROItrack_nSVDhits(NULL)
  , m_h1ROItrack_nCDChits(NULL)
  //tracks with an attached Good ROI
  , m_h1GoodROItrack_pt(NULL)
  , m_h1GoodROItrack_phi(NULL)
  , m_h1GoodROItrack_lambda(NULL)
  , m_h1GoodROItrack_cosTheta(NULL)
  , m_h1GoodROItrack_pVal(NULL)
  , m_h1GoodROItrack_nSVDhits(NULL)
  , m_h1GoodROItrack_nCDChits(NULL)
  //tracks with an attached Good ROI contaning at least one SVDShaperDigit
  , m_h1FullROItrack_pt(NULL)
  , m_h1FullROItrack_phi(NULL)
  , m_h1FullROItrack_lambda(NULL)
  , m_h1FullROItrack_cosTheta(NULL)
  , m_h1FullROItrack_pVal(NULL)
  , m_h1FullROItrack_nSVDhits(NULL)
  , m_h1FullROItrack_nCDChits(NULL)
  //digits inside ROI
  , m_h1PullU(NULL)
  , m_h1PullV(NULL)
  , m_h2sigmaUphi(NULL)
  , m_h2sigmaVphi(NULL)
  , m_h1ResidU(NULL)
  , m_h1ResidV(NULL)
  , m_h1SigmaU(NULL)
  , m_h1SigmaV(NULL)
  , m_h1GlobalTime(NULL)
  //digits outside2 ROI
  , m_h2sigmaUphi_out(NULL)
  , m_h2sigmaVphi_out(NULL)
  , m_h1ResidU_out(NULL)
  , m_h1ResidV_out(NULL)
  , m_h1SigmaU_out(NULL)
  , m_h1SigmaV_out(NULL)
  , m_h1GlobalTime_out(NULL)
  //ROI stuff
  , m_h2ROIuMinMax(NULL)
  , m_h2ROIvMinMax(NULL)
  , m_h1totROIs(NULL)
  , m_h1goodROIs(NULL)
  , m_h1okROIs(NULL)
  , m_h1effROIs(NULL)
  //  , m_h1totUstrips(NULL)
  //  , m_h1totVstrips(NULL)
  //variables
  , m_globalTime(0.)
  , m_coorU(0.)
  , m_coorV(0.)
  , m_sigmaU(0.)
  , m_sigmaV(0.)
  , m_vxdID(-1)
  , n_rois(0)
  , n_intercepts(0)
  , n_tracks(0)
{
  //Set module properties
  setDescription("This module performs the analysis of the SVDROIFinder module output ON DATA");

  addParam("writeToRoot", m_writeToRoot,
           "set true if you want to save the informations in a root file named by parameter 'rootFileName'", bool(true));

  addParam("rootFileName", m_rootFileName,
           "fileName used for . Will be ignored if parameter 'writeToRoot' is false (standard)",
           string("svdDataRedAnalysisData"));

  addParam("recoTrackListName", m_recoTrackListName,
           "name of the input collection of RecoTracks", std::string(""));

  addParam("shapers", m_shapersName,
           "name of the input collection of SVDShaperDigits", std::string(""));

  addParam("SVDInterceptListName", m_SVDInterceptListName,
           "name of the list of interceptions", std::string(""));

  addParam("ROIListName", m_ROIListName,
           "name of the list of ROIs", std::string(""));

  m_rootEvent = 0;
}

SVDROIFinderAnalysisDataModule::~SVDROIFinderAnalysisDataModule()
{
}


void SVDROIFinderAnalysisDataModule::initialize()
{

  m_shapers.isRequired(m_shapersName);
  m_trackList.isRequired(m_recoTrackListName);
  m_ROIs.isRequired(m_ROIListName);
  m_SVDIntercepts.isRequired(m_SVDInterceptListName);

  n_rois           = 0;
  m_nGoodROIs = 0; //data
  m_nOkROIs = 0; //data
  n_intercepts     = 0;
  n_tracks         = 0;

  if (m_writeToRoot == true) {
    m_rootFileName += ".root";
    m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");
  } else
    m_rootFilePtr = NULL;


  m_h1GlobalTime = new TH1F("hGlobalTime", "global time for SVDShaperDigits contained in ROI", 200, -100, 100);
  m_h1PullU = new TH1F("hPullU", "U pulls for SVDShaperDigits contained in ROI", 100, -6, 6);
  m_h1PullV = new TH1F("hPullV", "V pulls for SVDShaperDigits contained in ROI", 100, -6, 6);
  m_h2sigmaUphi = new TH2F("hsigmaUvsPhi", "sigmaU vs phi digits in ROI", 100, -180, 180, 100, 0, 0.35);
  m_h2sigmaVphi = new TH2F("hsigmaVvsPhi", "sigmaU vs phi digits in ROI", 100, -180, 180, 100, 0, 0.4);
  m_h1ResidU = new TH1F("hResidU", "U resid for SVDShaperDigits contained in ROI", 100, -0.5, 0.5);
  m_h1ResidV = new TH1F("hResidV", "V resid for SVDShaperDigits contained in ROI", 100, -0.5, 0.5);
  m_h1SigmaU = new TH1F("hSigmaU", "sigmaU for SVDShaperDigits contained in ROI", 100, 0, 0.35);
  m_h1SigmaV = new TH1F("hSigmaV", "sigmaV for SVDShaperDigits contained in ROI", 100, 0, 0.35);

  /*
  m_h1GlobalTime_out = new TH1F("hGlobalTime_out", "global time for SVDShaperDigits not contained in ROI", 200, -100, 100);
  m_h2sigmaUphi_out = new TH2F("hsigmaUvsPhi_out", "sigmaU vs phi digits not contained in ROI", 100, -180, 180, 100, 0, 0.35);
  m_h2sigmaVphi_out = new TH2F("hsigmaVvsPhi_out", "sigmaU vs phi digits not contained in ROI", 100, -180, 180, 100, 0, 0.4);
  m_h1ResidU_out = new TH1F("hResidU_out", "U resid for SVDShaperDigits not contained in ROI", 100, -2.5, 2.5);
  m_h1ResidV_out = new TH1F("hResidV_out", "V resid for SVDShaperDigits not contained in ROI", 100, -2.5, 2.5);
  m_h1SigmaU_out = new TH1F("hSigmaU_out", "sigmaU for SVDShaperDigits not contained in ROI", 100, 0, 0.35);
  m_h1SigmaV_out = new TH1F("hSigmaV_out", "sigmaV for SVDShaperDigits not contained in ROI", 100, 0, 0.35);
  */

  m_h1totROIs = new TH1F("h1TotNROIs", "number of all ROIs", 110, 0, 110);
  m_h1goodROIs = new TH1F("h1GoodNROIs", "number of ROIs from Good Track", 110, 0, 110);
  m_h1okROIs = new TH1F("h1OkNROIs", "number of Good ROIs containing a SVDShaperDigit", 110, 0, 110);
  m_h1effROIs = new TH1F("h1EffSVD", "fraction of Good ROIs containing a SVDShaperDigit", 100, 0, 1.1);

  //  m_h1totUstrips = new TH1F("h1TotUstrips", "number of U strips in ROIs", 100, 0, 250000);
  //  m_h1totVstrips = new TH1F("h1TotVstrips", "number of V strips in ROIs", 100, 0, 250000);


  m_h2ROIuMinMax = new TH2F("h2ROIuMinMax", "u Min vs Max", 960, -100, 860, 960, -100, 860);
  m_h2ROIvMinMax = new TH2F("h2ROIvMinMax", "v Min vs Max", 960, -100, 860, 960, -100, 860);


  //analysis
  /*  Double_t lowBin[6 + 1];
  for (int i = 0; i < 6; i++)
    lowBin[i] = pt[i] - ptErr[i];
  lowBin[6] = pt[5] + ptErr[5];
  */

  m_h1ROItrack = new TH1F("hROITrack", "track with an attached Good ROI", 20, 0, 20);
  m_h1ROItrack_pt = new TH1F("hROITrack_pT", "Track with an attached Good ROI, Transverse Momentum", 100, 0, 8);
  m_h1ROItrack_phi = new TH1F("h1ROITrack_phi", "Track with an attached Good ROI, Momentum Phi", 200, -TMath::Pi() - 0.01,
                              TMath::Pi() + 0.01);
  m_h1ROItrack_lambda = new TH1F("h1ROITrack_lambda", "Track with an attached Good ROI, Lambda", 100, -TMath::Pi() - 0.01,
                                 TMath::Pi() + 0.01);
  m_h1ROItrack_cosTheta = new TH1F("h1ROITrack_cosTheta", "Track with an attached Good ROI, Momentum CosTheta", 100, -1 - 0.01, 1.01);
  m_h1ROItrack_pVal = new TH1F("h1ROITrack_pVal", "Track with an attached Good ROI, P-Value", 1000, 0, 1 + 0.01);
  m_h1ROItrack_nSVDhits = new TH1F("h1ROITrack_nSVDhits", "RecoTrack with an attached Good ROI, number of SVD hits", 50, 0, 50);
  m_h1ROItrack_nCDChits = new TH1F("h1ROITrack_nCDChits", "RecoTrack with an attached Good ROI, number of CDC hits", 100, 0, 100);

  m_h1FullROItrack = new TH1F("hFullROITrack", "track with an attached Full ROI", 20, 0, 20);
  m_h1FullROItrack_pt = new TH1F("hFullROITrack_pT", "Track with an attached Full ROI, Transverse Momentum", 100, 0, 8);
  m_h1FullROItrack_phi = new TH1F("h1FullROITrack_phi", "Track with an attached Full ROI, Momentum Phi", 200, -TMath::Pi() - 0.01,
                                  TMath::Pi() + 0.01);
  m_h1FullROItrack_lambda = new TH1F("h1FullROITrack_lambda", "Track with an attached Full ROI, Lambda", 100, -TMath::Pi() - 0.01,
                                     TMath::Pi() + 0.01);
  m_h1FullROItrack_cosTheta = new TH1F("h1FullROITrack_cosTheta", "Track with an attached Full ROI, Momentum CosTheta", 100,
                                       -1 - 0.01, 1.01);
  m_h1FullROItrack_pVal = new TH1F("h1FullROITrack_pVal", "Track with an attached Full ROI, P-Value", 1000, 0, 1 + 0.01);
  m_h1FullROItrack_nSVDhits = new TH1F("h1FullROITrack_nSVDhits", "RecoTrack with an attached Full ROI, number of SVD hits", 50, 0,
                                       50);
  m_h1FullROItrack_nCDChits = new TH1F("h1FullROITrack_nCDChits", "RecoTrack with an attached Full ROI, number of CDC hits", 100, 0,
                                       100);

  m_h1GoodROItrack = new TH1F("hGoodROITrack", "track with an attached Good ROI", 20, 0, 20);
  m_h1GoodROItrack_pt = new TH1F("hGoodROITrack_pT", "Track with an attached Good ROI, Transverse Momentum", 100, 0, 8);
  m_h1GoodROItrack_phi = new TH1F("h1GoodROITrack_phi", "Track with an attached Good ROI, Momentum Phi", 200, -TMath::Pi() - 0.01,
                                  TMath::Pi() + 0.01);
  m_h1GoodROItrack_lambda = new TH1F("h1GoodROITrack_lambda", "Track with an attached Good ROI, Lambda", 100, -TMath::Pi() - 0.01,
                                     TMath::Pi() + 0.01);
  m_h1GoodROItrack_cosTheta = new TH1F("h1GoodROITrack_cosTheta", "Track with an attached Good ROI, Momentum CosTheta", 100,
                                       -1 - 0.01, 1.01);
  m_h1GoodROItrack_pVal = new TH1F("h1GoodROITrack_pVal", "Track with an attached Good ROI, P-Value", 1000, 0, 1 + 0.01);
  m_h1GoodROItrack_nSVDhits = new TH1F("h1GoodROITrack_nSVDhits", "RecoTrack with an attached Good ROI, number of SVD hits", 50, 0,
                                       50);
  m_h1GoodROItrack_nCDChits = new TH1F("h1GoodROITrack_nCDChits", "RecoTrack with an attached Good ROI, number of CDC hits", 100, 0,
                                       100);

  m_h1Track = new TH1F("hTrack", "Number of Tracks per Event", 20, 0, 20);
  m_h1Track_pt = new TH1F("hTrack_pT", "Track Transverse Momentum", 100, 0, 8);
  m_h1Track_lambda = new TH1F("h1Track_lambda", "Track Momentum Lambda", 100, -TMath::Pi() + 0.01, TMath::Pi() + 0.01);
  m_h1Track_phi = new TH1F("h1Track_phi", "Track momentum Phi", 200, -TMath::Pi() - 0.01, TMath::Pi() + 0.01);
  m_h1Track_cosTheta = new TH1F("h1Track_cosTheta", "Track Momentum CosTheta", 100, -1 - 0.01, 1 + 0.01);
  m_h1Track_pVal = new TH1F("h1Track_pVal", "Track P-Value", 1000, 0, 1 + 0.01);
  m_h1Track_nSVDhits = new TH1F("h1Track_nSVDhits", "RecoTrack, number of SVD hits", 50, 0, 50);
  m_h1Track_nCDChits = new TH1F("h1Track_nCDChits", "RecoTrack, number of CDC hits", 100, 0, 100);

  //  m_h1digiIn = new TH1F("hdigiIn", "digits inside ROI", 6, lowBin);
  //  m_h1digiOut = new TH1F("hdigiOut", "digits outside ROI", 6, lowBin);

  m_rootEvent = 0;
}

void SVDROIFinderAnalysisDataModule::event()
{

  B2DEBUG(1, "  ++++++++++++++ SVDROIFinderAnalysisDataModule");

  int nGoodROIs = 0;
  int nOkROIs = 0;

  StoreArray<Track> tracks;

  //Tracks generals
  for (int i = 0; i < (int)tracks.getEntries(); i++) { //loop on all Tracks


    RelationVector<RecoTrack> theRC = DataStore::getRelationsWithObj<RecoTrack>(tracks[i]);

    const TrackFitResult* tfr = tracks[i]->getTrackFitResultWithClosestMass(Const::pion);

    TVector3 mom = tfr->getMomentum();
    m_h1Track_pt->Fill(mom.Perp());
    m_h1Track_phi->Fill(mom.Phi());
    m_h1Track_cosTheta->Fill(mom.CosTheta());
    m_h1Track_lambda->Fill(TMath::Pi() / 2 - mom.Theta());
    m_h1Track_pVal->Fill(tfr->getPValue());
    m_h1Track_nSVDhits->Fill(theRC[0]->getNumberOfSVDHits());
    m_h1Track_nCDChits->Fill(theRC[0]->getNumberOfCDCHits());

  }
  m_h1Track->Fill(tracks.getEntries());

  //ROIs general
  for (int i = 0; i < (int)m_ROIs.getEntries(); i++) { //loop on ROIlist

    m_h2ROIuMinMax->Fill(m_ROIs[i]->getMinUid(), m_ROIs[i]->getMaxUid());
    m_h2ROIvMinMax->Fill(m_ROIs[i]->getMinVid(), m_ROIs[i]->getMaxVid());

    RelationVector<SVDIntercept> theIntercept = DataStore::getRelationsWithObj<SVDIntercept>(m_ROIs[i]);
    RelationVector<RecoTrack> theRC = DataStore::getRelationsWithObj<RecoTrack>(theIntercept[0]);

    if (!theRC[0]->wasFitSuccessful())
      break;

    RelationVector<Track> theTrack = DataStore::getRelationsWithObj<Track>(theRC[0]);

    const TrackFitResult* tfr = theTrack[0]->getTrackFitResultWithClosestMass(Const::pion);

    TVector3 mom = tfr->getMomentum();
    m_h1ROItrack->Fill(1);
    m_h1ROItrack_pt->Fill(mom.Perp());
    m_h1ROItrack_phi->Fill(mom.Phi());
    m_h1ROItrack_cosTheta->Fill(mom.CosTheta());
    m_h1ROItrack_lambda->Fill(TMath::Pi() / 2 - mom.Theta());
    m_h1ROItrack_pVal->Fill(tfr->getPValue());
    m_h1ROItrack_nSVDhits->Fill(theRC[0]->getNumberOfSVDHits());
    m_h1ROItrack_nCDChits->Fill(theRC[0]->getNumberOfCDCHits());

    if (tfr->getPValue() < 0.001)
      break;

    nGoodROIs++;

    m_h1GoodROItrack->Fill(1);
    m_h1GoodROItrack_pt->Fill(mom.Perp());
    m_h1GoodROItrack_phi->Fill(mom.Phi());
    m_h1GoodROItrack_cosTheta->Fill(mom.CosTheta());
    m_h1GoodROItrack_lambda->Fill(TMath::Pi() / 2 - mom.Theta());
    m_h1GoodROItrack_pVal->Fill(tfr->getPValue());
    m_h1GoodROItrack_nSVDhits->Fill(theRC[0]->getNumberOfSVDHits());
    m_h1GoodROItrack_nCDChits->Fill(theRC[0]->getNumberOfCDCHits());

    for (int s = 0; s < m_shapers.getEntries(); s++) {
      if (m_ROIs[i]->Contains(*(m_shapers[s]))) {
        nOkROIs++;

        m_h1FullROItrack->Fill(1);
        m_h1FullROItrack_pt->Fill(mom.Perp());
        m_h1FullROItrack_phi->Fill(mom.Phi());
        m_h1FullROItrack_cosTheta->Fill(mom.CosTheta());
        m_h1FullROItrack_lambda->Fill(TMath::Pi() / 2 - mom.Theta());
        m_h1FullROItrack_pVal->Fill(tfr->getPValue());
        m_h1FullROItrack_nSVDhits->Fill(theRC[0]->getNumberOfSVDHits());
        m_h1FullROItrack_nCDChits->Fill(theRC[0]->getNumberOfCDCHits());

        break;
      }

    }
    m_nGoodROIs += nGoodROIs;
    m_h1goodROIs->Fill(nGoodROIs);
    m_nOkROIs += nOkROIs;
    m_h1okROIs->Fill(nOkROIs);

  }
  m_h1totROIs->Fill(m_ROIs.getEntries());
  if (nGoodROIs > 0)
    m_h1effROIs->Fill((float) nOkROIs / nGoodROIs);
  n_rois += m_ROIs.getEntries();

  //RecoTrack general
  n_tracks += m_trackList.getEntries();

  //SVDIntercepts general
  n_intercepts += m_SVDIntercepts.getEntries();

  m_rootEvent++;

  if (nGoodROIs > 0) {
    B2RESULT(" o  SVDROIFinder ANALYSIS: good ROIs = " << nGoodROIs << ", with shapers = " << nOkROIs);

    for (int i = 0; i < m_ROIs.getEntries(); i++) {
      VxdID sensor = m_ROIs[i]->getSensorID();
      B2RESULT(i + 1 << ") ROI " << sensor.getLayerNumber() << "." << sensor.getLadderNumber() << "." << sensor.getSensorNumber() <<
               ": U side " << m_ROIs[i]->getMinUid() << "->" << m_ROIs[i]->getMaxUid() << ", V side " << m_ROIs[i]->getMinVid() << "->" <<
               m_ROIs[i]->getMaxVid());
    }
  }
  if (nGoodROIs > m_ROIs.getEntries()) B2RESULT(" HOUSTON WE HAVE A PROBLEM!");

}


void SVDROIFinderAnalysisDataModule::terminate()
{

  B2RESULT("     ROI AnalysisData Summary     ");
  B2RESULT("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  B2RESULT("");
  B2RESULT(" number of events = " <<   m_rootEvent);
  B2RESULT(" number of tracks = " << n_tracks);
  B2RESULT(" number of Intercepts = " << n_intercepts);
  B2RESULT(" number of ROIs = " << n_rois);
  B2RESULT(" number of Good ROIs = " << m_nGoodROIs);
  if (m_nGoodROIs > 0) {
    B2RESULT(" number of Good ROIs with SVDShaperDigit= " << m_nOkROIs);
    B2RESULT(" SVD INefficiency = " << 1 - (float)m_nOkROIs / m_nGoodROIs);
    B2RESULT(" average SVD INefficiency = " << 1 - m_h1effROIs->GetMean());
    B2RESULT(" number of EMPTY ROIs = " << m_nGoodROIs - m_nOkROIs);
  }



  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd(); //important! without this the famework root I/O (SimpleOutput etc) could mix with the root I/O of this module

    TDirectory* oldDir = gDirectory;
    //    TDirectory* m_digiDir = oldDir->mkdir("digits");
    TDirectory* m_alltracks = oldDir->mkdir("ALLtracks");
    TDirectory* m_roitracks = oldDir->mkdir("ROItracks");
    TDirectory* m_goodroitracks = oldDir->mkdir("GoodROItracks");
    TDirectory* m_fullroitracks = oldDir->mkdir("FullROItracks");
    //    TDirectory* m_in = oldDir->mkdir("digi_in");
    //    TDirectory* m_out = oldDir->mkdir("digi_out");
    TDirectory* m_ROIDir = oldDir->mkdir("roi");
    /*
    m_digiDir->cd();
    m_h1digiIn->Write();
    m_h1digiOut->Write();
    */

    m_alltracks->cd();
    m_h1Track->Write();
    m_h1Track_pt->Write();
    m_h1Track_phi->Write();
    m_h1Track_lambda->Write();
    m_h1Track_cosTheta->Write();
    m_h1Track_pVal->Write();
    m_h1Track_nSVDhits->Write();
    m_h1Track_nCDChits->Write();

    m_roitracks->cd();
    m_h1ROItrack->Write();
    m_h1ROItrack_pt->Write();
    m_h1ROItrack_phi->Write();
    m_h1ROItrack_lambda->Write();
    m_h1ROItrack_cosTheta->Write();
    m_h1ROItrack_nSVDhits->Write();
    m_h1ROItrack_nCDChits->Write();

    m_goodroitracks->cd();
    m_h1GoodROItrack->Write();
    m_h1GoodROItrack_pt->Write();
    m_h1GoodROItrack_phi->Write();
    m_h1GoodROItrack_lambda->Write();
    m_h1GoodROItrack_cosTheta->Write();
    m_h1GoodROItrack_nSVDhits->Write();
    m_h1GoodROItrack_nCDChits->Write();

    m_fullroitracks->cd();
    m_h1FullROItrack->Write();
    m_h1FullROItrack_pt->Write();
    m_h1FullROItrack_phi->Write();
    m_h1FullROItrack_lambda->Write();
    m_h1FullROItrack_cosTheta->Write();
    m_h1FullROItrack_nSVDhits->Write();
    m_h1FullROItrack_nCDChits->Write();
    /*
    m_in->cd();
    m_h1GlobalTime->Write();
    m_h1PullU->Write();
    m_h1PullV->Write();
    m_h1ResidU->Write();
    m_h1ResidV->Write();
    m_h1SigmaU->Write();
    m_h1SigmaV->Write();
    m_h2sigmaUphi->Write();
    m_h2sigmaVphi->Write();

    m_out->cd();
    m_h1GlobalTime_out->Write();
    m_h1ResidU_out->Write();
    m_h1ResidV_out->Write();
    m_h1SigmaU_out->Write();
    m_h1SigmaV_out->Write();
    m_h2sigmaUphi_out->Write();
    m_h2sigmaVphi_out->Write();
    */

    m_ROIDir->cd();
    //    m_h1totUstrips->Write();
    //    m_h1totVstrips->Write();
    m_h1effROIs->Write();
    m_h1totROIs->Write();
    m_h1goodROIs->Write();
    m_h1okROIs->Write();
    m_h2ROIuMinMax->Write();
    m_h2ROIvMinMax->Write();

    m_rootFilePtr->Close();

  }

}

