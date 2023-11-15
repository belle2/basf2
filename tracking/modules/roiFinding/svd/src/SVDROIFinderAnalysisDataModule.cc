/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/roiFinding/svd/SVDROIFinderAnalysisDataModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/ROIid.h>
#include <tracking/dataobjects/SVDIntercept.h>
#include <svd/dataobjects/SVDShaperDigit.h>

#include <iostream>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDROIFinderAnalysisData);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDROIFinderAnalysisDataModule::SVDROIFinderAnalysisDataModule() : Module()
  , m_recoTrackListName()
  , m_SVDInterceptListName()
  , m_ROIListName()
  , m_rootFilePtr(nullptr)
  , m_rootFileName("")
  , m_writeToRoot(false)
  , m_rootEvent(0)
  //all tracks
  , m_h1Track(nullptr)
  , m_h1Track_pt(nullptr)
  , m_h1Track_phi(nullptr)
  , m_h1Track_lambda(nullptr)
  , m_h1Track_cosTheta(nullptr)
  , m_h1Track_pVal(nullptr)
  //tracks with an attached ROI
  , m_h1ROItrack(nullptr)
  , m_h1ROItrack_pt(nullptr)
  , m_h1ROItrack_phi(nullptr)
  , m_h1ROItrack_lambda(nullptr)
  , m_h1ROItrack_cosTheta(nullptr)
  , m_h1ROItrack_pVal(nullptr)
  //tracks with an attached Good ROI
  , m_h1GoodROItrack(nullptr)
  , m_h1GoodROItrack_pt(nullptr)
  , m_h1GoodROItrack_phi(nullptr)
  , m_h1GoodROItrack_lambda(nullptr)
  , m_h1GoodROItrack_cosTheta(nullptr)
  , m_h1GoodROItrack_pVal(nullptr)
  //tracks with an attached Good ROI contaning at least one SVDShaperDigit
  , m_h1FullROItrack(nullptr)
  , m_h1FullROItrack_pt(nullptr)
  , m_h1FullROItrack_phi(nullptr)
  , m_h1FullROItrack_lambda(nullptr)
  , m_h1FullROItrack_cosTheta(nullptr)
  , m_h1FullROItrack_pVal(nullptr)
  //digits inside ROI
  , m_h1PullU(nullptr)
  , m_h1PullV(nullptr)
  , m_h2sigmaUphi(nullptr)
  , m_h2sigmaVphi(nullptr)
  , m_h1ResidU(nullptr)
  , m_h1ResidV(nullptr)
  , m_h1SigmaU(nullptr)
  , m_h1SigmaV(nullptr)
  , m_h1GlobalTime(nullptr)
  //ROI stuff
  , m_h2ROIuMinMax(nullptr)
  , m_h2ROIvMinMax(nullptr)
  , m_h2ROIcenters(nullptr)
  , m_h2GoodROIcenters(nullptr)
  , m_h2FullROIcenters(nullptr)
  , m_h1totROIs(nullptr)
  , m_h1goodROIs(nullptr)
  , m_h1okROIs(nullptr)
  , m_h1effROIs(nullptr)
  //  , m_h1totUstrips(nullptr)
  //  , m_h1totVstrips(nullptr)
  //variables
  , n_rois(0)
  , m_nGoodROIs(0)
  , m_nOkROIs(0)
  , n_intercepts(0)
  , n_tracks(0)
{
  //Set module properties
  setDescription("This module performs the analysis of the SVDROIFinder module output ON DATA");

  addParam("writeToRoot", m_writeToRoot,
           "set true if you want to save the informations in a root file named by parameter 'rootFileName'", bool(true));

  addParam("rootFileName", m_rootFileName,
           "fileName used for . Will be ignored if parameter 'writeToRoot' is false (standard)",
           std::string("svdDataRedAnalysisData"));

  addParam("recoTrackListName", m_recoTrackListName,
           "name of the input collection of RecoTracks", std::string(""));

  addParam("shapers", m_shapersName,
           "name of the input collection of SVDShaperDigits", std::string(""));

  addParam("SVDInterceptListName", m_SVDInterceptListName,
           "name of the list of interceptions", std::string(""));

  addParam("ROIListName", m_ROIListName,
           "name of the list of ROIs", std::string(""));

  addParam("edgeU", m_edgeU, "fiducial region: edge U [mm]", float(10));
  addParam("edgeV", m_edgeV, "fiducial region: edge V [mm]", float(10));
  addParam("minPVal", m_minPVal, "fiducial region: minimum track P-Value", float(0.001));

}

SVDROIFinderAnalysisDataModule::~SVDROIFinderAnalysisDataModule()
{
}


void SVDROIFinderAnalysisDataModule::initialize()
{

  m_shapers.isRequired(m_shapersName);
  m_recoTracks.isRequired(m_recoTrackListName);
  m_tracks.isRequired();
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
    m_rootFilePtr = nullptr;


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


  m_h2ROIuMinMax = new TH2F("h2ROIuMinMax", "u Min vs Max (all ROIs)", 960, -100, 860, 960, -100, 860);
  m_h2ROIvMinMax = new TH2F("h2ROIvMinMax", "v Min vs Max (all ROIs)", 960, -100, 860, 960, -100, 860);
  m_h2ROIcenters = new TH2F("h2ROIcenters", "ROI Centers", 768, 0, 768, 512, 0, 512);
  m_h2GoodROIcenters = new TH2F("h2GoodROIcenters", "Good ROI Centers", 768, 0, 768, 512, 0, 512);
  m_h2FullROIcenters = new TH2F("h2FullROIcenters", "Full ROI Centers", 768, 0, 768, 512, 0, 512);

  //analysis
  /*  Double_t lowBin[6 + 1];
  for (int i = 0; i < 6; i++)
    lowBin[i] = pt[i] - ptErr[i];
  lowBin[6] = pt[5] + ptErr[5];
  */

  m_h1ROItrack = new TH1F("hROITrack", "track with an attached Good ROI", 2, 0, 2);
  m_h1ROItrack_pt = new TH1F("hROITrack_pT", "Track with an attached Good ROI, Transverse Momentum", 100, 0, 8);
  m_h1ROItrack_phi = new TH1F("h1ROITrack_phi", "Track with an attached Good ROI, Momentum Phi", 200, -TMath::Pi() - 0.01,
                              TMath::Pi() + 0.01);
  m_h1ROItrack_lambda = new TH1F("h1ROITrack_lambda", "Track with an attached Good ROI, Lambda", 100, -TMath::Pi() - 0.01,
                                 TMath::Pi() + 0.01);
  m_h1ROItrack_cosTheta = new TH1F("h1ROITrack_cosTheta", "Track with an attached Good ROI, Momentum CosTheta", 100, -1 - 0.01, 1.01);
  m_h1ROItrack_pVal = new TH1F("h1ROITrack_pVal", "Track with an attached Good ROI, P-Value", 1000, 0, 1 + 0.01);

  m_h1FullROItrack = new TH1F("hFullROITrack", "track with an attached Full ROI", 20, 0, 20);
  m_h1FullROItrack_pt = new TH1F("hFullROITrack_pT", "Track with an attached Full ROI, Transverse Momentum", 100, 0, 8);
  m_h1FullROItrack_phi = new TH1F("h1FullROITrack_phi", "Track with an attached Full ROI, Momentum Phi", 200, -TMath::Pi() - 0.01,
                                  TMath::Pi() + 0.01);
  m_h1FullROItrack_lambda = new TH1F("h1FullROITrack_lambda", "Track with an attached Full ROI, Lambda", 100, -TMath::Pi() - 0.01,
                                     TMath::Pi() + 0.01);
  m_h1FullROItrack_cosTheta = new TH1F("h1FullROITrack_cosTheta", "Track with an attached Full ROI, Momentum CosTheta", 100,
                                       -1 - 0.01, 1.01);
  m_h1FullROItrack_pVal = new TH1F("h1FullROITrack_pVal", "Track with an attached Full ROI, P-Value", 1000, 0, 1 + 0.01);

  m_h1GoodROItrack = new TH1F("hGoodROITrack", "track with an attached Good ROI", 20, 0, 20);
  m_h1GoodROItrack_pt = new TH1F("hGoodROITrack_pT", "Track with an attached Good ROI, Transverse Momentum", 100, 0, 8);
  m_h1GoodROItrack_phi = new TH1F("h1GoodROITrack_phi", "Track with an attached Good ROI, Momentum Phi", 200, -TMath::Pi() - 0.01,
                                  TMath::Pi() + 0.01);
  m_h1GoodROItrack_lambda = new TH1F("h1GoodROITrack_lambda", "Track with an attached Good ROI, Lambda", 100, -TMath::Pi() - 0.01,
                                     TMath::Pi() + 0.01);
  m_h1GoodROItrack_cosTheta = new TH1F("h1GoodROITrack_cosTheta", "Track with an attached Good ROI, Momentum CosTheta", 100,
                                       -1 - 0.01, 1.01);
  m_h1GoodROItrack_pVal = new TH1F("h1GoodROITrack_pVal", "Track with an attached Good ROI, P-Value", 1000, 0, 1 + 0.01);

  m_h1Track = new TH1F("hTrack", "Number of Tracks per Event", 20, 0, 20);
  m_h1Track_pt = new TH1F("hTrack_pT", "Track Transverse Momentum", 100, 0, 8);
  m_h1Track_lambda = new TH1F("h1Track_lambda", "Track Momentum Lambda", 100, -TMath::Pi() + 0.01, TMath::Pi() + 0.01);
  m_h1Track_phi = new TH1F("h1Track_phi", "Track momentum Phi", 200, -TMath::Pi() - 0.01, TMath::Pi() + 0.01);
  m_h1Track_cosTheta = new TH1F("h1Track_cosTheta", "Track Momentum CosTheta", 100, -1 - 0.01, 1 + 0.01);
  m_h1Track_pVal = new TH1F("h1Track_pVal", "Track P-Value", 1000, 0, 1 + 0.01);

  m_rootEvent = 0;
}

void SVDROIFinderAnalysisDataModule::event()
{

  B2DEBUG(21, "  ++++++++++++++ SVDROIFinderAnalysisDataModule");

  int nGoodROIs = 0;
  int nOkROIs = 0;


  //Tracks generals
  for (int i = 0; i < (int)m_tracks.getEntries(); i++) { //loop on all Tracks

    const TrackFitResult* tfr = m_tracks[i]->getTrackFitResultWithClosestMass(Const::pion);

    ROOT::Math::XYZVector mom = tfr->getMomentum();
    m_h1Track_pt->Fill(mom.Rho());
    m_h1Track_phi->Fill(mom.Phi());
    m_h1Track_cosTheta->Fill(cos(mom.Theta()));
    m_h1Track_lambda->Fill(TMath::Pi() / 2 - mom.Theta());
    m_h1Track_pVal->Fill(tfr->getPValue());

  }
  m_h1Track->Fill(m_tracks.getEntries());

  //ROIs general
  for (int i = 0; i < (int)m_ROIs.getEntries(); i++) { //loop on ROIlist

    float centerROIU = (m_ROIs[i]->getMaxUid() + m_ROIs[i]->getMinUid()) / 2;
    float centerROIV = (m_ROIs[i]->getMaxVid() + m_ROIs[i]->getMinVid()) / 2;

    m_h2ROIuMinMax->Fill(m_ROIs[i]->getMinUid(), m_ROIs[i]->getMaxUid());
    m_h2ROIvMinMax->Fill(m_ROIs[i]->getMinVid(), m_ROIs[i]->getMaxVid());
    m_h2ROIcenters->Fill(centerROIU, centerROIV);

    RelationVector<SVDIntercept> theIntercept = DataStore::getRelationsWithObj<SVDIntercept>(m_ROIs[i]);
    RelationVector<RecoTrack> theRC = DataStore::getRelationsWithObj<RecoTrack>(theIntercept[0]);

    if (!theRC[0]->wasFitSuccessful()) {
      m_h1ROItrack->Fill(0);
      continue;
    }

    RelationVector<Track> theTrack = DataStore::getRelationsWithObj<Track>(theRC[0]);

    const TrackFitResult* tfr = theTrack[0]->getTrackFitResultWithClosestMass(Const::pion);

    if (tfr->getPValue() < m_minPVal) {
      m_h1ROItrack->Fill(0);
      continue;
    }

    ROOT::Math::XYZVector mom = tfr->getMomentum();
    m_h1ROItrack->Fill(1);
    m_h1ROItrack_pt->Fill(mom.Rho());
    m_h1ROItrack_phi->Fill(mom.Phi());
    m_h1ROItrack_cosTheta->Fill(cos(mom.Theta()));
    m_h1ROItrack_lambda->Fill(TMath::Pi() / 2 - mom.Theta());
    m_h1ROItrack_pVal->Fill(tfr->getPValue());


    VxdID sensorID = m_ROIs[i]->getSensorID();

    float nStripsU = 768;
    float nStripsV = 512;
    float centerSensorU = nStripsU / 2;
    float centerSensorV = nStripsV / 2;
    float pitchU = 0.075; //mm
    float pitchV = 0.240; //mm

    if (sensorID.getLayerNumber() == 3) {
      nStripsV = 768;
      pitchU = 0.050;
      pitchV = 0.160;
    }

    float edgeStripsU = m_edgeU / pitchU;
    float edgeStripsV = m_edgeV / pitchV;
    B2DEBUG(21, "good U in range " << edgeStripsU << ", " << nStripsU - edgeStripsU);
    B2DEBUG(21, "good V in range " << edgeStripsV << ", " << nStripsV - edgeStripsV);

    B2DEBUG(21, "U check: " << abs(centerROIU - centerSensorU) << " < (good) " << centerSensorU - edgeStripsU);
    B2DEBUG(21, "V check: " << abs(centerROIV - centerSensorV) << " < (good) " << centerSensorV - edgeStripsV);

    if ((abs(centerROIU - centerSensorU) > centerSensorU - edgeStripsU)
        || (abs(centerROIV - centerSensorV) > centerSensorV - edgeStripsV))
      continue;

    nGoodROIs++;
    m_h2GoodROIcenters->Fill(centerROIU, centerROIV);

    B2RESULT("");
    B2RESULT("GOOD ROI " << sensorID.getLayerNumber() << "." << sensorID.getLadderNumber() << "." << sensorID.getSensorNumber() <<
             ": U side " << m_ROIs[i]->getMinUid() << "->" << m_ROIs[i]->getMaxUid() << ", V side " << m_ROIs[i]->getMinVid() << "->" <<
             m_ROIs[i]->getMaxVid());

    m_h1GoodROItrack_pt->Fill(mom.Rho());
    m_h1GoodROItrack_phi->Fill(mom.Phi());
    m_h1GoodROItrack_cosTheta->Fill(cos(mom.Theta()));
    m_h1GoodROItrack_lambda->Fill(TMath::Pi() / 2 - mom.Theta());
    m_h1GoodROItrack_pVal->Fill(tfr->getPValue());

    for (int s = 0; s < m_shapers.getEntries(); s++) {
      if (m_ROIs[i]->Contains(*(m_shapers[s]))) {
        nOkROIs++;

        m_h2FullROIcenters->Fill(centerROIU, centerROIV);
        m_h1FullROItrack->Fill(1);
        m_h1FullROItrack_pt->Fill(mom.Rho());
        m_h1FullROItrack_phi->Fill(mom.Phi());
        m_h1FullROItrack_cosTheta->Fill(cos(mom.Theta()));
        m_h1FullROItrack_lambda->Fill(TMath::Pi() / 2 - mom.Theta());
        m_h1FullROItrack_pVal->Fill(tfr->getPValue());

        B2RESULT("  --> is Full");
        break;
      }

    }
  }

  m_nGoodROIs += nGoodROIs;
  m_h1goodROIs->Fill(nGoodROIs);
  m_nOkROIs += nOkROIs;
  m_h1okROIs->Fill(nOkROIs);

  m_h1totROIs->Fill(m_ROIs.getEntries());
  if (nGoodROIs > 0)
    m_h1effROIs->Fill((float) nOkROIs / nGoodROIs);
  n_rois += m_ROIs.getEntries();

  //RecoTrack general
  n_tracks += m_tracks.getEntries();

  //SVDIntercepts general
  n_intercepts += m_SVDIntercepts.getEntries();

  m_rootEvent++;

  if (nGoodROIs > 0)
    B2RESULT(" o  Good ROIs = " << nGoodROIs << ", of which Full = " << nOkROIs
             << " --> efficiency = " << (float)nOkROIs / nGoodROIs);

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



  if (m_rootFilePtr != nullptr) {
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

    m_roitracks->cd();
    m_h1ROItrack->Write();
    m_h1ROItrack_pt->Write();
    m_h1ROItrack_phi->Write();
    m_h1ROItrack_lambda->Write();
    m_h1ROItrack_cosTheta->Write();

    m_goodroitracks->cd();
    m_h1GoodROItrack->Write();
    m_h1GoodROItrack_pt->Write();
    m_h1GoodROItrack_phi->Write();
    m_h1GoodROItrack_lambda->Write();
    m_h1GoodROItrack_cosTheta->Write();

    m_fullroitracks->cd();
    m_h1FullROItrack->Write();
    m_h1FullROItrack_pt->Write();
    m_h1FullROItrack_phi->Write();
    m_h1FullROItrack_lambda->Write();
    m_h1FullROItrack_cosTheta->Write();

    m_ROIDir->cd();
    m_h1effROIs->Write();
    m_h1totROIs->Write();
    m_h1goodROIs->Write();
    m_h1okROIs->Write();
    m_h2ROIuMinMax->Write();
    m_h2ROIvMinMax->Write();
    m_h2ROIcenters->Write();
    m_h2GoodROIcenters->Write();
    m_h2FullROIcenters->Write();

    m_rootFilePtr->Close();

  }

}

