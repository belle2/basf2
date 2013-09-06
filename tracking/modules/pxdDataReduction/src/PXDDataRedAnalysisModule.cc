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

#include <GFTrackCand.h>
#include <generators/dataobjects/MCParticle.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <tracking/dataobjects/ROIid.h>
#include <tracking/dataobjects/PXDIntercept.h>
#include <list>
#include <iostream>
#include <TVector3.h>



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
  , n_pxdDigit(0)
  , n_pxdDigitInROI(0)
  , n_noIntercept(0)
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

  StoreArray<GFTrackCand>::required(m_gfTrackCandsColName);
  StoreArray<ROIid>::required(m_ROIListName);
  StoreArray<PXDIntercept>::required(m_PXDInterceptListName);

  StoreArray<MCParticle>::required();


  n_tracks         = 0;
  n_pxdDigit       = 0;
  n_pxdDigitInROI  = 0;


  if (m_writeToRoot == true) {
    m_rootFileName += ".root";
    m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

    m_rootNoHitTreePtr = new TTree("nohit", "no Hit tree");
    m_rootNoHitTreePtr->Branch("coorU", &m_rootNoHitCoorU);
    m_rootNoHitTreePtr->Branch("coorV", &m_rootNoHitCoorV);
    m_rootNoHitTreePtr->Branch("sigmaU", &m_rootNoHitSigmaU);
    m_rootNoHitTreePtr->Branch("sigmaV", &m_rootNoHitSigmaV);
    m_rootNoHitTreePtr->Branch("lambda", &m_rootNoHitLambda);
    m_rootNoHitTreePtr->Branch("vxdID", &m_rootNoHitVxdID);
    m_rootNoHitTreePtr->Branch("globalTime", &m_rootGlobalTime);
    m_rootNoHitTreePtr->Branch("nGlobalTime", &m_rootNGlobalTime);
    m_rootNoHitTreePtr->Branch("momXmc", &m_rootMomXmc);
    m_rootNoHitTreePtr->Branch("momYmc", &m_rootMomYmc);
    m_rootNoHitTreePtr->Branch("momZmc", &m_rootMomZmc);
    m_rootNoHitTreePtr->Branch("coorUmc", &m_rootCoorUmc);
    m_rootNoHitTreePtr->Branch("coorVmc", &m_rootCoorVmc);
    m_rootNoHitTreePtr->Branch("Uidmc", &m_rootUidmc);
    m_rootNoHitTreePtr->Branch("Vidmc", &m_rootVidmc);
    m_rootNoHitTreePtr->Branch("vxdIDmc", &m_rootVxdIDmc);

    m_rootHitTreePtr = new TTree("hit", "Hit tree");
    m_rootHitTreePtr->Branch("coorU", &m_rootHitCoorU);
    m_rootHitTreePtr->Branch("coorV", &m_rootHitCoorV);
    m_rootHitTreePtr->Branch("sigmaU", &m_rootHitSigmaU);
    m_rootHitTreePtr->Branch("sigmaV", &m_rootHitSigmaV);
    m_rootHitTreePtr->Branch("lambda", &m_rootHitLambda);
    m_rootHitTreePtr->Branch("vxdID", &m_rootHitVxdID);
    m_rootHitTreePtr->Branch("globalTime", &m_rootGlobalTime);
    m_rootHitTreePtr->Branch("nGlobalTime", &m_rootNGlobalTime);
    m_rootHitTreePtr->Branch("momXmc", &m_rootMomXmc);
    m_rootHitTreePtr->Branch("momYmc", &m_rootMomYmc);
    m_rootHitTreePtr->Branch("momZmc", &m_rootMomZmc);
    m_rootHitTreePtr->Branch("coorUmc", &m_rootCoorUmc);
    m_rootHitTreePtr->Branch("coorVmc", &m_rootCoorVmc);
    m_rootHitTreePtr->Branch("Uidmc", &m_rootUidmc);
    m_rootHitTreePtr->Branch("Vidmc", &m_rootVidmc);
    m_rootHitTreePtr->Branch("vxdIDmc", &m_rootVxdIDmc);
    m_rootHitTreePtr->Branch("minUidroi", &m_rootROIminUid);
    m_rootHitTreePtr->Branch("minVidroi", &m_rootROIminVid);
    m_rootHitTreePtr->Branch("maxUidroi", &m_rootROImaxUid);
    m_rootHitTreePtr->Branch("maxVidroi", &m_rootROImaxVid);
    m_rootHitTreePtr->Branch("vxdIDroi", &m_rootROIVxdID);

    m_rootNoInterTreePtr = new TTree("nointer", "NoInter tree");
    m_rootNoInterTreePtr->Branch("momXmc", &m_rootMomXmc);
    m_rootNoInterTreePtr->Branch("momYmc", &m_rootMomYmc);
    m_rootNoInterTreePtr->Branch("momZmc", &m_rootMomZmc);
    m_rootNoInterTreePtr->Branch("coorUmc", &m_rootCoorUmc);
    m_rootNoInterTreePtr->Branch("coorVmc", &m_rootCoorVmc);
    m_rootNoInterTreePtr->Branch("Uidmc", &m_rootUidmc);
    m_rootNoInterTreePtr->Branch("Vidmc", &m_rootVidmc);
    m_rootNoInterTreePtr->Branch("vxdIDmc", &m_rootVxdIDmc);
  } else {
    m_rootFilePtr = NULL;
    m_rootNoHitTreePtr = NULL;
    m_rootHitTreePtr = NULL;
    m_rootNoInterTreePtr = NULL;
  }

}

void PXDDataRedAnalysisModule::beginRun()
{
  m_rootEvent = 0;
}


void PXDDataRedAnalysisModule::event()
{

  B2DEBUG(1, "  ++++++++++++++ PXDDataRedAnalysisModule");
  StoreArray<GFTrackCand> trackCandList(m_gfTrackCandsColName);
  StoreArray<PXDIntercept> PXDInterceptList(m_PXDInterceptListName);
  StoreArray<ROIid> ROIList(m_ROIListName);


  StoreArray<MCParticle> mcParticles;

  StoreArray<PXDDigit> pxdDigits;
  StoreArray<PXDSimHit> pxdSimHits;
  StoreArray<PXDTrueHit> pxdTrueHits;

  RelationIndex < GFTrackCand, PXDIntercept >
  gfTrackCandToPXDIntercept(DataStore::relationName(m_gfTrackCandsColName, m_PXDInterceptListName));

  if (! gfTrackCandToPXDIntercept)
    B2FATAL("No GFTrackCand -> PXDIntercept relation found! :'(");

  typedef RelationIndex < GFTrackCand, PXDIntercept>::range_from PXDInterceptsFromGFTracks;
  typedef RelationIndex < GFTrackCand, PXDIntercept>::iterator_from PXDInterceptIteratorType;

  typedef RelationIndex < PXDDigit, PXDTrueHit>::range_from PXDTrueHitFromPXDDigit;
  typedef RelationIndex < PXDDigit, PXDTrueHit>::iterator_from PXDTrueHitIteratorType;

  RelationIndex < PXDDigit, PXDTrueHit > relDigitTrueHit(DataStore::relationName(
                                                           DataStore::arrayName<PXDDigit>(""),
                                                           DataStore::arrayName<PXDTrueHit>(""))
                                                        );
  double tmpGlobalTime;

  for (int i = 0; i < trackCandList.getEntries(); i++) {

    n_tracks ++;

    int McId = trackCandList[i]->getMcTrackId();

    if (McId < 0)
      continue;

    MCParticle* aMcParticle = mcParticles[McId];

    RelationVector<PXDDigit> pxdRelations = aMcParticle->getRelationsFrom<PXDDigit>();

    PXDInterceptsFromGFTracks  PXDIntercepts = gfTrackCandToPXDIntercept.getElementsFrom(trackCandList[i]);

    m_rootMomXmc = (aMcParticle->getMomentum()).X();
    m_rootMomYmc = (aMcParticle->getMomentum()).Y();
    m_rootMomZmc = (aMcParticle->getMomentum()).Z();

    for (unsigned int iPXDDigit = 0; iPXDDigit < pxdRelations.size(); iPXDDigit++) {

      n_pxdDigit ++ ;

      PXDTrueHitFromPXDDigit  PXDTrueHits = relDigitTrueHit.getElementsFrom(*pxdRelations[iPXDDigit]);
      PXDTrueHitIteratorType thePXDTrueHitIterator = PXDTrueHits.begin();
      PXDTrueHitIteratorType thePXDTrueHitIteratorEnd = PXDTrueHits.end();
      tmpGlobalTime = 0;
      m_rootNGlobalTime = 0;
      for (; thePXDTrueHitIterator != thePXDTrueHitIteratorEnd; thePXDTrueHitIterator++) {
        tmpGlobalTime = tmpGlobalTime + thePXDTrueHitIterator->to->getGlobalTime();
        m_rootNGlobalTime++;
      }
      m_rootGlobalTime = tmpGlobalTime / m_rootNGlobalTime;

      m_rootCoorUmc = pxdRelations[iPXDDigit]->getUCellPosition();
      m_rootCoorVmc = pxdRelations[iPXDDigit]->getVCellPosition();
      m_rootUidmc = pxdRelations[iPXDDigit]->getUCellID();
      m_rootVidmc = pxdRelations[iPXDDigit]->getVCellID();
      m_rootVxdIDmc = pxdRelations[iPXDDigit]->getSensorID();

      PXDInterceptIteratorType thePXDInterceptIterator = PXDIntercepts.begin();
      PXDInterceptIteratorType thePXDInterceptIteratorEnd = PXDIntercepts.end();
      bool MissingHit(true);

      for (; thePXDInterceptIterator != thePXDInterceptIteratorEnd; thePXDInterceptIterator++) {

        const PXDIntercept* theIntercept = thePXDInterceptIterator->to;

        if (theIntercept) {

          m_rootNoHitCoorU = theIntercept->getCoorU();
          m_rootNoHitCoorV = theIntercept->getCoorV();
          m_rootNoHitSigmaU = theIntercept->getSigmaU();
          m_rootNoHitSigmaV = theIntercept->getSigmaV();
          m_rootNoHitLambda = theIntercept->getLambda();

          m_rootNoHitVxdID = theIntercept->getSensorID();

          const ROIid* theROIid = theIntercept->getRelatedTo<ROIid>(m_ROIListName);

          if (theROIid) {
            if (theROIid->Contains(*(pxdRelations[iPXDDigit]))) {

              m_rootHitCoorU = theIntercept->getCoorU();
              m_rootHitCoorV = theIntercept->getCoorV();
              m_rootHitSigmaU = theIntercept->getSigmaU();
              m_rootHitSigmaV = theIntercept->getSigmaV();
              m_rootHitLambda = theIntercept->getLambda();

              m_rootHitVxdID = theIntercept->getSensorID();
              m_rootROIminUid =  theROIid->getMinUid();
              m_rootROImaxUid =  theROIid->getMaxUid();
              m_rootROIminVid =  theROIid->getMinVid();
              m_rootROImaxVid =  theROIid->getMaxVid();
              m_rootROIVxdID =  theROIid->getSensorID();

              m_rootHitTreePtr->Fill();

              MissingHit = false;

              n_pxdDigitInROI ++;

              break; // To avoid double counting
            }
          }
        } else {
          //    cout << "I";
          m_rootNoInterTreePtr->Fill();
          n_noIntercept++;
        }

      }

      if (MissingHit) {
        //  cout << "%";
        m_rootNoHitTreePtr->Fill();
        n_noHit ++;
      }
      // else
      //  cout << "~";

    }
    //    cout << endl;

  }

  m_rootEvent++;

}


void PXDDataRedAnalysisModule::endRun()
{
}


void PXDDataRedAnalysisModule::terminate()
{

  if (m_rootNoHitTreePtr != NULL) {
    m_rootFilePtr->cd(); //important! without this the famework root I/O (SimpleOutput etc) could mix with the root I/O of this module
    m_rootNoInterTreePtr->Write();
    m_rootNoHitTreePtr->Write();
    m_rootHitTreePtr->Write();
    m_rootFilePtr->Close();
  }

  B2INFO("     ROI Analysis Summary     ");
  B2INFO("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  B2INFO("~ Efficiency estimated from  ~");
  B2INFO("      tracks : " << n_tracks);
  B2INFO("no intercepts: " << n_noIntercept);
  B2INFO("       no hit: " << n_noHit);
  B2INFO("    pxdDigit : " << n_pxdDigit);
  B2INFO("  pxdDigitIn : " << n_pxdDigitInROI);
  B2INFO("  efficiency : " << (double)n_pxdDigitInROI / (double) n_pxdDigit);
  B2INFO("intercept ineff: " << (double)n_noIntercept / (double) n_pxdDigit);

}

