/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/pxdDataReduction/PXDDataRedAnalysisModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/RelationArray.h>

#include <genfit/TrackCand.h>
#include <mdst/dataobjects/MCParticle.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
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
REG_MODULE(PXDDataRedAnalysis)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDDataRedAnalysisModule::PXDDataRedAnalysisModule()
  : Module()
  , n_tracks(0)
  , n_tracksWithDigits(0)
  , n_pxdDigit(0)
  , n_pxdDigitInROI(0)
  , n_noHit2(0)
  , n_noHit3(0)
  , n_noROI4(0)
  , n_noROI5(0)
{
  //Set module properties
  setDescription("This module performs the analysis of the PXDDataReduction module output");

  addParam("writeToRoot", m_writeToRoot,
           "set true if you want to save the informations in a root file named by parameter 'rootFileName'", bool(true));

  addParam("rootFileName", m_rootFileName,
           "fileName used for . Will be ignored if parameter 'writeToRoot' is false (standard)",
           string("pxdDataRedAnalysis"));

  addParam("trackCandCollName", m_gfTrackCandsColName,
           "name of the input collection of track candidates", std::string(""));

  addParam("PXDInterceptListName", m_PXDInterceptListName,
           "name of the list of interceptions", std::string(""));

  addParam("ROIListName", m_ROIListName,
           "name of the list of ROIs", std::string(""));

  m_rootEvent = 0;
}

PXDDataRedAnalysisModule::~PXDDataRedAnalysisModule()
{
}


void PXDDataRedAnalysisModule::initialize()
{

  StoreArray<genfit::TrackCand>::required(m_gfTrackCandsColName);
  StoreArray<ROIid>::required(m_ROIListName);
  StoreArray<PXDIntercept>::required(m_PXDInterceptListName);

  StoreArray<MCParticle>::required();

  n_tracks         = 0;
  n_pxdDigit       = 0;
  n_pxdDigitInROI  = 0;

  m_nNoMCPart = 0;


  NtrackHit = 0;
  NtrackNoHit2 = 0;
  NtrackNoHit3 = 0;
  NtrackNoROI4 = 0;
  NtrackNoROI5 = 0;

  for (int i = 0; i < 6; i++) {
    npxdDigit[i] = 0;
    npxdDigitInROI[i] = 0;
    nnoHit2[i] = 0;
    nnoHit3[i] = 0;
    nnoROI4[i] = 0;
    nnoROI5[i] = 0;
  }

  if (m_writeToRoot == true) {
    m_rootFileName += ".root";
    m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");
  } else
    m_rootFilePtr = NULL;

  m_h1ptAll = new TH1F("hptAll", "transverse momentum for all PXDDigits", 100, 0, 3);
  m_h1pt = new TH1F("hpt", "transverse momentum for all PXDDigits contained in ROI", 100, 0, 3);
  m_h1ptBad = new TH1F("hptBad", "transverse momentum for PXDDigits  when no ROI exists and intercept has wrong vxdID", 100, 0, 3);

  m_h1GlobalTime = new TH1F("hGlobalTime", "global time for PXDDigits contained in ROI", 100, 0, 1);
  m_h1GlobalTimeFail = new TH1F("hGlobalTimeFail", "global time for PXDDigits NOT contained in ROI", 500, 0, 10);
  m_h1GlobalTimeNoROI = new TH1F("hGlobalTimeNoROI", "global time for PXDDigits when no ROI exists", 500, 0, 10);
  m_h1GlobalTimeBad = new TH1F("hGlobalTimeBad", "global time for PXDDigits when no ROI exists and intercept has wrong vxdID", 500, 0, 10);

  m_h1PullU = new TH1F("hPullU", "U pulls for PXDDigits contained in ROI", 100, -6, 6);
  m_h1PullV = new TH1F("hPullV", "V pulls for PXDDigits contained in ROI", 100, -6, 6);
  m_h1PullUFail = new TH1F("hPullUFail", "U pulls for intercepts NOT contained in ROI", 100, -50, 50);
  m_h1PullVFail = new TH1F("hPullVFail", "V pulls for intercepts NOT contained in ROI", 100, -50, 50);
  m_h1PullUNoROI = new TH1F("hPullUNoROI", "U pulls for intercepts when no ROI exists", 100, -6, 6);
  m_h1PullVNoROI = new TH1F("hPullVNoROI", "V pulls for intercepts when no ROI exists", 100, -6, 6);

  m_h1ResidU = new TH1F("hResidU", "U resid for PXDDigits contained in ROI", 100, -6, 6);
  m_h1ResidV = new TH1F("hResidV", "V resid for PXDDigits contained in ROI", 100, -6, 6);
  m_h1ResidUFail = new TH1F("hResidUFail", "U resid for intercepts NOT contained in ROI", 100, -10, 10);
  m_h1ResidVFail = new TH1F("hResidVFail", "V resid for intercepts NOT contained in ROI", 100, -10, 10);
  m_h1ResidUNoROI = new TH1F("hResidUNoROI", "U resid for intercepts when no ROI exists", 100, -6, 6);
  m_h1ResidVNoROI = new TH1F("hResidVNoROI", "V resid for intercepts when no ROI exists", 100, -6, 6);

  m_h1SigmaU = new TH1F("hSigmaU", "sigmaU for PXDDigits contained in ROI", 100, 0, 0.35);
  m_h1SigmaV = new TH1F("hSigmaV", "sigmaV for PXDDigits contained in ROI", 100, 0, 0.35);
  m_h1SigmaUFail = new TH1F("hSigmaUFail", "sigmaU for intercepts NOT contained in ROI", 100, 0, 1);
  m_h1SigmaVFail = new TH1F("hSigmaVFail", "sigmaV for intercepts NOT contained in ROI", 100, 0, 1);
  m_h1SigmaUNoROI = new TH1F("hSigmaUNoROI", "sigmaU for intercepts when no ROI exists", 100, 0, 1);
  m_h1SigmaVNoROI = new TH1F("hSigmaVNoROI", "sigmaV for intercepts when no ROI exists", 100, 0, 1);

  m_h1DistUFail = new TH1F("hDistUFail", "distance of PXDDigit from ROI (if NOT contained)", 600, -300, 300);
  m_h1DistVFail = new TH1F("hDistVFail", "distance of PXDDigit from ROI (if NOT contained)", 800, -500, 300);
  m_h2DistUVFail = new TH2F("hDistUVFail", "distance of PXDDigit from ROI (if NOT contained)", 600, -300, 300, 800, -500, 300);

  m_h1redFactor = new TH1F("hRedFactor", "reduction factor", 100, 0, 0.1);
  m_h1totROIs = new TH1F("htotNrois", "number of ROIs", 100, 0, 100);
  m_h1nROIs = new TH1F("hNrois", "number of ROIs", 100, 0, 100);
  m_h1nROIs_all = new TH1F("hNrois_all", "number of ROIs", 100, 0, 100);
  m_h1totarea = new TH1F("hTotArea", "Total Areas of ROIs", 100, 0, 100000);
  m_h1area = new TH1F("hArea", "Areas of ROIs", 100, 0, 10000);
  m_h1areaFail = new TH1F("hAreaFail", "Areas of ROIs when PXDDigit is not contained", 100, 0, 10000);

  m_h1Phi = new TH1F("hPhi", "phi when PXDDigit is contained in a ROI", 100, -180, 180);
  m_h1Theta = new TH1F("hTheta", "theta when PXDDigit is contained in a ROI", 100, 0, 180);
  m_h1CosTheta = new TH1F("hCosTheta", "costheta when PXDDigit is contained in a ROI", 100, -1, 1);
  m_h1Lambda = new TH1F("hLambda", "lambda when the PXDDigit is contained in a ROI", 100, -90, 90);

  m_h1CoorUBad = new TH1F("hCoorUBad", "u coor when no ROI and wrong intercept VxdID", 100, -2, 2);
  m_h1CoorVBad = new TH1F("hCoorVBad", "v coor when no ROI and wrong intercept VxdID", 100, -10, 10);
  m_h2CoorUVBad = new TH2F("hCoorUVBad", "u,v coor when no ROI and wrong intercept VxdID", 100, -2, 2, 100, -10, 10);

  m_h1PhiBad = new TH1F("hPhiBad", "phi when no ROI and wrong intercept VxdID", 100, -180, 180);
  m_h1PhiBad_L1 = new TH1F("hPhiBad_L1", "L1 phi when no ROI and wrong intercept VxdID", 100, -180, 180);
  m_h1PhiBad_L2 = new TH1F("hPhiBad_L2", "L2 phi when no ROI and wrong intercept VxdID", 100, -180, 180);
  m_h1PhiBadLambda0_L1 = new TH1F("hPhiBadLambda0_L1", "L1 phi (|lambda| < 10) when no ROI and wrong intercept VxdID", 16, -180, 180);
  m_h1PhiBadLambda0_L2 = new TH1F("hPhiBadLambda0_L2", "L2 phi (|lambda| < 10) when no ROI and wrong intercept VxdID", 24, -180, 180);
  m_h1PhiBadLambdaF_L1 = new TH1F("hPhiBadLambdaF_L1", "L1 phi (lambda > 55) when no ROI and wrong intercept VxdID", 16, -180, 180);
  m_h1PhiBadLambdaF_L2 = new TH1F("hPhiBadLambdaF_L2", "L2 phi (lambda > 55) when no ROI and wrong intercept VxdID", 24, -180, 180);
  m_h1ThetaBad = new TH1F("hThetaBad", "theta when no ROI and wrong intercept VxdID", 100, 0, 180);
  m_h1LambdaBad = new TH1F("hLambdaBad", "lambda when no ROI and wrong intercept VxdID", 100, -180, 180);
  m_h1CosThetaBad = new TH1F("hCosThetaBad", "costheta when PXDDigit is contained in a ROI", 100, -1, 1);
  m_h1LambdaBad_timeL1 = new TH1F("hLambdaBad_timeLess1", "lambda when no ROI and wrong intercept VxdID", 100, -180, 180);
  m_h1LambdaBad_timeG1 = new TH1F("hLambdaBad_timeGreater1", "lambda when no ROI and wrong intercept VxdID", 100, -180, 180);

  m_h1CosThetaMCPart = new TH1F("hCosThetaMCPart", "costheta MCParticles", 100, -1, 1);

  m_hNhits = new TH1F("hNHits", "number of SVD hits of the GFTrackCand", 100, 0, 100);
  m_hNhitsBad = new TH1F("hNHitsBad", "number of SVD hits of the GFTrackCand BAD", 100, 0, 100);

  m_h2Map = new TH2F("h2Map", "test", 500, 1.2, 2.6, 100, -TMath::Pi(), TMath::Pi());
  //  m_h2MapBad_L1 = new TH2F("h2MapBad_L1","z,phi BAD",100,-6,8,100,-TMath::Pi(),TMath::Pi());
  m_h2MapBad_L1 = new TH2F("h2MapBad_L1", "z,phi BAD", 500, 1.2, 2.6, 100, -TMath::Pi(), TMath::Pi());
  m_h2MapBad_L2 = new TH2F("h2MapBad_L2", "z,phi BAD", 100, -6, 8, 100, -TMath::Pi(), TMath::Pi());


  //analysis
  Double_t lowBin[6 + 1];
  for (int i = 0; i < 6; i++)
    lowBin[i] = pt[6 - 1 - i] - ptErr[6 - 1 - i];
  lowBin[6] = pt[0] + ptErr[0];

  m_h1digiIn = new TH1F("hDigiIn", "PXDDigits contained in a ROI", 6, lowBin);
  m_h1digiOut2 = new TH1F("hDigiOut2", "lost PXDDigits: ROI exists with right VxdID", 6, lowBin);
  m_h1digiOut3 = new TH1F("hDigiOut3", "lost PXDDigits: ROI exists with wrong VxdID", 6, lowBin);
  m_h1digiOut4 = new TH1F("hDigiOut4", "lost PXDDigits: ROI does not exist, intercept with right VxdID", 6, lowBin);
  m_h1digiOut5 = new TH1F("hDigiOut5", "lost PXDDigits: ROI does not exist, intercept with wrong VxdID", 6, lowBin);

}

void PXDDataRedAnalysisModule::beginRun()
{
  m_rootEvent = 0;
}


void PXDDataRedAnalysisModule::event()
{
  int nROIs = 0;
  int nROIs_all = 0;
  int totArea = 0;
  B2DEBUG(1, "  ++++++++++++++ PXDDataRedAnalysisModule");
  StoreArray<genfit::TrackCand> trackCandList(m_gfTrackCandsColName);
  StoreArray<PXDIntercept> PXDInterceptList(m_PXDInterceptListName);
  StoreArray<ROIid> ROIList(m_ROIListName);

  m_h1totROIs->Fill(ROIList.getEntries());

  StoreArray<MCParticle> mcParticles;

  StoreArray<PXDDigit> pxdDigits;
  StoreArray<PXDSimHit> pxdSimHits;
  StoreArray<PXDTrueHit> pxdTrueHits;

  RelationIndex < genfit::TrackCand, PXDIntercept >
  gfTrackCandToPXDIntercept(DataStore::relationName(m_gfTrackCandsColName, m_PXDInterceptListName));

  if (! gfTrackCandToPXDIntercept)
    B2FATAL("No genfit::TrackCand -> PXDIntercept relation found! :'(");

  typedef RelationIndex < genfit::TrackCand, PXDIntercept>::range_from PXDInterceptsFromGFTracks;
  typedef RelationIndex < genfit::TrackCand, PXDIntercept>::iterator_from PXDInterceptIteratorType;

  typedef RelationIndex < PXDDigit, PXDTrueHit>::range_from PXDTrueHitFromPXDDigit;
  typedef RelationIndex < PXDDigit, PXDTrueHit>::iterator_from PXDTrueHitIteratorType;

  RelationIndex < PXDDigit, PXDTrueHit > relDigitTrueHit(DataStore::relationName(
                                                           DataStore::arrayName<PXDDigit>(""),
                                                           DataStore::arrayName<PXDTrueHit>(""))
                                                        );
  double tmpGlobalTime;
  int tmpNGlobalTime;

  //  for(int iMCPart =0; iMCPart < mcParticles.getEntries(); iMCPart++){
  //    const MCParticle* aPart = mcParticles[iMCPart];
  const MCParticle* aPart = mcParticles[0];

  double momX = (aPart->getMomentum()).X();
  double momY = (aPart->getMomentum()).Y();
  double momZ = (aPart->getMomentum()).Z();

  m_costhetaMCPart = TMath::Cos(3.1415 / 2 - atan(momZ / sqrt(TMath::Power(momX, 2) + TMath::Power(momY, 2))));
  m_h1CosThetaMCPart->Fill(m_costhetaMCPart);

  //  }

  for (int i = 0; i < trackCandList.getEntries(); i++) {

    n_tracks ++;

    bool onceHit(false);
    bool onceNoHit2(false);
    bool onceNoHit3(false);
    bool onceNoROI4(false);
    bool onceNoROI5(false);

    int McId = trackCandList[i]->getMcTrackId();

    m_hNhits->Fill(trackCandList[i]->getNHits());

    if (McId < 0) {
      m_nNoMCPart++;
      continue;
    }

    MCParticle* aMcParticle = mcParticles[McId];

    RelationVector<PXDDigit> pxdRelations = aMcParticle->getRelationsFrom<PXDDigit>();

    PXDInterceptsFromGFTracks  PXDIntercepts = gfTrackCandToPXDIntercept.getElementsFrom(trackCandList[i]);

    // m_rootMomXmc = (aMcParticle->getMomentum()).X();
    // m_rootMomYmc = (aMcParticle->getMomentum()).Y();
    // m_rootMomZmc = (aMcParticle->getMomentum()).Z();

    bool theROI(false);
    bool interceptRightVxdID = false;
    bool roiRightVxdID = false;

    if (pxdRelations.size() >= 1)
      n_tracksWithDigits++;

    m_momXmc = (aMcParticle->getMomentum()).X();
    m_momYmc = (aMcParticle->getMomentum()).Y();
    m_momZmc = (aMcParticle->getMomentum()).Z();
    m_phimc =  atan2(m_momYmc, m_momXmc) * 180 / 3.1415;
    m_lambdamc = atan(m_momZmc / sqrt(TMath::Power(m_momXmc, 2) + TMath::Power(m_momYmc, 2))) * 180 / 3.1415;
    m_thetamc = 90 - m_lambdamc;
    m_costhetamc = TMath::Cos(m_thetamc / 180 * 3.1415);

    pT = sqrt(TMath::Power(m_momXmc, 2) + TMath::Power(m_momYmc, 2));

    for (unsigned int iPXDDigit = 0; iPXDDigit < pxdRelations.size(); iPXDDigit++) {

      n_pxdDigit ++ ;

      PXDTrueHitFromPXDDigit  PXDTrueHits = relDigitTrueHit.getElementsFrom(*pxdRelations[iPXDDigit]);
      PXDTrueHitIteratorType thePXDTrueHitIterator = PXDTrueHits.begin();
      PXDTrueHitIteratorType thePXDTrueHitIteratorEnd = PXDTrueHits.end();
      tmpGlobalTime = 0;
      tmpNGlobalTime = 0;

      for (; thePXDTrueHitIterator != thePXDTrueHitIteratorEnd; thePXDTrueHitIterator++) {
        tmpGlobalTime = tmpGlobalTime + thePXDTrueHitIterator->to->getGlobalTime();
        tmpNGlobalTime++;
      }
      m_globalTime = tmpGlobalTime / tmpNGlobalTime;

      //      m_coorUmc = thePXDTrueHitIterator->to->getU(); //
      //      m_coorVmc = thePXDTrueHitIterator->to->getV();
      m_coorUmc = pxdRelations[iPXDDigit]->getUCellPosition();
      m_coorVmc = pxdRelations[iPXDDigit]->getVCellPosition();
      m_Uidmc = pxdRelations[iPXDDigit]->getUCellID();
      m_Vidmc = pxdRelations[iPXDDigit]->getVCellID();
      m_vxdIDmc = pxdRelations[iPXDDigit]->getSensorID();


      //giulia
      VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
      TVector3 local(m_coorUmc, m_coorVmc, 0);
      //      TVector3 local(2, 3, 0);
      //      TVector3 uVector(1, 0, 0);
      //      TVector3 vVector(0, 1, 0);

      const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(pxdRelations[iPXDDigit]->getSensorID());
      TVector3 globalSensorPos = aSensorInfo.pointToGlobal(local);
      //      TVector3 globaluVector = aSensorInfo.vectorToGlobal(uVector);
      //      TVector3 globalvVector = aSensorInfo.vectorToGlobal(vVector);

      //      if( ( pxdRelations[iPXDDigit]->getSensorID() ).getLayerNumber() == 1) //L1
      //m_hTest->Fill( globalSensorPos.Mag() , globalSensorPos.Phi());
      m_h2Map->Fill(globalSensorPos.Perp() , globalSensorPos.Phi());



      m_h1ptAll->Fill(pT);
      if (pT > 1) npxdDigit[5]++;
      if (pT <= 1 && pT > 0.5) npxdDigit[4]++;
      if (pT <= 0.5 && pT > 0.3) npxdDigit[3]++;
      if (pT <= 0.3 && pT > 0.2) npxdDigit[2]++;
      if (pT <= 0.2 && pT > 0.1) npxdDigit[1]++;
      if (pT <= 0.1) npxdDigit[0]++;

      PXDInterceptIteratorType thePXDInterceptIterator = PXDIntercepts.begin();
      PXDInterceptIteratorType thePXDInterceptIteratorEnd = PXDIntercepts.end();
      bool MissingHit(true);

      double tmpPullU = 0;
      double tmpPullV = 0;
      double tmpResidU = 0;
      double tmpResidV = 0;
      double tmpSigmaU = 0;
      double tmpSigmaV = 0;
      int tmpDistU = 0 ;
      int tmpDistV = 0 ;
      double tmpArea = 0;

      double distance = 100000;
      double distanceROI = 100000;

      for (; thePXDInterceptIterator != thePXDInterceptIteratorEnd; thePXDInterceptIterator++) {

        theROI = false;
        interceptRightVxdID = false;
        roiRightVxdID = false;

        const PXDIntercept* theIntercept = thePXDInterceptIterator->to;

        if (theIntercept) {

          m_coorU = theIntercept->getCoorU();
          m_coorV = theIntercept->getCoorV();
          m_sigmaU = theIntercept->getSigmaU();
          m_sigmaV = theIntercept->getSigmaV();
          m_vxdID = theIntercept->getSensorID();
          // m_lambda = theIntercept->getLambda();

          if (m_vxdID == m_vxdIDmc) {

            interceptRightVxdID = true;

            if (distance >  TMath::Sqrt(TMath::Power(m_coorU - m_coorUmc, 2) + TMath::Power(m_coorU - m_coorUmc, 2))) {

              tmpPullU = (m_coorU - m_coorUmc) / m_sigmaU;
              tmpPullV = (m_coorV - m_coorVmc) / m_sigmaV;

              tmpResidU = m_coorU - m_coorUmc;
              tmpResidV = m_coorV - m_coorVmc;

              tmpSigmaU = m_sigmaU;
              tmpSigmaV = m_sigmaV;

              distance = TMath::Power(m_coorU - m_coorUmc, 2) + TMath::Power(m_coorV - m_coorVmc, 2);
            }
          }

          const ROIid* theROIid = theIntercept->getRelatedTo<ROIid>(m_ROIListName);

          if (theROIid) {
            nROIs_all++;

            theROI = true;

            if ((int)theROIid->getSensorID() == m_vxdIDmc) {

              roiRightVxdID = true;

              if (distanceROI > TMath::Power(tmpDistU, 2) + TMath::Power(tmpDistV, 2)) {

                if (m_Uidmc < theROIid->getMinUid())
                  tmpDistU = m_Uidmc - theROIid->getMinUid();
                else
                  tmpDistU = m_Uidmc - theROIid->getMaxUid();

                if (m_Vidmc < theROIid->getMinVid())
                  tmpDistV = m_Vidmc - theROIid->getMinVid();
                else
                  tmpDistV = m_Vidmc - theROIid->getMaxVid();

                tmpArea = (theROIid->getMaxUid() - theROIid->getMinUid()) * (theROIid->getMaxVid() - theROIid->getMinVid());

                distanceROI = TMath::Power(tmpDistU, 2) + TMath::Power(tmpDistV, 2);

              }
            }

            if (theROIid->Contains(*(pxdRelations[iPXDDigit]))) { //CASO1

              onceHit = true;

              m_h1GlobalTime->Fill(m_globalTime);

              m_h1PullU->Fill((m_coorU - m_coorUmc) / m_sigmaU);
              m_h1PullV->Fill((m_coorV - m_coorVmc) / m_sigmaV);

              m_h1ResidU->Fill(m_coorU - m_coorUmc);
              m_h1ResidV->Fill(m_coorV - m_coorVmc);

              m_h1SigmaU->Fill(m_sigmaU);
              m_h1SigmaV->Fill(m_sigmaV);

              m_h1area->Fill(tmpArea);
              totArea = totArea + tmpArea;
              nROIs++;

              m_h1Phi->Fill(m_phimc);
              m_h1Theta->Fill(m_thetamc);
              m_h1CosTheta->Fill(m_costhetamc);
              m_h1Lambda->Fill(m_lambdamc);
              m_h1pt->Fill(pT);

              if (pT > 1) npxdDigitInROI[5]++;
              if (pT <= 1 && pT > 0.5) npxdDigitInROI[4]++;
              if (pT <= 0.5 && pT > 0.3) npxdDigitInROI[3]++;
              if (pT <= 0.3 && pT > 0.2) npxdDigitInROI[2]++;
              if (pT <= 0.2 && pT > 0.1) npxdDigitInROI[1]++;
              if (pT <= 0.1) npxdDigitInROI[0]++;


              MissingHit = false;

              break; // To avoid double counting

            } //if theROIid contains
          } //if (theROIid)
        } //if (theintercept)
      } //(end loop on intercept list)


      if (MissingHit) {

        if (theROI && roiRightVxdID) { //CASO2

          onceNoHit2 = true;
          m_h1GlobalTimeFail->Fill(m_globalTime);

          m_h1PullUFail->Fill(tmpPullU);
          m_h1PullVFail->Fill(tmpPullV);

          m_h1ResidUFail->Fill(tmpResidU);
          m_h1ResidVFail->Fill(tmpResidV);

          m_h1SigmaUFail->Fill(tmpSigmaU);
          m_h1SigmaVFail->Fill(tmpSigmaV);

          m_h1DistUFail->Fill(tmpDistU);
          m_h1DistVFail->Fill(tmpDistV);
          m_h2DistUVFail->Fill(tmpDistU, tmpDistV);

          m_h1areaFail->Fill(tmpArea);

          n_noHit2 ++;

          if (pT > 1) nnoHit2[5]++;
          if (pT <= 1 && pT > 0.5) nnoHit2[4]++;
          if (pT <= 0.5 && pT > 0.3) nnoHit2[3]++;
          if (pT <= 0.3 && pT > 0.2) nnoHit2[2]++;
          if (pT <= 0.2 && pT > 0.1) nnoHit2[1]++;
          if (pT <= 0.1) nnoHit2[0]++;


          cout << "@";
        }

        if (theROI && !roiRightVxdID) { //CASO3
          onceNoHit3 = true;
          if (pT > 1) nnoHit3[5]++;
          if (pT <= 1 && pT > 0.5) nnoHit3[4]++;
          if (pT <= 0.5 && pT > 0.3) nnoHit3[3]++;
          if (pT <= 0.3 && pT > 0.2) nnoHit3[2]++;
          if (pT <= 0.2 && pT > 0.1) nnoHit3[1]++;
          if (pT <= 0.1) nnoHit3[0]++;

          n_noHit3 ++;
          cout << "#";
        }

        if (!theROI && interceptRightVxdID) { //CASO4
          onceNoROI4 = true;
          if (pT > 1) nnoROI4[5]++;
          if (pT <= 1 && pT > 0.5) nnoROI4[4]++;
          if (pT <= 0.5 && pT > 0.3) nnoROI4[3]++;
          if (pT <= 0.3 && pT > 0.2) nnoROI4[2]++;
          if (pT <= 0.2 && pT > 0.1) nnoROI4[1]++;
          if (pT <= 0.1) nnoROI4[0]++;

          m_h1GlobalTimeNoROI->Fill(m_globalTime);

          m_h1PullUNoROI->Fill(tmpPullU);
          m_h1PullVNoROI->Fill(tmpPullV);

          m_h1ResidUNoROI->Fill(tmpResidU);
          m_h1ResidVNoROI->Fill(tmpResidV);

          m_h1SigmaUNoROI->Fill(tmpSigmaU);
          m_h1SigmaVNoROI->Fill(tmpSigmaV);

          cout << "$";
          n_noROI4 ++;
        }
        if (!theROI && !interceptRightVxdID) { //CASO5

          //    if((pxdRelations[iPXDDigit]->getSensorID()).getLayerNumber() == 1)
          m_h2MapBad_L1->Fill(globalSensorPos.Perp() , globalSensorPos.Phi());
          //    else
          //      m_h2MapBad_L2->Fill( globalSensorPos.Perp() , globalSensorPos.Phi());



          if (!onceNoROI5) {
            m_hNhitsBad->Fill(trackCandList[i]->getNHits());
            if ((pxdRelations[iPXDDigit]->getSensorID()).getLayerNumber() == 1) {
              m_h1PhiBad_L1->Fill(m_phimc);
              if (abs(m_lambdamc) < 10)
                m_h1PhiBadLambda0_L1->Fill(m_phimc);
              if (m_lambdamc > 55)
                m_h1PhiBadLambdaF_L1->Fill(m_phimc);
            } else {
              m_h1PhiBad_L2->Fill(m_phimc);
              if (abs(m_lambdamc) < 10)
                m_h1PhiBadLambda0_L2->Fill(m_phimc);
              if (m_lambdamc > 55)
                m_h1PhiBadLambdaF_L2->Fill(m_phimc);
            }
          }
          onceNoROI5 = true;

          if (pT > 1) nnoROI5[5]++;
          if (pT <= 1 && pT > 0.5) nnoROI5[4]++;
          if (pT <= 0.5 && pT > 0.3) nnoROI5[3]++;
          if (pT <= 0.3 && pT > 0.2) nnoROI5[2]++;
          if (pT <= 0.2 && pT > 0.1) nnoROI5[1]++;
          if (pT <= 0.1) nnoROI5[0]++;

          // m_h1PhiBad->Fill(m_phimc);
          // m_h1ThetaBad->Fill(m_thetamc);
          // m_h1CosThetaBad->Fill(m_costhetamc);
          // m_h1LambdaBad->Fill(m_lambdamc);

          // m_h1ptBad->Fill(pT);

          //digits stuff
          if (m_globalTime < 1)
            m_h1LambdaBad_timeL1->Fill(m_lambdamc);
          else
            m_h1LambdaBad_timeG1->Fill(m_lambdamc);

          m_h1GlobalTimeBad->Fill(m_globalTime);
          m_h1CoorUBad->Fill(m_coorUmc);
          m_h1CoorVBad->Fill(m_coorVmc);
          m_h2CoorUVBad->Fill(m_coorUmc, m_coorVmc);

          cout << "%";
          n_noROI5 ++;
        }
      } else {
        n_pxdDigitInROI ++;
        cout << "+";
      }
    }

    if (onceHit) NtrackHit++;
    if (onceNoHit2) NtrackNoHit2++;
    if (onceNoHit3) NtrackNoHit3++;
    if (onceNoROI4) NtrackNoROI4++;
    if (onceNoROI5) { // && onceHit) {
      //    if(onceNoROI5 && !onceHit && !onceNoHit2 && !onceNoHit3 && !onceNoROI4) {
      NtrackNoROI5++;
      /*      if (pT > 1) nnoROI5[5]++;
      if (pT <= 1 && pT > 0.5) nnoROI5[4]++;
      if (pT <= 0.5 && pT > 0.3) nnoROI5[3]++;
      if (pT <= 0.3 && pT > 0.2) nnoROI5[2]++;
      if (pT <= 0.2 && pT > 0.1) nnoROI5[1]++;
      if (pT <= 0.1) nnoROI5[0]++;*/
      m_h1PhiBad->Fill(m_phimc);
      m_h1ThetaBad->Fill(m_thetamc);
      m_h1CosThetaBad->Fill(m_costhetamc);
      m_h1LambdaBad->Fill(m_lambdamc);

      m_h1ptBad->Fill(pT);
    }
  }

  m_rootEvent++;
  m_h1totarea->Fill(totArea);
  m_h1redFactor->Fill((double)totArea / 768. / 250. / 40.);
  cout << endl << " o  ANA: area = " << totArea << "  redFactor = " << (double)totArea / 768. / 250. / 40. << endl;
  m_h1nROIs->Fill(nROIs);
  m_h1nROIs_all->Fill(nROIs_all);
  cout << "" << endl;
}


void PXDDataRedAnalysisModule::endRun()
{
}


void PXDDataRedAnalysisModule::terminate()
{

  Double_t epsilon[6];
  Double_t epsilonErr[6];

  B2INFO("     ROI Analysis Summary     ");
  B2INFO("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  B2INFO("~ Efficiency estimated from  ~");
  B2INFO("      tracks : " << n_tracks);
  B2INFO("tracks w digits: " << n_tracksWithDigits);
  B2INFO("    pxdDigit : " << n_pxdDigit);
  B2INFO("  pxdDigitIn : " << n_pxdDigitInROI);
  double epsilonTot = (double)n_pxdDigitInROI / (double) n_pxdDigit;
  B2INFO("  efficiency : " << epsilonTot << " +/- " << sqrt(epsilonTot * (1 - epsilonTot) / n_pxdDigit));
  B2INFO("  inefficiency (PXDDigits): ");
  B2INFO("       no hit: " << n_noHit2 + n_noHit3);
  B2INFO("         correct vxdID: " << n_noHit2);
  B2INFO("         wrong vxdID: " << n_noHit3);
  B2INFO("       no ROI: " << n_noROI4 + n_noROI5);
  B2INFO("         correct vxdID: " << n_noROI4);
  B2INFO("         wrong vxdID: " << n_noROI5);
  B2INFO("");

  B2INFO(" N tracks hit = " << NtrackHit);
  B2INFO(" N tracks No Hit = " << NtrackNoHit2 << " + " << NtrackNoHit3);
  B2INFO(" N tracks No ROI = " << NtrackNoROI4 << " + " << NtrackNoROI5);
  B2INFO("overlap = " << NtrackHit + NtrackNoHit2 + NtrackNoHit3 + NtrackNoROI4 + NtrackNoROI5 - n_tracks);
  B2INFO("");

  B2INFO(" pT > 1");
  B2INFO("       no hit: " << nnoHit2[5] << " + " << nnoHit3[5]);
  B2INFO("       no ROI: " << nnoROI4[5] << " + " << nnoROI5[5]);
  B2INFO("    pxdDigit : " << npxdDigit[5]);
  B2INFO("  pxdDigitIn : " << npxdDigitInROI[5]);
  epsilon[5] = (double)npxdDigitInROI[5] / (double) npxdDigit[5];
  epsilonErr[5] = sqrt(epsilon[5] * (1 - epsilon[5]) / npxdDigit[5]);
  B2INFO("  efficiency : " << epsilon[5] << " +/- " << epsilonErr[5]);

  B2INFO("");
  B2INFO(" 0.5 < pT < 1");
  B2INFO("       no hit: " << nnoHit2[4] << " + " << nnoHit3[4]);
  B2INFO("       no ROI: " << nnoROI4[4] << " + " << nnoROI5[4]);
  B2INFO("    pxdDigit : " << npxdDigit[4]);
  B2INFO("  pxdDigitIn : " << npxdDigitInROI[4]);
  epsilon[4] = (double)npxdDigitInROI[4] / (double) npxdDigit[4];
  epsilonErr[4] = sqrt(epsilon[4] * (1 - epsilon[4]) / npxdDigit[4]);
  B2INFO("  efficiency : " << epsilon[4] << " +/- " << epsilonErr[4]);

  B2INFO("");
  B2INFO(" 0.3 < pT < 0.5");
  B2INFO("       no hit: " << nnoHit2[3] << " + " << nnoHit3[3]);
  B2INFO("       no ROI: " << nnoROI4[3] << " + " << nnoROI5[3]);
  B2INFO("    pxdDigit : " << npxdDigit[3]);
  B2INFO("  pxdDigitIn : " << npxdDigitInROI[3]);
  epsilon[3] = (double)npxdDigitInROI[3] / (double) npxdDigit[3];
  epsilonErr[3] = sqrt(epsilon[3] * (1 - epsilon[3]) / npxdDigit[3]);
  B2INFO("  efficiency : " << epsilon[3] << " +/- " << epsilonErr[3]);


  B2INFO("");
  B2INFO(" 0.2 < pT < 0.3");
  B2INFO("       no hit: " << nnoHit2[2] << " + " << nnoHit3[2]);
  B2INFO("       no ROI: " << nnoROI4[2] << " + " << nnoROI5[2]);
  B2INFO("    pxdDigit : " << npxdDigit[2]);
  B2INFO("  pxdDigitIn : " << npxdDigitInROI[2]);
  epsilon[2] = (double)npxdDigitInROI[2] / (double) npxdDigit[2];
  epsilonErr[2] = sqrt(epsilon[2] * (1 - epsilon[2]) / npxdDigit[2]);
  B2INFO("  efficiency : " << epsilon[2] << " +/- " << epsilonErr[2]);


  B2INFO("");
  B2INFO(" 0.1 < pT < 0.2");

  B2INFO("       no hit: " << nnoHit2[1] << " + " << nnoHit3[1]);
  B2INFO("       no ROI: " << nnoROI4[1] << " + " << nnoROI5[1]);
  B2INFO("    pxdDigit : " << npxdDigit[1]);
  B2INFO("  pxdDigitIn : " << npxdDigitInROI[1]);
  epsilon[1] = (double)npxdDigitInROI[1] / (double) npxdDigit[1];
  epsilonErr[1] = sqrt(epsilon[1] * (1 - epsilon[1]) / npxdDigit[1]);
  B2INFO("  efficiency : " << epsilon[1] << " +/- " << epsilonErr[1]);


  B2INFO("");
  B2INFO(" pT < 0.1");
  B2INFO("       no hit: " << nnoHit2[0] << " + " << nnoHit3[0]);
  B2INFO("       no ROI: " << nnoROI4[0] << " + " << nnoROI5[0]);
  B2INFO("    pxdDigit : " << npxdDigit[0]);
  B2INFO("  pxdDigitIn : " << npxdDigitInROI[0]);
  epsilon[0] = (double)npxdDigitInROI[0] / (double) npxdDigit[0];
  epsilonErr[0] = sqrt(epsilon[0] * (1 - epsilon[0]) / npxdDigit[0]);
  B2INFO("  efficiency : " << epsilon[0] << " +/- " << epsilonErr[0]);

  m_gEff = new TGraphErrors(6, pt, epsilon, ptErr, epsilonErr);
  m_gEff->SetName("g_eff");

  for (int i = 0; i < 6; i++) {
    m_h1digiOut2->SetBinContent(i + 1, nnoHit2[i]);
    m_h1digiOut3->SetBinContent(i + 1, nnoHit3[i]);
    m_h1digiOut4->SetBinContent(i + 1, nnoROI4[i]);
    m_h1digiOut5->SetBinContent(i + 1, nnoROI5[i]);
    m_h1digiIn->SetBinContent(i + 1, npxdDigitInROI[i]);
  }


  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd(); //important! without this the famework root I/O (SimpleOutput etc) could mix with the root I/O of this module
    m_gEff->Write();
    m_h1digiIn->Write();
    m_h1digiOut2->Write();
    m_h1digiOut3->Write();
    m_h1digiOut4->Write();
    m_h1digiOut5->Write();

    m_h1GlobalTime->Write();
    m_h1PullU->Write();
    m_h1PullV->Write();
    m_h1ResidU->Write();
    m_h1ResidV->Write();
    m_h1SigmaU->Write();
    m_h1SigmaV->Write();
    m_h1Phi->Write();
    m_h1Theta->Write();
    m_h1CosTheta->Write();
    m_h1Lambda->Write();
    m_h1totarea->Write();
    m_h1area->Write();
    m_h1nROIs->Write();
    m_h1nROIs_all->Write();
    m_h1totROIs->Write();
    m_h1redFactor->Write();
    m_h1pt->Write();

    m_h1GlobalTimeFail->Write();
    m_h1PullUFail->Write();
    m_h1PullVFail->Write();
    m_h1ResidUFail->Write();
    m_h1ResidVFail->Write();
    m_h1SigmaUFail->Write();
    m_h1SigmaVFail->Write();
    m_h1DistUFail->Write();
    m_h1DistVFail->Write();
    m_h2DistUVFail->Write();
    m_h1areaFail->Write();


    m_h1GlobalTimeNoROI->Write();
    m_h1PullUNoROI->Write();
    m_h1PullVNoROI->Write();
    m_h1ResidUNoROI->Write();
    m_h1ResidVNoROI->Write();
    m_h1SigmaUNoROI->Write();
    m_h1SigmaVNoROI->Write();

    m_h1PhiBad->Write();
    m_h1PhiBad_L1->Write();
    m_h1PhiBadLambda0_L2->Write();
    m_h1PhiBadLambda0_L1->Write();
    m_h1PhiBadLambdaF_L2->Write();
    m_h1PhiBadLambdaF_L1->Write();
    m_h1PhiBad_L2->Write();
    m_h1ThetaBad->Write();
    m_h1CosThetaBad->Write();
    m_h1LambdaBad->Write();
    m_h1LambdaBad_timeL1->Write();
    m_h1LambdaBad_timeG1->Write();
    m_h1CoorUBad->Write();
    m_h1CoorVBad->Write();
    m_h2CoorUVBad->Write();
    m_h1GlobalTimeBad->Write();
    m_h1ptBad->Write();

    m_h1CosThetaMCPart->Write();
    m_h1ptAll->Write();

    m_h2Map->Write();
    m_h2MapBad_L1->Write();
    m_h2MapBad_L2->Write();
    m_hNhits->Write();
    m_hNhitsBad->Write();

    m_rootFilePtr->Close();
  }



  cout << "****TrackCand wth no MCPart = " << m_nNoMCPart << endl;



}

