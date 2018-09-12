/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/svdROIFinder/SVDROIFinderAnalysisModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/RelationArray.h>

#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <list>
#include <iostream>
#include <TVector3.h>

#include <vxd/geometry/GeoCache.h>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDROIFinderAnalysis)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDROIFinderAnalysisModule::SVDROIFinderAnalysisModule() : Module()
  , m_recoTrackListName()
  , m_SVDInterceptListName()
  , m_ROIListName()
  , m_rootFilePtr(NULL)
  , m_rootFileName("")
  , m_writeToRoot(false)
  , m_rootEvent(-1)
  //svd sensors
  , m_nSensorsL3(0)
  , m_nSensorsL4(0)
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
  //digits outside2 ROI
  , m_h2sigmaUphi_out2(NULL)
  , m_h2sigmaVphi_out2(NULL)
  , m_h1ResidU_out2(NULL)
  , m_h1ResidV_out2(NULL)
  , m_h1SigmaU_out2(NULL)
  , m_h1SigmaV_out2(NULL)
  , m_h1GlobalTime_out2(NULL)
  //digits outside3 ROI
  , m_h2sigmaUphi_out3(NULL)
  , m_h2sigmaVphi_out3(NULL)
  , m_h1ResidU_out3(NULL)
  , m_h1ResidV_out3(NULL)
  , m_h1SigmaU_out3(NULL)
  , m_h1SigmaV_out3(NULL)
  , m_h1GlobalTime_out3(NULL)
  //digits outside4 ROI
  , m_h2sigmaUphi_out4(NULL)
  , m_h2sigmaVphi_out4(NULL)
  , m_h1SigmaU_out4(NULL)
  , m_h1SigmaV_out4(NULL)
  , m_h1GlobalTime_out4(NULL)
  //digits outside5 ROI
  , m_h1GlobalTime_out5(NULL)

  //ROI stuff
  , m_h2ROIbottomLeft(NULL)
  , m_h2ROItopRight(NULL)
  , m_h2ROIuMinMax(NULL)
  , m_h2ROIvMinMax(NULL)
  , m_h1totROIs(NULL)
  , m_h1okROIs(NULL)
  , m_h1totUstrips(NULL)
  , m_h1totVstrips(NULL)

  , m_h1effPerTrack(NULL)

  //variables
  , m_globalTime(0.)
  , m_coorU(0.)
  , m_coorV(0.)
  , m_sigmaU(0.)
  , m_sigmaV(0.)
  , m_vxdID(-1)

  , m_coormc(0.)
  , m_idmc(-1)
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
  , n_OKrois(0)
  , n_intercepts(0)
  , n_tracks(0)
  , n_tracksWithDigits(0)
  , n_tracksWithDigitsInROI(0)


  , n_svdDigit(0)
  , n_svdDigitInROI(0)
  , n_notINdigit2(0)
  , n_notINdigit3(0)
  , n_notINdigit4(0)
  , n_notINdigit5(0)

  //vectors
  , nsvdDigit{0}
  , nsvdDigitInROI{0}
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
  setDescription("This module performs the analysis of the SVDROIFinder module output");

  addParam("isSimulation", m_isSimulation,
           "set true if you want to evaluate efficiency on simulation", bool(true));
  addParam("writeToRoot", m_writeToRoot,
           "set true if you want to save the informations in a root file named by parameter 'rootFileName'", bool(true));

  addParam("rootFileName", m_rootFileName,
           "fileName used for . Will be ignored if parameter 'writeToRoot' is false (standard)",
           string("svdDataRedAnalysis"));

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

SVDROIFinderAnalysisModule::~SVDROIFinderAnalysisModule()
{
}


void SVDROIFinderAnalysisModule::initialize()
{

  m_shapers.isRequired(m_shapersName);
  m_trackList.isRequired(m_recoTrackListName);
  m_ROIs.isRequired(m_ROIListName);
  m_SVDIntercepts.isRequired(m_SVDInterceptListName);
  m_mcParticles.isRequired();

  n_rois           = 0;
  n_OKrois           = 0; //simulation
  m_nGoodROIs = 0; //data
  n_intercepts     = 0;
  n_tracks         = 0;
  n_svdDigit       = 0;
  n_svdDigitInROI  = 0;

  n_tracksWithDigits = 0;
  n_tracksWithDigitsInROI = 0;
  NtrackHit = 0;
  n_notINtrack2 = 0;
  n_notINtrack3 = 0;
  n_notINtrack4 = 0;
  n_notINtrack5 = 0;

  for (int i = 0; i < 6; i++) {
    nsvdDigit[i] = 0;
    nsvdDigitInROI[i] = 0;
  }

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


  m_h1GlobalTime_out2 = new TH1F("hGlobalTime_out2", "global time for SVDShaperDigits not contained in ROI", 200, -100, 100);
  m_h2sigmaUphi_out2 = new TH2F("hsigmaUvsPhi_out2", "sigmaU vs phi digits not contained in ROI", 100, -180, 180, 100, 0, 0.35);
  m_h2sigmaVphi_out2 = new TH2F("hsigmaVvsPhi_out2", "sigmaU vs phi digits not contained in ROI", 100, -180, 180, 100, 0, 0.4);
  m_h1ResidU_out2 = new TH1F("hResidU_out2", "U resid for SVDShaperDigits not contained in ROI", 100, -2.5, 2.5);
  m_h1ResidV_out2 = new TH1F("hResidV_out2", "V resid for SVDShaperDigits not contained in ROI", 100, -2.5, 2.5);
  m_h1SigmaU_out2 = new TH1F("hSigmaU_out2", "sigmaU for SVDShaperDigits not contained in ROI", 100, 0, 0.35);
  m_h1SigmaV_out2 = new TH1F("hSigmaV_out2", "sigmaV for SVDShaperDigits not contained in ROI", 100, 0, 0.35);

  m_h1GlobalTime_out3 = new TH1F("hGlobalTime_out3", "global time for SVDShaperDigits not contained in ROI", 200, -100, 100);
  m_h2sigmaUphi_out3 = new TH2F("hsigmaUvsPhi_out3", "sigmaU vs phi digits not contained in ROI", 100, -180, 180, 100, 0, 0.35);
  m_h2sigmaVphi_out3 = new TH2F("hsigmaVvsPhi_out3", "sigmaU vs phi digits not contained in ROI", 100, -180, 180, 100, 0, 0.4);
  m_h1ResidU_out3 = new TH1F("hResidU_out3", "U resid for SVDShaperDigits not contained in ROI", 100, -2.5, 2.5);
  m_h1ResidV_out3 = new TH1F("hResidV_out3", "V resid for SVDShaperDigits not contained in ROI", 100, -2.5, 2.5);
  m_h1SigmaU_out3 = new TH1F("hSigmaU_out3", "sigmaU for SVDShaperDigits not contained in ROI", 100, 0, 0.35);
  m_h1SigmaV_out3 = new TH1F("hSigmaV_out3", "sigmaV for SVDShaperDigits not contained in ROI", 100, 0, 0.35);


  m_h1GlobalTime_out4 = new TH1F("hGlobalTime_out4", "global time for SVDShaperDigits not contained in ROI", 200, -100, 100);
  m_h2sigmaUphi_out4 = new TH2F("hsigmaUvsPhi_out4", "sigmaU vs phi digits not contained in ROI", 100, -180, 180, 100, 0, 0.35);
  m_h2sigmaVphi_out4 = new TH2F("hsigmaVvsPhi_out4", "sigmaU vs phi digits not contained in ROI", 100, -180, 180, 100, 0, 0.4);
  m_h1SigmaU_out4 = new TH1F("hSigmaU_out4", "sigmaU for SVDShaperDigits not contained in ROI", 100, 0, 0.35);
  m_h1SigmaV_out4 = new TH1F("hSigmaV_out4", "sigmaV for SVDShaperDigits not contained in ROI", 100, 0, 0.35);


  m_h1GlobalTime_out5 = new TH1F("hGlobalTime_out5", "global time for SVDShaperDigits not contained in ROI", 200, -100, 100);



  m_h1totROIs = new TH1F("h1TotNROIs", "number of all ROIs", 110, 0, 110);
  m_h1okROIs = new TH1F("h1OkNROIs", "number of all ROIs containing a SVDShaperDigit", 110, 0, 110);

  m_h1totUstrips = new TH1F("h1TotUstrips", "number of U strips in ROIs", 100, 0, 250000);
  m_h1totVstrips = new TH1F("h1TotVstrips", "number of V strips in ROIs", 100, 0, 250000);

  m_h1effPerTrack = new TH1F("heffPerTrack", "fraction of digits in ROI per track", 100, -0.02, 1.02);



  m_h2ROIbottomLeft = new TH2F("h2ROIbottomLeft", "u,v ID of the bottom left pixel", 650, -200, 450, 1300, -300, 1000);
  m_h2ROItopRight = new TH2F("h2ROItopRight", "u,v ID of the top right pixel", 650, -200, 450, 1300, -300, 1000);

  m_h2ROIuMinMax = new TH2F("h2ROIuMinMax", "u Min vs Max", 650, -200, 450, 650, -200, 450);
  m_h2ROIvMinMax = new TH2F("h2ROIvMinMax", "v Min vs Max", 1300, -300, 1000, 1300, -300, 1000);


  m_h1DigitsPerParticle = new TH1F("h1DigitsPerPart", "Number of SVDShaperDigits per Particle", 50, 0, 50);
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

void SVDROIFinderAnalysisModule::beginRun()
{
  m_rootEvent = 0;

  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();

  std::set<Belle2::VxdID> svdLayers = aGeometry.getLayers(VXD::SensorInfoBase::SVD);
  std::set<Belle2::VxdID>::iterator itSvdLayers = svdLayers.begin();

  while ((itSvdLayers != svdLayers.end()) && (itSvdLayers->getLayerNumber() != 7)) {

    std::set<Belle2::VxdID> svdLadders = aGeometry.getLadders(*itSvdLayers);
    std::set<Belle2::VxdID>::iterator itSvdLadders = svdLadders.begin();

    while (itSvdLadders != svdLadders.end()) {

      std::set<Belle2::VxdID> svdSensors = aGeometry.getSensors(*itSvdLadders);
      std::set<Belle2::VxdID>::iterator itSvdSensors = svdSensors.begin();

      while (itSvdSensors != svdSensors.end()) {

        if (itSvdLadders->getLayerNumber() == 3)
          m_nSensorsL3++;
        if (itSvdLadders->getLayerNumber() == 4)
          m_nSensorsL4++;

        ++itSvdSensors;
      }
      ++itSvdLadders;
    }
    ++itSvdLayers;
  }


}


void SVDROIFinderAnalysisModule::event()
{

  typedef RelationIndex < RecoTrack, SVDIntercept>::range_from SVDInterceptsFromRecoTracks;
  typedef RelationIndex < RecoTrack, SVDIntercept>::iterator_from SVDInterceptIteratorType;
  typedef RelationIndex < SVDShaperDigit, SVDTrueHit>::range_from SVDTrueHitFromSVDShaperDigit;
  typedef RelationIndex < SVDShaperDigit, SVDTrueHit>::iterator_from SVDTrueHitIteratorType;
  RelationIndex < SVDShaperDigit, SVDTrueHit >
  relDigitTrueHit(DataStore::relationName(DataStore::arrayName<SVDShaperDigit>(""),
                                          DataStore::arrayName<SVDTrueHit>("")));
  RelationIndex < RecoTrack, SVDIntercept >
  recoTrackToSVDIntercept(DataStore::relationName(m_recoTrackListName, m_SVDInterceptListName));

  double tmpGlobalTime;
  int tmpNGlobalTime;

  NtrackHit = 0;
  Ntrack = 0;

  B2DEBUG(1, "  ++++++++++++++ SVDROIFinderAnalysisModule");

  int nROIs = 0;



  //ROIs general
  for (int i = 0; i < (int)m_ROIs.getEntries(); i++) { //loop on ROIlist

    m_h2ROIbottomLeft->Fill(m_ROIs[i]->getMinUid(), m_ROIs[i]->getMinVid());
    m_h2ROItopRight->Fill(m_ROIs[i]->getMaxUid(), m_ROIs[i]->getMaxVid());
    m_h2ROIuMinMax->Fill(m_ROIs[i]->getMinUid(), m_ROIs[i]->getMaxUid());
    m_h2ROIvMinMax->Fill(m_ROIs[i]->getMinVid(), m_ROIs[i]->getMaxVid());

    for (int s = 0; s < m_shapers.getEntries(); s++) {
      if (m_ROIs[i]->Contains(*(m_shapers[s]))) {
        m_nGoodROIs++;
        break;
      }

    }


    bool isOK = false;

    for (int j = 0; j < (int)m_mcParticles.getEntries(); j++) {
      MCParticle* aMcParticle = m_mcParticles[j];

      // continue only if MCParticle has a related SVDShaperDigit and RecoTrack
      RelationVector<SVDShaperDigit> svdDigits_MCParticle = aMcParticle->getRelationsFrom<SVDShaperDigit>();

      if (!isOK)
        //loop on SVDShaperDigits
        for (unsigned int iSVDShaperDigit = 0; iSVDShaperDigit < svdDigits_MCParticle.size(); iSVDShaperDigit++)
          if (m_ROIs[i]->Contains(*(svdDigits_MCParticle[iSVDShaperDigit]))) {
            nROIs++;
            isOK = true;
            break;
          }
    }
  }

  m_h1totROIs->Fill(m_ROIs.getEntries());
  n_rois += m_ROIs.getEntries();

  //RecoTrack general
  n_tracks += m_trackList.getEntries();

  //SVDIntercepts general
  n_intercepts += m_SVDIntercepts.getEntries();

  Int_t n_NoInterceptTracks = 0;

  //loop on MCParticles
  for (int j = 0; j < (int)m_mcParticles.getEntries(); j++) {

    MCParticle* aMcParticle = m_mcParticles[j];

    // continue only if MCParticle has a related SVDShaperDigit and RecoTrack
    RelationVector<SVDShaperDigit> svdDigits_MCParticle = aMcParticle->getRelationsFrom<SVDShaperDigit>();
    RelationVector<RecoTrack> recoTracks_MCParticle = aMcParticle->getRelationsWith<RecoTrack>();

    m_h1DigitsPerParticle->Fill(svdDigits_MCParticle.size());
    if (svdDigits_MCParticle.size() == 0)
      continue;

    //    m_h1RecoTracksPerParticle->Fill(recoTracks_MCParticle.size());
    //    if (recoTracks_MCParticle.size() == 0)
    //      continue;

    Ntrack++;

    B2DEBUG(1, "Number of RecoTracks = " << recoTracks_MCParticle.size() << " and SVDShaperDigits = " << svdDigits_MCParticle.size() <<
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


    bool part_outsideROI = false;
    bool part_noROI = false;
    bool part_wrongVxdID = false;
    bool part_noInter = false;
    bool hasOneDigitInROI = false;

    Int_t nDigitsInRoiPerTrack = 0;
    Int_t nDigitsPerTrack = 0;

    //loop on SVDShaperDigits
    for (unsigned int iSVDShaperDigit = 0; iSVDShaperDigit < svdDigits_MCParticle.size(); iSVDShaperDigit++) {

      bool isU = svdDigits_MCParticle[iSVDShaperDigit]->isUStrip();

      bool hasIntercept = false;
      bool hasROI = false;
      bool interceptRightVxdID = false;
      bool MissingHit = true;

      n_svdDigit ++ ;
      nDigitsPerTrack++;

      SVDTrueHitFromSVDShaperDigit  SVDTrueHits = relDigitTrueHit.getElementsFrom(*svdDigits_MCParticle[iSVDShaperDigit]);
      SVDTrueHitIteratorType theSVDTrueHitIterator = SVDTrueHits.begin();
      SVDTrueHitIteratorType theSVDTrueHitIteratorEnd = SVDTrueHits.end();
      tmpGlobalTime = 0;
      tmpNGlobalTime = 0;

      for (; theSVDTrueHitIterator != theSVDTrueHitIteratorEnd; theSVDTrueHitIterator++) {
        tmpGlobalTime = tmpGlobalTime + theSVDTrueHitIterator->to->getGlobalTime();
        tmpNGlobalTime++;
      }

      m_globalTime = tmpGlobalTime / tmpNGlobalTime;
      m_idmc = svdDigits_MCParticle[iSVDShaperDigit]->getCellID();
      m_vxdIDmc = svdDigits_MCParticle[iSVDShaperDigit]->getSensorID();

      VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
      const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(m_vxdIDmc);

      if (isU)
        m_coormc = aSensorInfo.getUCellPosition(m_idmc);
      else
        m_coormc = aSensorInfo.getVCellPosition(m_idmc);

      if (m_pTmc > 1) nsvdDigit[5]++;
      if (m_pTmc <= 1 && m_pTmc > 0.5) nsvdDigit[4]++;
      if (m_pTmc <= 0.5 && m_pTmc > 0.3) nsvdDigit[3]++;
      if (m_pTmc <= 0.3 && m_pTmc > 0.2) nsvdDigit[2]++;
      if (m_pTmc <= 0.2 && m_pTmc > 0.1) nsvdDigit[1]++;
      if (m_pTmc <= 0.1) nsvdDigit[0]++;


      //      for (int i = 0; i < (int)recoTracks_MCParticle.size(); i++) { //loop on related RecoTracks
      for (int i = 0; i < (int)m_trackList.getEntries(); i++) { //loop on all RecoTracks

        //        SVDInterceptsFromRecoTracks  SVDIntercepts = recoTrackToSVDIntercept.getElementsFrom(recoTracks_MCParticle[i]);
        SVDInterceptsFromRecoTracks  SVDIntercepts = recoTrackToSVDIntercept.getElementsFrom(m_trackList[i]);

        SVDInterceptIteratorType theSVDInterceptIterator = SVDIntercepts.begin();
        SVDInterceptIteratorType theSVDInterceptIteratorEnd = SVDIntercepts.end();


        for (; theSVDInterceptIterator != theSVDInterceptIteratorEnd; theSVDInterceptIterator++) {

          const SVDIntercept* theIntercept = theSVDInterceptIterator->to;

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

              if (theROIid->Contains(*(svdDigits_MCParticle[iSVDShaperDigit]))) { //CASO1

                if (MissingHit) {
                  nDigitsInRoiPerTrack++;

                  m_h1GlobalTime->Fill(m_globalTime);

                  if (isU) {
                    m_h1PullU->Fill((m_coorU - m_coormc) / m_sigmaU);
                    m_h1ResidU->Fill(m_coorU - m_coormc);
                    m_h2sigmaUphi->Fill(m_phimc, m_sigmaU);
                    m_h1SigmaU->Fill(m_sigmaU);
                  } else {
                    m_h1PullV->Fill((m_coorV - m_coormc) / m_sigmaV);
                    m_h1ResidV->Fill(m_coorV - m_coormc);
                    m_h2sigmaVphi->Fill(m_phimc, m_sigmaV);
                    m_h1SigmaV->Fill(m_sigmaV);
                  }

                  hasOneDigitInROI = true;
                  n_svdDigitInROI++;

                  if (m_pTmc > 1) nsvdDigitInROI[5]++;
                  if (m_pTmc <= 1 && m_pTmc > 0.5) nsvdDigitInROI[4]++;
                  if (m_pTmc <= 0.5 && m_pTmc > 0.3) nsvdDigitInROI[3]++;
                  if (m_pTmc <= 0.3 && m_pTmc > 0.2) nsvdDigitInROI[2]++;
                  if (m_pTmc <= 0.2 && m_pTmc > 0.1) nsvdDigitInROI[1]++;
                  if (m_pTmc <= 0.1) nsvdDigitInROI[0]++;

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
          if (isU) {
            m_h1ResidU_out2->Fill(m_coorU - m_coormc);
            m_h2sigmaUphi_out2->Fill(m_phimc, m_sigmaU);
            m_h1SigmaU_out2->Fill(m_sigmaU);
          } else {
            m_h1ResidV_out2->Fill(m_coorV - m_coormc);
            m_h2sigmaVphi_out2->Fill(m_phimc, m_sigmaV);
            m_h1SigmaV_out2->Fill(m_sigmaV);
          }

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
          if (isU) {
            m_h1ResidU_out3->Fill(m_coorU - m_coormc);
            m_h2sigmaUphi_out3->Fill(m_phimc, m_sigmaU);
            m_h1SigmaU_out3->Fill(m_sigmaU);
          } else {
            m_h1ResidV_out3->Fill(m_coorV - m_coormc);
            m_h2sigmaVphi_out3->Fill(m_phimc, m_sigmaV);
            m_h1SigmaV_out3->Fill(m_sigmaV);
          }

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
          if (isU) {
            m_h2sigmaUphi_out4->Fill(m_phimc, m_sigmaU);
            m_h1SigmaU_out4->Fill(m_sigmaU);
          } else {
            m_h2sigmaVphi_out4->Fill(m_phimc, m_sigmaV);
            m_h1SigmaV_out4->Fill(m_sigmaV);
          }

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
  if ((m_ROIs.getEntries() > 0) && m_isSimulation) {
    B2RESULT(" o  SVDROIFinder ANALYSIS: tot ROIs = " << m_ROIs.getEntries() << ", ok ROIs = " << nROIs);
    B2RESULT(" o                           : NtrackHit/Ntrack = " << NtrackHit << "/ " << Ntrack << " = " <<
             (double)NtrackHit / Ntrack);
    for (int i = 0; i < m_ROIs.getEntries(); i++) {
      VxdID sensor = m_ROIs[i]->getSensorID();
      B2RESULT(i << " ROI " << sensor.getLadderNumber() << "." << sensor.getLayerNumber() << "." << sensor.getSensorNumber() <<
               ": " << m_ROIs[i]->getMinUid() << ", " << m_ROIs[i]->getMinVid() << ", " << m_ROIs[i]->getMaxUid() << ", " <<
               m_ROIs[i]->getMaxVid());
    }
  }
  if (nROIs > m_ROIs.getEntries()) B2RESULT(" HOUSTON WE HAVE A PROBLEM!");

  m_h1okROIs->Fill(nROIs);
  n_OKrois += nROIs;
}


void SVDROIFinderAnalysisModule::endRun()
{
}


void SVDROIFinderAnalysisModule::terminate()
{

  Double_t epsilon[6];
  Double_t epsilonErr[6];
  double epsilonTot = (double)n_svdDigitInROI / (double) n_svdDigit;
  Double_t epsilon2[6];
  Double_t epsilon2Err[6];
  double epsilon2Tot = (double)n_tracksWithDigitsInROI / (double) n_tracksWithDigits;

  for (int i = 0; i < 6; i++) {
    m_h1digiOut2->SetBinContent(i + 1, nnotINdigit2[i]);
    m_h1digiOut3->SetBinContent(i + 1, nnotINdigit3[i]);
    m_h1digiOut4->SetBinContent(i + 1, nnotINdigit4[i]);
    m_h1digiOut5->SetBinContent(i + 1, nnotINdigit5[i]);
    m_h1digiIn->SetBinContent(i + 1, nsvdDigitInROI[i]);
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
  if (m_isSimulation)
    B2RESULT(" number of GOOD ROIs = " << n_OKrois);
  //  B2RESULT("");
  B2RESULT("          average number of ROIs = " << m_h1totROIs->GetMean());
  if (m_isSimulation) {
    B2RESULT(" average number of ROIs w digits = " << m_h1okROIs->GetMean());
    B2RESULT("");
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

    B2RESULT("    svdDigit : " << n_svdDigit);
    B2RESULT("  svdDigitIn : " << n_svdDigitInROI);

    B2RESULT("        eff DGT: " << epsilonTot << " +/- " << sqrt(epsilonTot * (1 - epsilonTot) / n_svdDigit));
    B2RESULT("  inefficiency (SVDShaperDigits): ");
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
    B2RESULT("    svdDigit : " << nsvdDigit[5]);
    B2RESULT("  svdDigitIn : " << nsvdDigitInROI[5]);
    if ((nsvdDigit[5] - nsvdDigitInROI[5]) != (nnotINdigit2[5] + nnotINdigit3[5] + nnotINdigit4[5] + nnotINdigit5[5]))
      B2RESULT(" svdDigitOut : " << nsvdDigit[5] - nsvdDigitInROI[5] << " != " << nnotINdigit2[5] + nnotINdigit3[5] + nnotINdigit4[5] +
               nnotINdigit5[5]);
    epsilon[5] = (double)nsvdDigitInROI[5] / (double) nsvdDigit[5];
    epsilonErr[5] = sqrt(epsilon[5] * (1 - epsilon[5]) / nsvdDigit[5]);
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
    B2RESULT("    svdDigit : " << nsvdDigit[4]);
    B2RESULT("  svdDigitIn : " << nsvdDigitInROI[4]);
    if ((nsvdDigit[4] - nsvdDigitInROI[4]) != (nnotINdigit2[4] + nnotINdigit3[4] + nnotINdigit4[4] + nnotINdigit5[4]))
      B2RESULT(" svdDigitOut : " << nsvdDigit[4] - nsvdDigitInROI[4] << " != " << nnotINdigit2[4] + nnotINdigit3[4] + nnotINdigit4[4] +
               nnotINdigit5[4]);
    epsilon[4] = (double)nsvdDigitInROI[4] / (double) nsvdDigit[4];
    epsilonErr[4] = sqrt(epsilon[4] * (1 - epsilon[4]) / nsvdDigit[4]);
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
    B2RESULT("    svdDigit : " << nsvdDigit[3]);
    B2RESULT("  svdDigitIn : " << nsvdDigitInROI[3]);
    if ((nsvdDigit[3] - nsvdDigitInROI[3]) != (nnotINdigit2[3] + nnotINdigit3[3] + nnotINdigit4[3] + nnotINdigit5[3]))
      B2RESULT(" svdDigitOut : " << nsvdDigit[3] - nsvdDigitInROI[3] << " != " << nnotINdigit2[3] + nnotINdigit3[3] + nnotINdigit4[3] +
               nnotINdigit5[3]);
    epsilon[3] = (double)nsvdDigitInROI[3] / (double) nsvdDigit[3];
    epsilonErr[3] = sqrt(epsilon[3] * (1 - epsilon[3]) / nsvdDigit[3]);
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
    B2RESULT("    svdDigit : " << nsvdDigit[2]);
    B2RESULT("  svdDigitIn : " << nsvdDigitInROI[2]);
    if ((nsvdDigit[2] - nsvdDigitInROI[2]) != (nnotINdigit2[2] + nnotINdigit3[2] + nnotINdigit4[2] + nnotINdigit5[2]))
      B2RESULT(" svdDigitOut : " << nsvdDigit[2] - nsvdDigitInROI[2] << " != " << nnotINdigit2[2] + nnotINdigit3[2] + nnotINdigit4[2] +
               nnotINdigit5[2]);
    epsilon[2] = (double)nsvdDigitInROI[2] / (double) nsvdDigit[2];
    epsilonErr[2] = sqrt(epsilon[2] * (1 - epsilon[2]) / nsvdDigit[2]);
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
    B2RESULT("    svdDigit : " << nsvdDigit[1]);
    B2RESULT("  svdDigitIn : " << nsvdDigitInROI[1]);
    if ((nsvdDigit[1] - nsvdDigitInROI[1]) != (nnotINdigit2[1] + nnotINdigit3[1] + nnotINdigit4[1] + nnotINdigit5[1]))
      B2RESULT(" svdDigitOut : " << nsvdDigit[1] - nsvdDigitInROI[1] << " ?=? " << nnotINdigit2[1] + nnotINdigit3[1] + nnotINdigit4[1] +
               nnotINdigit5[1]);
    epsilon[1] = (double)nsvdDigitInROI[1] / (double) nsvdDigit[1];
    epsilonErr[1] = sqrt(epsilon[1] * (1 - epsilon[1]) / nsvdDigit[1]);
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
    B2RESULT("    svdDigit : " << nsvdDigit[0]);
    B2RESULT("  svdDigitIn : " << nsvdDigitInROI[0]);
    if ((nsvdDigit[0] - nsvdDigitInROI[0]) != (nnotINdigit2[0] + nnotINdigit3[0] + nnotINdigit4[0] + nnotINdigit5[0]))
      B2RESULT(" svdDigitOut : " << nsvdDigit[0] - nsvdDigitInROI[0] << " ?=? " << nnotINdigit2[0] + nnotINdigit3[0] + nnotINdigit4[0] +
               nnotINdigit5[0]);
    epsilon[0] = (double)nsvdDigitInROI[0] / (double) nsvdDigit[0];
    epsilonErr[0] = sqrt(epsilon[0] * (1 - epsilon[0]) / nsvdDigit[0]);
    B2RESULT("  efficiency : " << epsilon[0] << " +/- " << epsilonErr[0]);
    epsilon2[0] = (double)TrackOneDigiIn[0] / (double) totTrack[0]  ;
    epsilon2Err[0] = sqrt(epsilon2[0] * (1 - epsilon2[0]) / totTrack[0]);
    B2RESULT("  efficiency2 : " << epsilon2[0] << " +/- " << epsilon2Err[0]);

    B2RESULT("legend:");
    B2RESULT(" CASO2:  if (ROI exists but no SVDShaperDigit inside)");
    B2RESULT(" CASO3:  if (ROI does not exist, intercept with correct VxdID)");
    B2RESULT(" CASO4:  if (intercept with wrong VxdID)");
    B2RESULT(" CASO5:  if (intercept does not exist)");

    B2RESULT("==========================");
  }
  B2RESULT(" tot ROIs = " << n_rois);
  B2RESULT(" ROIs with digits = " << m_nGoodROIs);
  B2RESULT(" SVD INefficiency = " << 1 - (float)m_nGoodROIs / n_rois);

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
    m_h1SigmaU->Write();
    m_h1SigmaV->Write();
    m_h2sigmaUphi->Write();
    m_h2sigmaVphi->Write();

    m_out2->cd();
    m_h1GlobalTime_out2->Write();
    m_h1ResidU_out2->Write();
    m_h1ResidV_out2->Write();
    m_h1SigmaU_out2->Write();
    m_h1SigmaV_out2->Write();
    m_h2sigmaUphi_out2->Write();
    m_h2sigmaVphi_out2->Write();

    m_out3->cd();
    m_h1GlobalTime_out3->Write();
    m_h1ResidU_out3->Write();
    m_h1ResidV_out3->Write();
    m_h1SigmaU_out3->Write();
    m_h1SigmaV_out3->Write();
    m_h2sigmaUphi_out3->Write();
    m_h2sigmaVphi_out3->Write();

    m_out4->cd();
    m_h1GlobalTime_out4->Write();
    m_h1SigmaU_out4->Write();
    m_h1SigmaV_out4->Write();
    m_h2sigmaUphi_out4->Write();
    m_h2sigmaVphi_out4->Write();

    m_out5->cd();
    m_h1GlobalTime_out5->Write();

    m_ROIDir->cd();
    m_h1totUstrips->Write();
    m_h1totVstrips->Write();
    m_h1okROIs->Write();
    m_h1totROIs->Write();


    m_h2ROIbottomLeft->Write();
    m_h2ROItopRight->Write();
    m_h2ROIuMinMax->Write();
    m_h2ROIvMinMax->Write();
    m_rootFilePtr->Close();

  }

}

