/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/pxdDataReduction/PXDROIFinderAnalysisModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/RelationArray.h>

#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/ROIid.h>
#include <tracking/dataobjects/PXDIntercept.h>
#include <list>
#include <iostream>
#include <TVector3.h>

//giulia
#include <vxd/geometry/GeoCache.h>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDROIFinderAnalysis)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDROIFinderAnalysisModule::PXDROIFinderAnalysisModule() : Module()
  , m_recoTrackListName()
  , m_PXDInterceptListName()
  , m_ROIListName()
  , m_rootFilePtr(NULL)
  , m_rootFileName("")
  , m_writeToRoot(false)
  , m_rootEvent(-1)
  //pxd sensors
  , m_nSensorsL1(0)
  , m_nSensorsL2(0)
  //efficiency graphs
  , m_gEff2(NULL)
  , m_gEff(NULL)
  , m_h1DigitsPerParticle(NULL)
  , m_h1RecoTracksPerParticle(NULL)
  //tracks with no digit in ROI
  , m_h1TrackOneDigiIn(NULL)
  , m_h1nnotINtrack2(NULL)
  , m_h1nnotINtrack3(NULL)
  , m_h1nnotINtrack4(NULL)
  , m_h1nnotINtrack5(NULL)
  //all tracks
  , m_h1Track(NULL)
  , m_h1Track_pt(NULL)
  , m_h1Track_phi(NULL)
  , m_h1Track_lambda(NULL)
  , m_h1Track_cosTheta(NULL)
  , m_h1Track_pVal(NULL)
  , m_h1Track_nSVDhits(NULL)
  , m_h1Track_nCDChits(NULL)
  //tracks with at least one digit in ROI
  , m_h1INtrack1(NULL)
  , m_h1INtrack1_pt(NULL)
  , m_h1INtrack1_phi(NULL)
  , m_h1INtrack1_lambda(NULL)
  , m_h1INtrack1_cosTheta(NULL)
  , m_h1INtrack1_pVal(NULL)
  , m_h1INtrack1_nSVDhits(NULL)
  , m_h1INtrack1_nCDChits(NULL)
  //tracks with no intercept
  , m_h1notINtrack5(NULL)
  , m_h1notINtrack5_pt(NULL)
  , m_h1notINtrack5_phi(NULL)
  , m_h1notINtrack5_lambda(NULL)
  , m_h1notINtrack5_cosTheta(NULL)
  , m_h1notINtrack5_pVal(NULL)
  , m_h1notINtrack5_nSVDhits(NULL)
  , m_h1notINtrack5_nCDChits(NULL)
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
  , m_h2Mapglob(NULL)
  , m_h2MaplocL1(NULL)
  , m_h2MaplocL2(NULL)
  //digits outside2 ROI
  , m_h2sigmaUphi_out2(NULL)
  , m_h2sigmaVphi_out2(NULL)
  , m_h1ResidU_out2(NULL)
  , m_h1ResidV_out2(NULL)
  , m_h1SigmaU_out2(NULL)
  , m_h1SigmaV_out2(NULL)
  , m_h1GlobalTime_out2(NULL)
  , m_h2Mapglob_out2(NULL)
  , m_h2MaplocL1_out2(NULL)
  , m_h2MaplocL2_out2(NULL)
  //digits outside3 ROI
  , m_h2sigmaUphi_out3(NULL)
  , m_h2sigmaVphi_out3(NULL)
  , m_h1ResidU_out3(NULL)
  , m_h1ResidV_out3(NULL)
  , m_h1SigmaU_out3(NULL)
  , m_h1SigmaV_out3(NULL)
  , m_h1GlobalTime_out3(NULL)
  , m_h2Mapglob_out3(NULL)
  , m_h2MaplocL1_out3(NULL)
  , m_h2MaplocL2_out3(NULL)
  //digits outside4 ROI
  , m_h2sigmaUphi_out4(NULL)
  , m_h2sigmaVphi_out4(NULL)
  , m_h1SigmaU_out4(NULL)
  , m_h1SigmaV_out4(NULL)
  , m_h1GlobalTime_out4(NULL)
  , m_h2Mapglob_out4(NULL)
  , m_h2MaplocL1_out4(NULL)
  , m_h2MaplocL2_out4(NULL)
  //digits outside5 ROI
  , m_h1GlobalTime_out5(NULL)
  , m_h2Mapglob_out5(NULL)
  , m_h2MaplocL1_out5(NULL)
  , m_h2MaplocL2_out5(NULL)

  //  ,m_h2_VXDhitsPR_xy(NULL)
  //  ,m_h2_VXDhitsPR_rz(NULL)

  //ROI stuff
  , m_h2ROIbottomLeft(NULL)
  , m_h2ROItopRight(NULL)
  , m_h2ROIuMinMax(NULL)
  , m_h2ROIvMinMax(NULL)
  , m_h1totROIs(NULL)
  , m_h1okROIs(NULL)
  , m_h1okROIfrac(NULL)
  , m_h1redFactor(NULL)
  , m_h1redFactor_L1(NULL)
  , m_h1redFactor_L2(NULL)

  , m_h1totArea(NULL)
  , m_h1okArea(NULL)

  , m_h1effPerTrack(NULL)

  //variables
  , m_globalTime(0.)
  , m_coorU(0.)
  , m_coorV(0.)
  , m_sigmaU(0.)
  , m_sigmaV(0.)
  , m_vxdID(-1)

  , m_coorUmc(0.)
  , m_coorVmc(0.)
  , m_Uidmc(-1)
  , m_Vidmc(-1)
  , m_vxdIDmc(-1)
  , m_pTmc(0.)
  , m_momXmc(0.)
  , m_momYmc(0.)
  , m_momZmc(0.)
  , m_thetamc(0.)
  , m_costhetamc(0.)
  , m_phimc(0.)
  , m_lambdamc(0.)

  , Ntrack(0)
  , NtrackHit(0)
  , n_notINtrack2(0)
  , n_notINtrack3(0)
  , n_notINtrack4(0)
  , n_notINtrack5(0)

  , n_rois(0)
  , n_intercepts(0)
  , n_tracks(0)
  , n_tracksWithDigits(0)
  , n_tracksWithDigitsInROI(0)


  , n_pxdDigit(0)
  , n_pxdDigitInROI(0)
  , n_notINdigit2(0)
  , n_notINdigit3(0)
  , n_notINdigit4(0)
  , n_notINdigit5(0)

  //vectors
  , npxdDigit{0}
  , npxdDigitInROI{0}
  , nnotINdigit2{0}
  , nnotINdigit3{0}
  , nnotINdigit4{0}
  , nnotINdigit5{0}
  , TrackOneDigiIn{0}
  , nnotINtrack2{0}
  , nnotINtrack3{0}
  , nnotINtrack4{0}
  , nnotINtrack5{0}

{
  //Set module properties
  setDescription("This module performs the analysis of the PXDROIFinder module output");

  addParam("writeToRoot", m_writeToRoot,
           "set true if you want to save the informations in a root file named by parameter 'rootFileName'", bool(true));

  addParam("rootFileName", m_rootFileName,
           "fileName used for . Will be ignored if parameter 'writeToRoot' is false (standard)",
           string("pxdDataRedAnalysis"));

  addParam("recoTrackListName", m_recoTrackListName,
           "name of the input collection of RecoTracks", std::string(""));

  addParam("PXDInterceptListName", m_PXDInterceptListName,
           "name of the list of interceptions", std::string(""));

  addParam("ROIListName", m_ROIListName,
           "name of the list of ROIs", std::string(""));

  m_rootEvent = 0;
}

PXDROIFinderAnalysisModule::~PXDROIFinderAnalysisModule()
{
}


void PXDROIFinderAnalysisModule::initialize()
{

  StoreArray<RecoTrack>::required(m_recoTrackListName);
  StoreArray<ROIid>::required(m_ROIListName);
  StoreArray<PXDIntercept>::required(m_PXDInterceptListName);

  StoreArray<MCParticle>::required();

  n_rois           = 0;
  n_intercepts     = 0;
  n_tracks         = 0;
  n_pxdDigit       = 0;
  n_pxdDigitInROI  = 0;

  n_tracksWithDigits = 0;
  n_tracksWithDigitsInROI = 0;
  NtrackHit = 0;
  n_notINtrack2 = 0;
  n_notINtrack3 = 0;
  n_notINtrack4 = 0;
  n_notINtrack5 = 0;

  for (int i = 0; i < 6; i++) {
    npxdDigit[i] = 0;
    npxdDigitInROI[i] = 0;
  }

  if (m_writeToRoot == true) {
    m_rootFileName += ".root";
    m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");
  } else
    m_rootFilePtr = NULL;


  m_h1GlobalTime = new TH1F("hGlobalTime", "global time for PXDDigits contained in ROI", 100, 0, 1);
  m_h1PullU = new TH1F("hPullU", "U pulls for PXDDigits contained in ROI", 100, -6, 6);
  m_h1PullV = new TH1F("hPullV", "V pulls for PXDDigits contained in ROI", 100, -6, 6);
  m_h2sigmaUphi = new TH2F("hsigmaUvsPhi", "sigmaU vs phi digits in ROI", 100, -180, 180, 100, 0, 0.35);
  m_h2sigmaVphi = new TH2F("hsigmaVvsPhi", "sigmaU vs phi digits in ROI", 100, -180, 180, 100, 0, 0.4);
  m_h1ResidU = new TH1F("hResidU", "U resid for PXDDigits contained in ROI", 100, -0.5, 0.5);
  m_h1ResidV = new TH1F("hResidV", "V resid for PXDDigits contained in ROI", 100, -0.5, 0.5);
  m_h2ResidUV = new TH2F("hResidUV", "U,V resid for PXDDigits contained in ROI", 100, -0.5, 0.5, 100, -0.5, 0.5);
  m_h1SigmaU = new TH1F("hSigmaU", "sigmaU for PXDDigits contained in ROI", 100, 0, 0.35);
  m_h1SigmaV = new TH1F("hSigmaV", "sigmaV for PXDDigits contained in ROI", 100, 0, 0.35);
  m_h2Mapglob = new TH2F("h2Mapglob", "global position, perp - phi", 500, 1.2, 2.6, 100, -TMath::Pi(), TMath::Pi());
  m_h2MaplocL1 = new TH2F("h2MaplocL1", "L1 local u v ID", 1535, -767.5, 767.5, 250, -0.5, 249.5);
  m_h2MaplocL2 = new TH2F("h2MaplocL2", "L2 local u v ID", 1535, -767.5, 767.5, 250, -0.5, 249.5);
  m_h2MaplocL1->GetXaxis()->SetTitle("v ID");
  m_h2MaplocL1->GetYaxis()->SetTitle("u ID");
  m_h2MaplocL2->GetXaxis()->SetTitle("v ID");
  m_h2MaplocL2->GetYaxis()->SetTitle("u ID");


  m_h1GlobalTime_out2 = new TH1F("hGlobalTime_out2", "global time for PXDDigits not contained in ROI", 100, 0, 1);
  m_h2sigmaUphi_out2 = new TH2F("hsigmaUvsPhi_out2", "sigmaU vs phi digits not contained in ROI", 100, -180, 180, 100, 0, 0.35);
  m_h2sigmaVphi_out2 = new TH2F("hsigmaVvsPhi_out2", "sigmaU vs phi digits not contained in ROI", 100, -180, 180, 100, 0, 0.4);
  m_h1ResidU_out2 = new TH1F("hResidU_out2", "U resid for PXDDigits not contained in ROI", 100, -2.5, 2.5);
  m_h1ResidV_out2 = new TH1F("hResidV_out2", "V resid for PXDDigits not contained in ROI", 100, -2.5, 2.5);
  m_h2ResidUV_out2 = new TH2F("hResidUV_out2", "U,V resid for PXDDigits contained in ROI", 100, -0.5, 0.5, 100, -0.5, 0.5);
  m_h1SigmaU_out2 = new TH1F("hSigmaU_out2", "sigmaU for PXDDigits not contained in ROI", 100, 0, 0.35);
  m_h1SigmaV_out2 = new TH1F("hSigmaV_out2", "sigmaV for PXDDigits not contained in ROI", 100, 0, 0.35);
  m_h2Mapglob_out2 = new TH2F("h2Mapglob_out2", "global position, perp - phi", 500, 1.2, 2.6, 100, -TMath::Pi(), TMath::Pi());
  m_h2MaplocL1_out2 = new TH2F("h2MaplocL1_out2", "L1 local u v ID", 1535, -767.5, 767.5, 250, -0.5, 249.5);
  m_h2MaplocL2_out2 = new TH2F("h2MaplocL2_out2", "L2 local u v ID", 1535, -767.5, 767.5, 250, -0.5, 249.5);


  m_h1GlobalTime_out3 = new TH1F("hGlobalTime_out3", "global time for PXDDigits not contained in ROI", 100, 0, 1);
  m_h2sigmaUphi_out3 = new TH2F("hsigmaUvsPhi_out3", "sigmaU vs phi digits not contained in ROI", 100, -180, 180, 100, 0, 0.35);
  m_h2sigmaVphi_out3 = new TH2F("hsigmaVvsPhi_out3", "sigmaU vs phi digits not contained in ROI", 100, -180, 180, 100, 0, 0.4);
  m_h1ResidU_out3 = new TH1F("hResidU_out3", "U resid for PXDDigits not contained in ROI", 100, -2.5, 2.5);
  m_h1ResidV_out3 = new TH1F("hResidV_out3", "V resid for PXDDigits not contained in ROI", 100, -2.5, 2.5);
  m_h2ResidUV_out3 = new TH2F("hResidUV_out3", "U,V resid for PXDDigits contained in ROI", 100, -0.5, 0.5, 100, -0.5, 0.5);
  m_h1SigmaU_out3 = new TH1F("hSigmaU_out3", "sigmaU for PXDDigits not contained in ROI", 100, 0, 0.35);
  m_h1SigmaV_out3 = new TH1F("hSigmaV_out3", "sigmaV for PXDDigits not contained in ROI", 100, 0, 0.35);

  m_h2Mapglob_out3 = new TH2F("h2Mapglob_out3", "global position, perp - phi", 500, 1.2, 2.6, 100, -TMath::Pi(), TMath::Pi());
  m_h2MaplocL1_out3 = new TH2F("h2MaplocL1_out3", "L1 local u v ID", 1535, -767.5, 767.5, 250, -0.5, 249.5);
  m_h2MaplocL2_out3 = new TH2F("h2MaplocL2_out3", "L2 local u v ID", 1535, -767.5, 767.5, 250, -0.5, 249.5);


  m_h1GlobalTime_out4 = new TH1F("hGlobalTime_out4", "global time for PXDDigits not contained in ROI", 100, 0, 1);
  m_h2sigmaUphi_out4 = new TH2F("hsigmaUvsPhi_out4", "sigmaU vs phi digits not contained in ROI", 100, -180, 180, 100, 0, 0.35);
  m_h2sigmaVphi_out4 = new TH2F("hsigmaVvsPhi_out4", "sigmaU vs phi digits not contained in ROI", 100, -180, 180, 100, 0, 0.4);
  m_h1SigmaU_out4 = new TH1F("hSigmaU_out4", "sigmaU for PXDDigits not contained in ROI", 100, 0, 0.35);
  m_h1SigmaV_out4 = new TH1F("hSigmaV_out4", "sigmaV for PXDDigits not contained in ROI", 100, 0, 0.35);

  m_h2Mapglob_out4 = new TH2F("h2Mapglob_out4", "global position, perp - phi", 500, 1.2, 2.6, 100, -TMath::Pi(), TMath::Pi());
  m_h2MaplocL1_out4 = new TH2F("h2MaplocL1_out4", "L1 local u v ID", 1535, -767.5, 767.5, 250, -0.5, 249.5);
  m_h2MaplocL2_out4 = new TH2F("h2MaplocL2_out4", "L2 local u v ID", 1535, -767.5, 767.5, 250, -0.5, 249.5);


  m_h1GlobalTime_out5 = new TH1F("hGlobalTime_out5", "global time for PXDDigits not contained in ROI", 100, 0, 1);

  m_h2Mapglob_out5 = new TH2F("h2Mapglob_out5", "global position, perp - phi", 500, 1.2, 2.6, 100, -TMath::Pi(), TMath::Pi());
  m_h2MaplocL1_out5 = new TH2F("h2MaplocL1_out5", "L1 local u v ID", 1535, -767.5, 767.5, 250, -0.5, 249.5);
  m_h2MaplocL2_out5 = new TH2F("h2MaplocL2_out5", "L2 local u v ID", 1535, -767.5, 767.5, 250, -0.5, 249.5);


  m_h1redFactor = new TH1F("hRedFactor", "L1+L2reduction factor", 1000, 0, 1);
  m_h1redFactor_L1 = new TH1F("hRedFactor_L1", "L1-only reduction factor", 1000, 0, 1);
  m_h1redFactor_L2 = new TH1F("hRedFactor_L2", "L2-only reduction factor", 1000, 0, 1);

  m_h1totROIs = new TH1F("h1TotNROIs", "number of all ROIs", 110, 0, 110);
  m_h1okROIs = new TH1F("h1OkNROIs", "number of all ROIs containing a PXDDigit", 110, 0, 110);
  m_h1okROIfrac = new TH1F("h1OkNROIfrac", "fraction of ROIs containing a PXDDigit", 100, 0, 1);

  m_h1totArea = new TH1F("h1TotArea", "Area of all ROIs", 100, 0, 2500000);
  m_h1okArea = new TH1F("h1OkArea", "Area of ROIs containing a PXDDigit", 100, 0, 75000);



  m_h1effPerTrack = new TH1F("heffPerTrack", "fraction of digits in ROI per track", 100, -0.02, 1.02);



  m_h2ROIbottomLeft = new TH2F("h2ROIbottomLeft", "u,v ID of the bottom left pixel", 650, -200, 450, 1300, -300, 1000);
  m_h2ROItopRight = new TH2F("h2ROItopRight", "u,v ID of the top right pixel", 650, -200, 450, 1300, -300, 1000);

  m_h2ROIuMinMax = new TH2F("h2ROIuMinMax", "u Min vs Max", 650, -200, 450, 650, -200, 450);
  m_h2ROIvMinMax = new TH2F("h2ROIvMinMax", "v Min vs Max", 1300, -300, 1000, 1300, -300, 1000);


  m_h1DigitsPerParticle = new TH1F("h1DigitsPerPart", "Number of PXDDigits per Particle", 50, 0, 50);
  m_h1RecoTracksPerParticle = new TH1F("h1RecoTracksPerPart", "Number of RecoTracks per Particle", 10, 0, 10);


  //analysis
  Double_t lowBin[6 + 1];
  for (int i = 0; i < 6; i++)
    lowBin[i] = pt[i] - ptErr[i];
  lowBin[6] = pt[5] + ptErr[5];

  m_h1TrackOneDigiIn = new TH1F("hTracksDigiIn", "Tracks with at least one digit contained in a ROI", 6, lowBin);
  m_h1nnotINtrack2 = new TH1F("h1outROITrack", "Tracks with ROI with correct VxdID but no digits inside ROI", 6, lowBin);
  m_h1nnotINtrack3 = new TH1F("h1noROITrack", "Tracks with ROI with wrong VxdID but no digits inside ROI", 6, lowBin);
  m_h1nnotINtrack4 = new TH1F("h1wrongVxdIDTrack", "Tracks with no ROI, Intercept with correct VxdID", 6, lowBin);
  m_h1nnotINtrack5 = new TH1F("h1noInterTrack", "Tracks with no Intercept matching a VxdID of digits", 6, lowBin);

  m_h1notINtrack5 = new TH1F("hNoInterTrack", "track with no intercepts", 20, 0, 20);
  m_h1notINtrack5_pt = new TH1F("hNoInterTrack_pT", "track with no intercepts", 100, 0, 6);
  m_h1notINtrack5_phi = new TH1F("h1NoInterTrack_phi", "hNoInterTrack_phi", 100, -180, 180);
  m_h1notINtrack5_lambda = new TH1F("h1NoInterTrack_lambda", "hNoInterTrack_lambda", 100, -180, 180);
  m_h1notINtrack5_cosTheta = new TH1F("h1NoInterTrack_cosTheta", "hNoInterTrack_cosTheta", 100, -1, 1);
  m_h1notINtrack5_pVal = new TH1F("hNoInterTrack_pVal", "track with no intercepts", 100, 0, 1);
  m_h1notINtrack5_nSVDhits = new TH1F("hNoInterTrack_nSVDhits", "track with no intercepts", 50, 0, 50);
  m_h1notINtrack5_nCDChits = new TH1F("hNoInterTrack_nCDChits", "track with no intercepts", 100, 0, 100);

  m_h1INtrack1 = new TH1F("hINTrack", "track with at least one digit inside ROI", 20, 0, 20);
  m_h1INtrack1_pt = new TH1F("hINTrack_pT", "track with at least one digit inside ROI", 100, 0, 6);
  m_h1INtrack1_phi = new TH1F("h1INTrack_phi", "hINTrack_phi", 100, -180, 180);
  m_h1INtrack1_lambda = new TH1F("h1INTrack_lambda", "hINTrack_lambda", 100, -180, 180);
  m_h1INtrack1_cosTheta = new TH1F("h1INTrack_cosTheta", "hINTrack_cosTheta", 100, -1, 1);
  m_h1INtrack1_pVal = new TH1F("h1INTrack_pVal", "track with no intercepts", 100, 0, 1);
  m_h1INtrack1_nSVDhits = new TH1F("h1INTrack_nSVDhits", "track with no intercepts", 50, 0, 50);
  m_h1INtrack1_nCDChits = new TH1F("h1INTrack_nCDChits", "track with no intercepts", 100, 0, 100);

  m_h1Track = new TH1F("hTrack", "all tracks", 20, 0, 20);
  m_h1Track_pt = new TH1F("hTrack_pT", "all tracks with digits", 100, 0, 6);
  m_h1Track_lambda = new TH1F("h1Track_lambda", "hTrack_lambda", 100, -180, 180);
  m_h1Track_phi = new TH1F("h1Track_phi", "hTrack_phi", 100, -180, 180);
  m_h1Track_cosTheta = new TH1F("h1Track_cosTheta", "hTrack_cos theta", 100, -1, 1);
  m_h1Track_pVal = new TH1F("h1Track_pVal", "track with no intercepts", 100, 0, 1);
  m_h1Track_nSVDhits = new TH1F("h1Track_nSVDhits", "track with no intercepts", 50, 0, 50);
  m_h1Track_nCDChits = new TH1F("h1Track_nCDChits", "track with no intercepts", 100, 0, 100);

  m_h1digiIn = new TH1F("hdigiIn", "digits inside ROI", 6, lowBin);
  m_h1digiOut2 = new TH1F("hdigiOut2", "ROI exists with with correct VxdID but no digits inside ROI", 6, lowBin);
  m_h1digiOut3 = new TH1F("hdigiOut3", "ROI exists with with wrong VxdID", 6, lowBin);
  m_h1digiOut4 = new TH1F("hdigiOut4", "ROI does not exist, but intercept has correct VxdID", 6, lowBin);
  m_h1digiOut5 = new TH1F("hdigiOut5", "no ROI, no Intercpets with correct VXDid", 6, lowBin);

  //  m_h2_VXDhitsPR_xy = new TH2F("hNoInteTrack_SVDhitsXY", "SVD Hits Missed by the VXDTF", 200, -15, 15, 200, -15, 15);

  //  m_h2_VXDhitsPR_rz = new TH2F("hNoInteTrack_SVDhitsRZ", "SVD Hits Missed by the VXDTF, r_{T} z", 200, -30, 40, 200, 0, 15);


}

void PXDROIFinderAnalysisModule::beginRun()
{
  m_rootEvent = 0;

  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();

  std::set<Belle2::VxdID> pxdLayers = aGeometry.getLayers(VXD::SensorInfoBase::PXD);
  std::set<Belle2::VxdID>::iterator itPxdLayers = pxdLayers.begin();

  while ((itPxdLayers != pxdLayers.end()) && (itPxdLayers->getLayerNumber() != 7)) {

    std::set<Belle2::VxdID> pxdLadders = aGeometry.getLadders(*itPxdLayers);
    std::set<Belle2::VxdID>::iterator itPxdLadders = pxdLadders.begin();

    while (itPxdLadders != pxdLadders.end()) {

      std::set<Belle2::VxdID> pxdSensors = aGeometry.getSensors(*itPxdLadders);
      std::set<Belle2::VxdID>::iterator itPxdSensors = pxdSensors.begin();

      while (itPxdSensors != pxdSensors.end()) {

        if (itPxdLadders->getLayerNumber() == 1)
          m_nSensorsL1++;
        if (itPxdLadders->getLayerNumber() == 2)
          m_nSensorsL2++;

        ++itPxdSensors;
      }
      ++itPxdLadders;
    }
    ++itPxdLayers;
  }


}


void PXDROIFinderAnalysisModule::event()
{

  typedef RelationIndex < RecoTrack, PXDIntercept>::range_from PXDInterceptsFromRecoTracks;
  typedef RelationIndex < RecoTrack, PXDIntercept>::iterator_from PXDInterceptIteratorType;
  typedef RelationIndex < PXDDigit, PXDTrueHit>::range_from PXDTrueHitFromPXDDigit;
  typedef RelationIndex < PXDDigit, PXDTrueHit>::iterator_from PXDTrueHitIteratorType;
  RelationIndex < PXDDigit, PXDTrueHit >
  relDigitTrueHit(DataStore::relationName(DataStore::arrayName<PXDDigit>(""),
                                          DataStore::arrayName<PXDTrueHit>("")));
  RelationIndex < RecoTrack, PXDIntercept >
  recoTrackToPXDIntercept(DataStore::relationName(m_recoTrackListName, m_PXDInterceptListName));

  double tmpGlobalTime;
  int tmpNGlobalTime;

  NtrackHit = 0;
  Ntrack = 0;

  B2DEBUG(1, "  ++++++++++++++ PXDROIFinderAnalysisModule");

  int nROIs = 0;
  int okArea_L1 = 0;
  int okArea_L2 = 0;
  int totArea_L1  = 0;
  int totArea_L2  = 0;

  //MCParticles
  StoreArray<MCParticle> mcParticles;


  //ROIs general
  StoreArray<ROIid> ROIList(m_ROIListName);
  for (int i = 0; i < (int)ROIList.getEntries(); i++) { //loop on ROIlist

    m_h2ROIbottomLeft->Fill(ROIList[i]->getMinUid(), ROIList[i]->getMinVid());
    m_h2ROItopRight->Fill(ROIList[i]->getMaxUid(), ROIList[i]->getMaxVid());
    m_h2ROIuMinMax->Fill(ROIList[i]->getMinUid(), ROIList[i]->getMaxUid());
    m_h2ROIvMinMax->Fill(ROIList[i]->getMinVid(), ROIList[i]->getMaxVid());
    int tmpArea = (ROIList[i]->getMaxUid() - ROIList[i]->getMinUid()) * (ROIList[i]->getMaxVid() - ROIList[i]->getMinVid());
    if ((ROIList[i]->getSensorID()).getLayerNumber() == 1)
      totArea_L1 += tmpArea;
    else
      totArea_L2 += tmpArea;

    bool isOK = false;

    for (int j = 0; j < (int)mcParticles.getEntries(); j++) {
      MCParticle* aMcParticle = mcParticles[j];

      // continue only if MCParticle has a related PXDDigit and RecoTrack
      RelationVector<PXDDigit> pxdDigits_MCParticle = aMcParticle->getRelationsFrom<PXDDigit>();

      if (!isOK)
        //loop on PXDDigits
        for (unsigned int iPXDDigit = 0; iPXDDigit < pxdDigits_MCParticle.size(); iPXDDigit++)
          if (ROIList[i]->Contains(*(pxdDigits_MCParticle[iPXDDigit]))) {
            nROIs++;
            isOK = true;
            break;
          }
    }
  }

  m_h1totArea->Fill(totArea_L1 + totArea_L2);
  double redFactor_L1 = totArea_L1 / 768. / 250. / m_nSensorsL1; //16
  double redFactor_L2 = totArea_L2 / 768. / 250. / m_nSensorsL2; //24
  m_h1redFactor->Fill((double)(totArea_L1 + totArea_L2) / 768. / 250. / (m_nSensorsL1 + m_nSensorsL2));
  m_h1redFactor_L1->Fill((double) redFactor_L1);
  m_h1redFactor_L2->Fill((double) redFactor_L2);

  m_h1totROIs->Fill(ROIList.getEntries());
  n_rois += ROIList.getEntries();

  //RecoTrack general
  StoreArray<RecoTrack> trackList(m_recoTrackListName);
  n_tracks += trackList.getEntries();

  //PXDIntercepts general
  StoreArray<PXDIntercept> PXDInterceptList(m_PXDInterceptListName);
  n_intercepts += PXDInterceptList.getEntries();

  Int_t n_NoInterceptTracks = 0;

  //  bool hasDigits = false;
  //  bool hasRecoTrack = false;

  //loop on MCParticles
  for (int j = 0; j < (int)mcParticles.getEntries(); j++) {

    MCParticle* aMcParticle = mcParticles[j];

    // continue only if MCParticle has a related PXDDigit and RecoTrack
    RelationVector<PXDDigit> pxdDigits_MCParticle = aMcParticle->getRelationsFrom<PXDDigit>();
    RelationVector<RecoTrack> recoTracks_MCParticle = aMcParticle->getRelationsWith<RecoTrack>();

    m_h1DigitsPerParticle->Fill(pxdDigits_MCParticle.size());
    if (pxdDigits_MCParticle.size() == 0)
      continue;

    //    hasDigits = true;

    m_h1RecoTracksPerParticle->Fill(recoTracks_MCParticle.size());
    if (recoTracks_MCParticle.size() == 0)
      continue;

    //    hasRecoTrack = true;

    Ntrack++;

    B2DEBUG(1, "Number of RecoTracks = " << recoTracks_MCParticle.size() << " and PXDDigits = " << pxdDigits_MCParticle.size() <<
            " related to this MCParticle");

    //retrieve general informations of MCParticle
    m_momXmc = (aMcParticle->getMomentum()).X();
    m_momYmc = (aMcParticle->getMomentum()).Y();
    m_momZmc = (aMcParticle->getMomentum()).Z();
    m_phimc = (aMcParticle->getMomentum()).Phi() * 180 / 3.1415;
    m_thetamc = (aMcParticle->getMomentum()).Theta() * 180 / 3.1415;
    m_costhetamc = (aMcParticle->getMomentum()).CosTheta();
    m_lambdamc = 90 - m_thetamc;
    m_pTmc = (aMcParticle->getMomentum()).Perp();

    //SVDhits
    RelationVector<SVDCluster> svdRelations = aMcParticle->getRelationsFrom<SVDCluster>();

    /*    Int_t nMCPartSVDhits = 0;
    Int_t nSVDhitLadder[4] = {0};
    if((int)svdRelations.size() > 0){
      nMCPartSVDhits = svdRelations.size();
      for(int s=0; s<(int)svdRelations.size();s++)
    nSVDhitLadder[ (svdRelations[s]->getSensorID()).getLayerNumber()-3 ]++;
    }
    */

    bool part_outsideROI = false;
    bool part_noROI = false;
    bool part_wrongVxdID = false;
    bool part_noInter = false;
    bool hasOneDigitInROI = false;

    double tmpArea = 0;

    Int_t nDigitsInRoiPerTrack = 0;
    Int_t nDigitsPerTrack = 0;

    //loop on PXDDigits
    for (unsigned int iPXDDigit = 0; iPXDDigit < pxdDigits_MCParticle.size(); iPXDDigit++) {

      bool hasIntercept = false;
      bool hasROI = false;
      bool interceptRightVxdID = false;
      bool MissingHit = true;

      n_pxdDigit ++ ;
      nDigitsPerTrack++;

      PXDTrueHitFromPXDDigit  PXDTrueHits = relDigitTrueHit.getElementsFrom(*pxdDigits_MCParticle[iPXDDigit]);
      PXDTrueHitIteratorType thePXDTrueHitIterator = PXDTrueHits.begin();
      PXDTrueHitIteratorType thePXDTrueHitIteratorEnd = PXDTrueHits.end();
      tmpGlobalTime = 0;
      tmpNGlobalTime = 0;

      for (; thePXDTrueHitIterator != thePXDTrueHitIteratorEnd; thePXDTrueHitIterator++) {
        tmpGlobalTime = tmpGlobalTime + thePXDTrueHitIterator->to->getGlobalTime();
        tmpNGlobalTime++;
      }
      m_globalTime = tmpGlobalTime / tmpNGlobalTime;


      m_Uidmc = pxdDigits_MCParticle[iPXDDigit]->getUCellID();
      m_Vidmc = pxdDigits_MCParticle[iPXDDigit]->getVCellID();
      m_vxdIDmc = pxdDigits_MCParticle[iPXDDigit]->getSensorID();

      int u2 = -999;
      int v2 = -999;
      if (VxdID(m_vxdIDmc).getSensorNumber() == 2) { //BKW
        u2 = m_Uidmc;
        v2 = -(767 - m_Vidmc);
      } else if (VxdID(m_vxdIDmc).getSensorNumber() == 1) { //FWD
        u2 = m_Uidmc;
        v2 = m_Vidmc;
      }

      VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
      const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(m_vxdIDmc);

      m_coorUmc = aSensorInfo.getUCellPosition(m_Uidmc);   //pxdDigits_MCParticle[iPXDDigit]->getUCellPosition();
      m_coorVmc = aSensorInfo.getVCellPosition(m_Vidmc);   //pxdDigits_MCParticle[iPXDDigit]->getVCellPosition();

      TVector3 local(m_coorUmc, m_coorVmc, 0);
      TVector3 globalSensorPos = aSensorInfo.pointToGlobal(local);


      if (m_pTmc > 1) npxdDigit[5]++;
      if (m_pTmc <= 1 && m_pTmc > 0.5) npxdDigit[4]++;
      if (m_pTmc <= 0.5 && m_pTmc > 0.3) npxdDigit[3]++;
      if (m_pTmc <= 0.3 && m_pTmc > 0.2) npxdDigit[2]++;
      if (m_pTmc <= 0.2 && m_pTmc > 0.1) npxdDigit[1]++;
      if (m_pTmc <= 0.1) npxdDigit[0]++;


      for (int i = 0; i < (int)recoTracks_MCParticle.size(); i++) { //loop on input RecoTracks

        PXDInterceptsFromRecoTracks  PXDIntercepts = recoTrackToPXDIntercept.getElementsFrom(recoTracks_MCParticle[i]);

        PXDInterceptIteratorType thePXDInterceptIterator = PXDIntercepts.begin();
        PXDInterceptIteratorType thePXDInterceptIteratorEnd = PXDIntercepts.end();


        for (; thePXDInterceptIterator != thePXDInterceptIteratorEnd; thePXDInterceptIterator++) {

          const PXDIntercept* theIntercept = thePXDInterceptIterator->to;

          if (theIntercept) {

            hasIntercept = true;

            m_coorU = theIntercept->getCoorU();
            m_coorV = theIntercept->getCoorV();
            m_sigmaU = theIntercept->getSigmaU();
            m_sigmaV = theIntercept->getSigmaV();
            m_vxdID = theIntercept->getSensorID();

            if (m_vxdID == m_vxdIDmc)
              interceptRightVxdID = true;
            else
              continue;

            const ROIid* theROIid = theIntercept->getRelatedTo<ROIid>(m_ROIListName);

            if (theROIid) {

              hasROI = true;

              tmpArea = (theROIid->getMaxUid() - theROIid->getMinUid()) * (theROIid->getMaxVid() - theROIid->getMinVid());

              if (theROIid->Contains(*(pxdDigits_MCParticle[iPXDDigit]))) { //CASO1

                if (MissingHit) {
                  nDigitsInRoiPerTrack++;

                  m_h1GlobalTime->Fill(m_globalTime);
                  m_h1PullU->Fill((m_coorU - m_coorUmc) / m_sigmaU);
                  m_h1PullV->Fill((m_coorV - m_coorVmc) / m_sigmaV);
                  m_h1ResidU->Fill(m_coorU - m_coorUmc);
                  m_h1ResidV->Fill(m_coorV - m_coorVmc);
                  m_h2ResidUV->Fill(m_coorU - m_coorUmc, m_coorV - m_coorVmc);
                  m_h2sigmaUphi->Fill(m_phimc, m_sigmaU);
                  m_h2sigmaVphi->Fill(m_phimc, m_sigmaV);
                  m_h1SigmaU->Fill(m_sigmaU);
                  m_h1SigmaV->Fill(m_sigmaV);
                  m_h2Mapglob->Fill(globalSensorPos.Perp() , globalSensorPos.Phi());


                  if (VxdID(m_vxdIDmc).getLayerNumber() == 1) //L1
                    m_h2MaplocL1->Fill(v2, u2);
                  if (VxdID(m_vxdIDmc).getLayerNumber() == 2) //L2
                    m_h2MaplocL2->Fill(v2, u2);


                  m_h1okArea->Fill(tmpArea);
                  if (VxdID(m_vxdIDmc).getLayerNumber() == 1) //L1
                    okArea_L1 = okArea_L1 + tmpArea;
                  if (VxdID(m_vxdIDmc).getLayerNumber() == 2) //L2
                    okArea_L2 = okArea_L2 + tmpArea;


                  hasOneDigitInROI = true;
                  n_pxdDigitInROI++;

                  if (m_pTmc > 1) npxdDigitInROI[5]++;
                  if (m_pTmc <= 1 && m_pTmc > 0.5) npxdDigitInROI[4]++;
                  if (m_pTmc <= 0.5 && m_pTmc > 0.3) npxdDigitInROI[3]++;
                  if (m_pTmc <= 0.3 && m_pTmc > 0.2) npxdDigitInROI[2]++;
                  if (m_pTmc <= 0.2 && m_pTmc > 0.1) npxdDigitInROI[1]++;
                  if (m_pTmc <= 0.1) npxdDigitInROI[0]++;

                  MissingHit = false;
                }


                break; // To avoid double counting (intercepts)
              } //if theROIid contains
            } //if (theROIid)
          } //if (theintercept)
        } //(end loop on intercept list)

        if (!MissingHit)
          break;// To avoid double counting (recoTracks)

      } //(end loop on recoTracks)


      if (MissingHit) {

        if (hasROI && hasIntercept && interceptRightVxdID) {
          part_outsideROI = true;

          n_notINdigit2 ++;

          m_h1GlobalTime_out2->Fill(m_globalTime);
          m_h1ResidU_out2->Fill(m_coorU - m_coorUmc);
          m_h1ResidV_out2->Fill(m_coorV - m_coorVmc);
          m_h2ResidUV_out2->Fill(m_coorU - m_coorUmc, m_coorV - m_coorVmc);
          m_h2sigmaUphi_out2->Fill(m_phimc, m_sigmaU);
          m_h2sigmaVphi_out2->Fill(m_phimc, m_sigmaV);
          m_h1SigmaU_out2->Fill(m_sigmaU);
          m_h1SigmaV_out2->Fill(m_sigmaV);
          m_h2Mapglob_out2->Fill(globalSensorPos.Perp() , globalSensorPos.Phi());
          if (VxdID(m_vxdIDmc).getLayerNumber() == 1) //L1
            m_h2MaplocL1_out2->Fill(v2, u2);
          if (VxdID(m_vxdIDmc).getLayerNumber() == 2) //L2
            m_h2MaplocL2_out2->Fill(v2, u2);

          if (m_pTmc > 1) nnotINdigit2[5]++;
          if (m_pTmc <= 1 && m_pTmc > 0.5) nnotINdigit2[4]++;
          if (m_pTmc <= 0.5 && m_pTmc > 0.3) nnotINdigit2[3]++;
          if (m_pTmc <= 0.3 && m_pTmc > 0.2) nnotINdigit2[2]++;
          if (m_pTmc <= 0.2 && m_pTmc > 0.1) nnotINdigit2[1]++;
          if (m_pTmc <= 0.1) nnotINdigit2[0]++;

        } else  if (!hasROI && hasIntercept && interceptRightVxdID) {
          part_noROI = true;

          n_notINdigit3 ++;

          m_h1GlobalTime_out3->Fill(m_globalTime);
          m_h1ResidU_out3->Fill(m_coorU - m_coorUmc);
          m_h1ResidV_out3->Fill(m_coorV - m_coorVmc);
          m_h2ResidUV_out3->Fill(m_coorU - m_coorUmc, m_coorV - m_coorVmc);
          m_h2sigmaUphi_out3->Fill(m_phimc, m_sigmaU);
          m_h2sigmaVphi_out3->Fill(m_phimc, m_sigmaV);
          m_h1SigmaU_out3->Fill(m_sigmaU);
          m_h1SigmaV_out3->Fill(m_sigmaV);
          m_h2Mapglob_out3->Fill(globalSensorPos.Perp() , globalSensorPos.Phi());
          if (VxdID(m_vxdIDmc).getLayerNumber() == 1) //L1
            m_h2MaplocL1_out3->Fill(v2, u2);

          if (VxdID(m_vxdIDmc).getLayerNumber() == 2) //L2
            m_h2MaplocL2_out3->Fill(v2, u2);


          if (m_pTmc > 1) nnotINdigit3[5]++;
          if (m_pTmc <= 1 && m_pTmc > 0.5) nnotINdigit3[4]++;
          if (m_pTmc <= 0.5 && m_pTmc > 0.3) nnotINdigit3[3]++;
          if (m_pTmc <= 0.3 && m_pTmc > 0.2) nnotINdigit3[2]++;
          if (m_pTmc <= 0.2 && m_pTmc > 0.1) nnotINdigit3[1]++;
          if (m_pTmc <= 0.1) nnotINdigit3[0]++;

        } else if (hasIntercept && !interceptRightVxdID) {
          part_wrongVxdID = true;

          n_notINdigit4 ++;

          m_h1GlobalTime_out4->Fill(m_globalTime);
          m_h2sigmaUphi_out4->Fill(m_phimc, m_sigmaU);
          m_h2sigmaVphi_out4->Fill(m_phimc, m_sigmaV);
          m_h1SigmaU_out4->Fill(m_sigmaU);
          m_h1SigmaV_out4->Fill(m_sigmaV);
          m_h2Mapglob_out4->Fill(globalSensorPos.Perp() , globalSensorPos.Phi());
          if (VxdID(m_vxdIDmc).getLayerNumber() == 1) //L1
            m_h2MaplocL1_out4->Fill(v2, u2);
          if (VxdID(m_vxdIDmc).getLayerNumber() == 2) //L2
            m_h2MaplocL2_out4->Fill(v2, u2);

          if (m_pTmc > 1) nnotINdigit4[5]++;
          if (m_pTmc <= 1 && m_pTmc > 0.5) nnotINdigit4[4]++;
          if (m_pTmc <= 0.5 && m_pTmc > 0.3) nnotINdigit4[3]++;
          if (m_pTmc <= 0.3 && m_pTmc > 0.2) nnotINdigit4[2]++;
          if (m_pTmc <= 0.2 && m_pTmc > 0.1) nnotINdigit4[1]++;
          if (m_pTmc <= 0.1) nnotINdigit4[0]++;

        } else if (!hasIntercept) {
          part_noInter = true;

          n_notINdigit5 ++;

          m_h1GlobalTime_out5->Fill(m_globalTime);
          m_h2Mapglob_out5->Fill(globalSensorPos.Perp() , globalSensorPos.Phi());
          if (VxdID(m_vxdIDmc).getLayerNumber() == 1) //L1
            m_h2MaplocL1_out5->Fill(v2, u2);
          if (VxdID(m_vxdIDmc).getLayerNumber() == 2) //L2
            m_h2MaplocL2_out5->Fill(v2, u2);

          if (m_pTmc > 1) nnotINdigit5[5]++;
          if (m_pTmc <= 1 && m_pTmc > 0.5) nnotINdigit5[4]++;
          if (m_pTmc <= 0.5 && m_pTmc > 0.3) nnotINdigit5[3]++;
          if (m_pTmc <= 0.3 && m_pTmc > 0.2) nnotINdigit5[2]++;
          if (m_pTmc <= 0.2 && m_pTmc > 0.1) nnotINdigit5[1]++;
          if (m_pTmc <= 0.1) nnotINdigit5[0]++;
        }
      }
    } //end loop on digits

    m_h1effPerTrack->Fill((float) nDigitsInRoiPerTrack / nDigitsPerTrack);
    m_h1Track_pt->Fill(m_pTmc);
    m_h1Track_phi->Fill(m_phimc);
    m_h1Track_lambda->Fill(m_lambdamc);
    m_h1Track_cosTheta->Fill(m_costhetamc);

    if (hasOneDigitInROI) {
      NtrackHit++;
      if (m_pTmc > 1) TrackOneDigiIn[5]++;
      if (m_pTmc <= 1 && m_pTmc > 0.5) TrackOneDigiIn[4]++;
      if (m_pTmc <= 0.5 && m_pTmc > 0.3) TrackOneDigiIn[3]++;
      if (m_pTmc <= 0.3 && m_pTmc > 0.2) TrackOneDigiIn[2]++;
      if (m_pTmc <= 0.2 && m_pTmc > 0.1) TrackOneDigiIn[1]++;
      if (m_pTmc <= 0.1) TrackOneDigiIn[0]++;

      m_h1INtrack1_pt->Fill(m_pTmc);
      m_h1INtrack1_phi->Fill(m_phimc);
      m_h1INtrack1_lambda->Fill(m_lambdamc);
      m_h1INtrack1_cosTheta->Fill(m_costhetamc);
    } else if (part_outsideROI)  { //CASO2
      n_notINtrack2++;
      if (m_pTmc > 1) nnotINtrack2[5]++;
      if (m_pTmc <= 1 && m_pTmc > 0.5) nnotINtrack2[4]++;
      if (m_pTmc <= 0.5 && m_pTmc > 0.3) nnotINtrack2[3]++;
      if (m_pTmc <= 0.3 && m_pTmc > 0.2) nnotINtrack2[2]++;
      if (m_pTmc <= 0.2 && m_pTmc > 0.1) nnotINtrack2[1]++;
      if (m_pTmc <= 0.1) nnotINtrack2[0]++;
    } else if (part_noROI) { //CASO3
      n_notINtrack3++;
      if (m_pTmc > 1) nnotINtrack3[5]++;
      if (m_pTmc <= 1 && m_pTmc > 0.5) nnotINtrack3[4]++;
      if (m_pTmc <= 0.5 && m_pTmc > 0.3) nnotINtrack3[3]++;
      if (m_pTmc <= 0.3 && m_pTmc > 0.2) nnotINtrack3[2]++;
      if (m_pTmc <= 0.2 && m_pTmc > 0.1) nnotINtrack3[1]++;
      if (m_pTmc <= 0.1) nnotINtrack3[0]++;
    } else if (part_wrongVxdID) { //CASO4
      n_notINtrack4++;
      if (m_pTmc > 1) nnotINtrack4[5]++;
      if (m_pTmc <= 1 && m_pTmc > 0.5) nnotINtrack4[4]++;
      if (m_pTmc <= 0.5 && m_pTmc > 0.3) nnotINtrack4[3]++;
      if (m_pTmc <= 0.3 && m_pTmc > 0.2) nnotINtrack4[2]++;
      if (m_pTmc <= 0.2 && m_pTmc > 0.1) nnotINtrack4[1]++;
      if (m_pTmc <= 0.1) nnotINtrack4[0]++;
    } else if (part_noInter) { //CASO5
      n_notINtrack5++;
      if (m_pTmc > 1) nnotINtrack5[5]++;
      if (m_pTmc <= 1 && m_pTmc > 0.5) nnotINtrack5[4]++;
      if (m_pTmc <= 0.5 && m_pTmc > 0.3) nnotINtrack5[3]++;
      if (m_pTmc <= 0.3 && m_pTmc > 0.2) nnotINtrack5[2]++;
      if (m_pTmc <= 0.2 && m_pTmc > 0.1) nnotINtrack5[1]++;
      if (m_pTmc <= 0.1) nnotINtrack5[0]++;

      n_NoInterceptTracks++;
      m_h1notINtrack5_phi->Fill(m_phimc);
      m_h1notINtrack5_lambda->Fill(m_lambdamc);
      m_h1notINtrack5_cosTheta->Fill(m_costhetamc);
      m_h1notINtrack5_pt->Fill(m_pTmc);
    }

  } // close loop on MCParticlet


  m_h1notINtrack5->Fill(n_NoInterceptTracks);
  m_h1Track->Fill(Ntrack);
  m_h1INtrack1->Fill(NtrackHit);

  n_tracksWithDigits += Ntrack;
  n_tracksWithDigitsInROI += NtrackHit;

  m_rootEvent++;
  B2RESULT(" o  PXDROIFinder ANALYSIS: tot ROIs = " << ROIList.getEntries() << ", ok ROIs = " << nROIs);
  B2RESULT(" o                           : NtrackHit/Ntrack = " << NtrackHit << "/ " << Ntrack << " = " <<
           (double)NtrackHit / Ntrack);
  if (nROIs > ROIList.getEntries()) B2RESULT(" HOUSTON WE HAVE A PROBLEM!");

  m_h1okROIs->Fill(nROIs);
  m_h1okROIfrac->Fill(1.*nROIs / ROIList.getEntries());
  cout << "" << endl;


}


void PXDROIFinderAnalysisModule::endRun()
{
}


void PXDROIFinderAnalysisModule::terminate()
{

  Double_t epsilon[6];
  Double_t epsilonErr[6];
  double epsilonTot = (double)n_pxdDigitInROI / (double) n_pxdDigit;
  Double_t epsilon2[6];
  Double_t epsilon2Err[6];
  double epsilon2Tot = (double)n_tracksWithDigitsInROI / (double) n_tracksWithDigits;

  for (int i = 0; i < 6; i++) {
    m_h1digiOut2->SetBinContent(i + 1, nnotINdigit2[i]);
    m_h1digiOut3->SetBinContent(i + 1, nnotINdigit3[i]);
    m_h1digiOut4->SetBinContent(i + 1, nnotINdigit4[i]);
    m_h1digiOut5->SetBinContent(i + 1, nnotINdigit5[i]);
    m_h1digiIn->SetBinContent(i + 1, npxdDigitInROI[i]);
  }

  for (int i = 0; i < 6; i++) {
    m_h1nnotINtrack2->SetBinContent(i + 1, nnotINtrack2[i]);
    m_h1nnotINtrack3->SetBinContent(i + 1, nnotINtrack3[i]);
    m_h1nnotINtrack4->SetBinContent(i + 1, nnotINtrack4[i]);
    m_h1nnotINtrack5->SetBinContent(i + 1, nnotINtrack5[i]);
    m_h1TrackOneDigiIn->SetBinContent(i + 1, TrackOneDigiIn[i]);
  }

  B2RESULT("     ROI Analysis Summary     ");
  B2RESULT("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  B2RESULT("");
  B2RESULT(" number of tracks = " << n_tracks);
  B2RESULT(" number of Intercepts = " << n_intercepts);
  B2RESULT(" number of ROIs = " << n_rois);
  B2RESULT("");
  B2RESULT("          average number of ROIs = " << m_h1totROIs->GetMean() << ", average area " << m_h1totArea->GetMean() <<
           " (not excluding overlaps!)");
  B2RESULT(" average number of ROIs w digits = " << m_h1okROIs->GetMean() << ", average tot area " << m_h1okArea->GetMean());
  B2RESULT("");
  B2RESULT(" red Factor = " << m_h1redFactor->GetMean()  << ", RMS = " << m_h1redFactor->GetRMS());
  B2RESULT("");
  B2RESULT("tracks w digits: " << n_tracksWithDigits);
  B2RESULT("tracks w digits in ROI: " << n_tracksWithDigitsInROI);
  B2RESULT("efficiency PTD : " << epsilon2Tot << " +/- " << sqrt(epsilon2Tot * (1 - epsilon2Tot) / n_tracksWithDigits));

  Int_t totTrackOneDigiIn = 0; //not used for the moment, added to double check
  Int_t totnnotINtrack2 = 0;
  Int_t totnnotINtrack3 = 0;
  Int_t totnnotINtrack4 = 0;
  Int_t totnnotINtrack5 = 0;

  Int_t totTrack[6];

  for (int j = 0; j < m_h1TrackOneDigiIn->GetNbinsX(); j++) {
    totTrackOneDigiIn = totTrackOneDigiIn + m_h1TrackOneDigiIn->GetBinContent(j + 1);
    totnnotINtrack2 = totnnotINtrack2 + m_h1nnotINtrack2->GetBinContent(j + 1);
    totnnotINtrack3 = totnnotINtrack3 + m_h1nnotINtrack3->GetBinContent(j + 1);
    totnnotINtrack4 = totnnotINtrack4 + m_h1nnotINtrack4->GetBinContent(j + 1);
    totnnotINtrack5 = totnnotINtrack5 + m_h1nnotINtrack5->GetBinContent(j + 1);

    totTrack[j] = m_h1nnotINtrack5->GetBinContent(j + 1) + m_h1nnotINtrack4->GetBinContent(j + 1) + m_h1nnotINtrack3->GetBinContent(
                    j + 1) + m_h1nnotINtrack2->GetBinContent(j + 1) + m_h1TrackOneDigiIn->GetBinContent(j + 1);
  }

  B2RESULT("      out ROI = " << totnnotINtrack2);
  B2RESULT("       no ROI = " << totnnotINtrack3);
  B2RESULT("  wrongVxdID  = " << totnnotINtrack4);
  B2RESULT("     no Inter = " << totnnotINtrack5);
  B2RESULT("");

  B2RESULT("    pxdDigit : " << n_pxdDigit);
  B2RESULT("  pxdDigitIn : " << n_pxdDigitInROI);

  B2RESULT("        eff DGT: " << epsilonTot << " +/- " << sqrt(epsilonTot * (1 - epsilonTot) / n_pxdDigit));
  B2RESULT("  inefficiency (PXDDigits): ");
  B2RESULT("         out ROI: " << n_notINdigit2);
  B2RESULT("          no ROI: " << n_notINdigit3);
  B2RESULT("      wrongVxdID: " << n_notINdigit4);
  B2RESULT("         noInter: " << n_notINdigit5);
  B2RESULT("");


  B2RESULT(" pT > 1 : " << pt[5]);
  B2RESULT("         out ROI: " << nnotINdigit2[5]);
  B2RESULT("          no ROI: " << nnotINdigit3[5]);
  B2RESULT("      wrongVxdID: " << nnotINdigit4[5]);
  B2RESULT("         noInter: " << nnotINdigit5[5]);
  B2RESULT("    pxdDigit : " << npxdDigit[5]);
  B2RESULT("  pxdDigitIn : " << npxdDigitInROI[5]);
  if ((npxdDigit[5] - npxdDigitInROI[5]) != (nnotINdigit2[5] + nnotINdigit3[5] + nnotINdigit4[5] + nnotINdigit5[5]))
    B2RESULT(" pxdDigitOut : " << npxdDigit[5] - npxdDigitInROI[5] << " != " << nnotINdigit2[5] + nnotINdigit3[5] + nnotINdigit4[5] +
             nnotINdigit5[5]);
  epsilon[5] = (double)npxdDigitInROI[5] / (double) npxdDigit[5];
  epsilonErr[5] = sqrt(epsilon[5] * (1 - epsilon[5]) / npxdDigit[5]);
  B2RESULT("  efficiency : " << epsilon[5] << " +/- " << epsilonErr[5]);
  epsilon2[5] = (double)TrackOneDigiIn[5] / (double) totTrack[5]  ;
  epsilon2Err[5] = sqrt(epsilon2[5] * (1 - epsilon2[5]) / totTrack[5]);
  B2RESULT("  efficiency2 : " << epsilon2[5] << " +/- " << epsilon2Err[5]);
  B2RESULT("");

  B2RESULT(" 0.5 < pT < 1 : " << pt[4]);
  B2RESULT("         out ROI: " << nnotINdigit2[4]);
  B2RESULT("          no ROI: " << nnotINdigit3[4]);
  B2RESULT("      wrongVxdID: " << nnotINdigit4[4]);
  B2RESULT("         noInter: " << nnotINdigit5[4]);
  B2RESULT("    pxdDigit : " << npxdDigit[4]);
  B2RESULT("  pxdDigitIn : " << npxdDigitInROI[4]);
  if ((npxdDigit[4] - npxdDigitInROI[4]) != (nnotINdigit2[4] + nnotINdigit3[4] + nnotINdigit4[4] + nnotINdigit5[4]))
    B2RESULT(" pxdDigitOut : " << npxdDigit[4] - npxdDigitInROI[4] << " != " << nnotINdigit2[4] + nnotINdigit3[4] + nnotINdigit4[4] +
             nnotINdigit5[4]);
  epsilon[4] = (double)npxdDigitInROI[4] / (double) npxdDigit[4];
  epsilonErr[4] = sqrt(epsilon[4] * (1 - epsilon[4]) / npxdDigit[4]);
  B2RESULT("  efficiency : " << epsilon[4] << " +/- " << epsilonErr[4]);
  epsilon2[4] = (double)TrackOneDigiIn[4] / (double) totTrack[4]  ;
  epsilon2Err[4] = sqrt(epsilon2[4] * (1 - epsilon2[4]) / totTrack[4]);
  B2RESULT("  efficiency2 : " << epsilon2[4] << " +/- " << epsilon2Err[4]);

  B2RESULT("");
  B2RESULT(" 0.3 < pT < 0.5 : " << pt[3]);
  B2RESULT("         out ROI: " << nnotINdigit2[3]);
  B2RESULT("          no ROI: " << nnotINdigit3[3]);
  B2RESULT("      wrongVxdID: " << nnotINdigit4[3]);
  B2RESULT("         noInter: " << nnotINdigit5[3]);
  B2RESULT("    pxdDigit : " << npxdDigit[3]);
  B2RESULT("  pxdDigitIn : " << npxdDigitInROI[3]);
  if ((npxdDigit[3] - npxdDigitInROI[3]) != (nnotINdigit2[3] + nnotINdigit3[3] + nnotINdigit4[3] + nnotINdigit5[3]))
    B2RESULT(" pxdDigitOut : " << npxdDigit[3] - npxdDigitInROI[3] << " != " << nnotINdigit2[3] + nnotINdigit3[3] + nnotINdigit4[3] +
             nnotINdigit5[3]);
  epsilon[3] = (double)npxdDigitInROI[3] / (double) npxdDigit[3];
  epsilonErr[3] = sqrt(epsilon[3] * (1 - epsilon[3]) / npxdDigit[3]);
  B2RESULT("  efficiency : " << epsilon[3] << " +/- " << epsilonErr[3]);
  epsilon2[3] = (double)TrackOneDigiIn[3] / (double) totTrack[3];
  epsilon2Err[3] = sqrt(epsilon2[3] * (1 - epsilon2[3]) / totTrack[3]);
  B2RESULT("  efficiency2 : " << epsilon2[3] << " +/- " << epsilon2Err[3]);

  B2RESULT("");
  B2RESULT(" 0.2 < pT < 0.3 : " << pt[2]);
  B2RESULT("         out ROI: " << nnotINdigit2[2]);
  B2RESULT("          no ROI: " << nnotINdigit3[2]);
  B2RESULT("      wrongVxdID: " << nnotINdigit4[2]);
  B2RESULT("         noInter: " << nnotINdigit5[2]);
  B2RESULT("    pxdDigit : " << npxdDigit[2]);
  B2RESULT("  pxdDigitIn : " << npxdDigitInROI[2]);
  if ((npxdDigit[2] - npxdDigitInROI[2]) != (nnotINdigit2[2] + nnotINdigit3[2] + nnotINdigit4[2] + nnotINdigit5[2]))
    B2RESULT(" pxdDigitOut : " << npxdDigit[2] - npxdDigitInROI[2] << " != " << nnotINdigit2[2] + nnotINdigit3[2] + nnotINdigit4[2] +
             nnotINdigit5[2]);
  epsilon[2] = (double)npxdDigitInROI[2] / (double) npxdDigit[2];
  epsilonErr[2] = sqrt(epsilon[2] * (1 - epsilon[2]) / npxdDigit[2]);
  B2RESULT("  efficiency : " << epsilon[2] << " +/- " << epsilonErr[2]);
  epsilon2[2] = (double)TrackOneDigiIn[2] / (double) totTrack[2]  ;
  epsilon2Err[2] = sqrt(epsilon2[2] * (1 - epsilon2[2]) / totTrack[2]);
  B2RESULT("  efficiency2 : " << epsilon2[2] << " +/- " << epsilon2Err[2]);

  B2RESULT("");
  B2RESULT(" 0.1 < pT < 0.2 : " << pt[1]);
  B2RESULT("         out ROI: " << nnotINdigit2[1]);
  B2RESULT("          no ROI: " << nnotINdigit3[1]);
  B2RESULT("      wrongVxdID: " << nnotINdigit4[1]);
  B2RESULT("         noInter: " << nnotINdigit5[1]);
  B2RESULT("    pxdDigit : " << npxdDigit[1]);
  B2RESULT("  pxdDigitIn : " << npxdDigitInROI[1]);
  if ((npxdDigit[1] - npxdDigitInROI[1]) != (nnotINdigit2[1] + nnotINdigit3[1] + nnotINdigit4[1] + nnotINdigit5[1]))
    B2RESULT(" pxdDigitOut : " << npxdDigit[1] - npxdDigitInROI[1] << " ?=? " << nnotINdigit2[1] + nnotINdigit3[1] + nnotINdigit4[1] +
             nnotINdigit5[1]);
  epsilon[1] = (double)npxdDigitInROI[1] / (double) npxdDigit[1];
  epsilonErr[1] = sqrt(epsilon[1] * (1 - epsilon[1]) / npxdDigit[1]);
  B2RESULT("  efficiency : " << epsilon[1] << " +/- " << epsilonErr[1]);
  epsilon2[1] = (double)TrackOneDigiIn[1] / (double) totTrack[1]  ;
  epsilon2Err[1] = sqrt(epsilon2[1] * (1 - epsilon2[1]) / totTrack[1]);
  B2RESULT("  efficiency2 : " << epsilon2[1] << " +/- " << epsilon2Err[1]);

  B2RESULT("");
  B2RESULT(" pT < 0.1 : " << pt[0]);
  B2RESULT("         out ROI: " << nnotINdigit2[0]);
  B2RESULT("          no ROI: " << nnotINdigit3[0]);
  B2RESULT("      wrongVxdID: " << nnotINdigit4[0]);
  B2RESULT("         noInter: " << nnotINdigit5[0]);
  B2RESULT("    pxdDigit : " << npxdDigit[0]);
  B2RESULT("  pxdDigitIn : " << npxdDigitInROI[0]);
  if ((npxdDigit[0] - npxdDigitInROI[0]) != (nnotINdigit2[0] + nnotINdigit3[0] + nnotINdigit4[0] + nnotINdigit5[0]))
    B2RESULT(" pxdDigitOut : " << npxdDigit[0] - npxdDigitInROI[0] << " ?=? " << nnotINdigit2[0] + nnotINdigit3[0] + nnotINdigit4[0] +
             nnotINdigit5[0]);
  epsilon[0] = (double)npxdDigitInROI[0] / (double) npxdDigit[0];
  epsilonErr[0] = sqrt(epsilon[0] * (1 - epsilon[0]) / npxdDigit[0]);
  B2RESULT("  efficiency : " << epsilon[0] << " +/- " << epsilonErr[0]);
  epsilon2[0] = (double)TrackOneDigiIn[0] / (double) totTrack[0]  ;
  epsilon2Err[0] = sqrt(epsilon2[0] * (1 - epsilon2[0]) / totTrack[0]);
  B2RESULT("  efficiency2 : " << epsilon2[0] << " +/- " << epsilon2Err[0]);

  B2RESULT("legend:");
  B2RESULT(" CASO2:  if (ROI exists but no PXDDigit inside)");
  B2RESULT(" CASO3:  if (ROI does not exist, intercept with correct VxdID)");
  B2RESULT(" CASO4:  if (intercept with wrong VxdID)");
  B2RESULT(" CASO5:  if (intercept does not exist)");


  m_gEff2 = new TGraphErrors(6, pt, epsilon2, ptErr, epsilon2Err);
  m_gEff2->SetName("g_eff2");
  m_gEff2->SetTitle("Normalized to MCParticles with digits and related track");
  m_gEff = new TGraphErrors(6, pt, epsilon, ptErr, epsilonErr);
  m_gEff->SetName("g_eff");
  m_gEff->SetTitle("Normalized to digits of MCParticles with digits and related track");



  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd(); //important! without this the famework root I/O (SimpleOutput etc) could mix with the root I/O of this module

    TDirectory* oldDir = gDirectory;
    TDirectory* m_digiDir = oldDir->mkdir("digits");
    TDirectory* m_tracksDir = oldDir->mkdir("tracks");
    TDirectory* m_notINtrack5 = oldDir->mkdir("notINtrack5");
    TDirectory* m_INtrack1 = oldDir->mkdir("INtrack1");
    TDirectory* m_alltracks = oldDir->mkdir("alltracks");
    TDirectory* m_in = oldDir->mkdir("digi_in");
    TDirectory* m_out2 = oldDir->mkdir("digi_out2");
    TDirectory* m_out3 = oldDir->mkdir("digi_out3");
    TDirectory* m_out4 = oldDir->mkdir("digi_out4");
    TDirectory* m_out5 = oldDir->mkdir("digi_out5");
    TDirectory* m_ROIDir = oldDir->mkdir("roi");

    m_h1DigitsPerParticle->Write();
    m_h1RecoTracksPerParticle->Write();

    m_gEff->Write();
    m_gEff2->Write();
    m_h1effPerTrack->Write();


    m_digiDir->cd();
    m_h1digiIn->Write();
    m_h1digiOut2->Write();
    m_h1digiOut3->Write();
    m_h1digiOut4->Write();
    m_h1digiOut5->Write();

    m_tracksDir->cd();
    m_h1TrackOneDigiIn->Write();
    m_h1nnotINtrack2->Write();
    m_h1nnotINtrack3->Write();
    m_h1nnotINtrack4->Write();
    m_h1nnotINtrack5->Write();

    m_notINtrack5->cd();
    m_h1notINtrack5->Write();
    m_h1notINtrack5_pt->Write();
    m_h1notINtrack5_phi->Write();
    m_h1notINtrack5_lambda->Write();
    m_h1notINtrack5_cosTheta->Write();

    m_INtrack1->cd();
    m_h1INtrack1->Write();
    m_h1INtrack1_pt->Write();
    m_h1INtrack1_phi->Write();
    m_h1INtrack1_lambda->Write();
    m_h1INtrack1_cosTheta->Write();

    m_alltracks->cd();
    m_h1Track->Write();
    m_h1Track_pt->Write();
    m_h1Track_phi->Write();
    m_h1Track_lambda->Write();
    m_h1Track_cosTheta->Write();

    m_in->cd();
    m_h1GlobalTime->Write();
    m_h1PullU->Write();
    m_h1PullV->Write();
    m_h1ResidU->Write();
    m_h1ResidV->Write();
    m_h2ResidUV->Write();
    m_h1SigmaU->Write();
    m_h1SigmaV->Write();
    m_h2sigmaUphi->Write();
    m_h2sigmaVphi->Write();
    m_h2Mapglob->Write();
    m_h2MaplocL1->Write();
    m_h2MaplocL2->Write();

    m_out2->cd();
    m_h1GlobalTime_out2->Write();
    m_h1ResidU_out2->Write();
    m_h1ResidV_out2->Write();
    m_h2ResidUV_out2->Write();
    m_h1SigmaU_out2->Write();
    m_h1SigmaV_out2->Write();
    m_h2sigmaUphi_out2->Write();
    m_h2sigmaVphi_out2->Write();
    m_h2Mapglob_out2->Write();
    m_h2MaplocL1_out2->Write();
    m_h2MaplocL2_out2->Write();

    m_out3->cd();
    m_h1GlobalTime_out3->Write();
    m_h1ResidU_out3->Write();
    m_h1ResidV_out3->Write();
    m_h2ResidUV_out3->Write();
    m_h1SigmaU_out3->Write();
    m_h1SigmaV_out3->Write();
    m_h2sigmaUphi_out3->Write();
    m_h2sigmaVphi_out3->Write();
    m_h2Mapglob_out3->Write();
    m_h2MaplocL1_out3->Write();
    m_h2MaplocL2_out3->Write();

    m_out4->cd();
    m_h1GlobalTime_out4->Write();
    m_h1SigmaU_out4->Write();
    m_h1SigmaV_out4->Write();
    m_h2sigmaUphi_out4->Write();
    m_h2sigmaVphi_out4->Write();
    m_h2Mapglob_out4->Write();
    m_h2MaplocL1_out4->Write();
    m_h2MaplocL2_out4->Write();

    m_out5->cd();
    m_h1GlobalTime_out5->Write();
    m_h2Mapglob_out5->Write();
    m_h2MaplocL1_out5->Write();
    m_h2MaplocL2_out5->Write();

    m_ROIDir->cd();
    m_h1totArea->Write();
    m_h1okArea->Write();
    m_h1okROIs->Write();
    m_h1okROIfrac->Write();
    m_h1totROIs->Write();
    m_h1redFactor->Write();
    m_h1redFactor_L1->Write();
    m_h1redFactor_L2->Write();

    //    m_h2_VXDhitsPR_xy->Write();
    //    m_h2_VXDhitsPR_rz->Write();


    m_h2ROIbottomLeft->Write();
    m_h2ROItopRight->Write();
    m_h2ROIuMinMax->Write();
    m_h2ROIvMinMax->Write();
    m_rootFilePtr->Close();

  }

}

