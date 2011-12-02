/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors:                                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <tracking/modules/trueHitTester/trueHitTesterModule.h>
//#include "RooDouble.h"


using namespace std;
using namespace Belle2;
using namespace boost::accumulators;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(trueHitTester)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

trueHitTesterModule::trueHitTesterModule() : Module()
{
//Set module properties
  setDescription("test the mutiple scattering with simulated true hits");

//Parameter definition
  addParam("outputFileName", m_dataOutFileName, "Output file name", string("tfc.txt"));

}


trueHitTesterModule::~trueHitTesterModule()
{

}

void trueHitTesterModule::initialize()
{

  m_nSiLayers = 2;
  m_nPxdLayers = 2;
  m_nSvdLayers = m_nSiLayers - m_nPxdLayers;
  m_deltaEsSiLayers.resize(m_nSiLayers);
  m_deltadudwsSiLayers.resize(m_nSiLayers);
  m_deltadvdwsSiLayers.resize(m_nSiLayers);

}


void trueHitTesterModule::beginRun()
{

}


void trueHitTesterModule::event()
{
//cerr << "===== event\n";
//StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
//m_dataOut << "event()" << "\n";
//simulated particles and hits
  StoreArray<MCParticle> aMcParticleArray("");
  int nMcParticles = aMcParticleArray.getEntries();
//cout << "nMcParticles " <<  nMcParticles << "\n";
//m_dataOut << "mcParticles " << aMcParticleArray->GetEntries() << "\n";
  StoreArray<PXDTrueHit> aPxdTrueHitArray("");
  int nPxdTrueHits = aPxdTrueHitArray.getEntries();
//cout << "nPxdTrueHits " <<  nPxdTrueHits << "\n";
  StoreArray<SVDTrueHit> aSvdTrueHitArray("");
  int nSvdTrueHits = aSvdTrueHitArray.getEntries();
//cout << "nSvdTrueHits " <<  nSvdTrueHits << "\n";
  RelationIndex<MCParticle, PXDTrueHit> relMcTrueHit;
  int sizeRelMcTrueHit = relMcTrueHit.size();
//cout << "sizeRelMcTrueHit " <<  sizeRelMcTrueHit << "\n";
//m_dataOut << "pxdSimHits " << aPxdSimHitArray->GetEntries() << "\n";
//int nSiSimHits = nPxdSimHits + nSvdSimHits;
//m_dataOut << "svdSimHits " << aSvdSimHitArray->GetEntries() << "\n";
//StoreArray<CDCSimHit> aCdcSimHitArray(m_cdcSimHitColName);
//m_dataOut << "cdcSimHits " << aCdcSimHitArray->GetEntries() << "\n";
//digitized hits
//StoreArray<PXDHit> aPxdHitArray(DEFAULT_PXDHITS);
//m_dataOut << "pxdHits " << aPxdHitArray->GetEntries() << "\n";
//StoreArray<SVDHit> aSvdHitArray(DEFAULT_SVDHITS);
//m_dataOut << "svdHits " << aSvdHitArray->GetEntries() << "\n";
//StoreArray<CDCHit> aCdcHitArray("CDCHits");
//m_dataOut << "cdcHits " << aCdcHitArray->GetEntries() << "\n";
//all relations
//StoreArray<Relation> MCPartToPXDSimHits(DEFAULT_PXDSIMHITSREL);
//m_dataOut << "mcParticle->pxdSimHit " << MCPartToPXDSimHits->GetEntries() << "\n";
//StoreArray<Relation> MCPartToSVDSimHits(DEFAULT_SVDSIMHITSREL);
//m_dataOut << "mcParticle->svdSimHit " << MCPartToSVDSimHits->GetEntries() << "\n";
//StoreArray<Relation> MCPartToCDCSimHits(DEFAULT_MCPART_TO_CDCSIMHITS);
//m_dataOut << "mcParticle->cdcSimHit " << MCPartToCDCSimHits->GetEntries() << "\n";




//VXD::GeoCache& aGeoCach = VXD::GeoCache::getInstance();



  /*StoreArray<GFTrack> fittedTracks("GFTracks");

  SiGeoCache* const geometry = SiGeoCache::instance();
  //cerr << "===== event2\n";
  double charge = aMcParticleArray[0]->getCharge();
  //StoreArray<RooDouble> pValueData("pValues");
  int nFittedTracks = fittedTracks.getEntries(); //number of fitted tracks in one event always 1 at the moment
  m_dataOut << "sollte 1 sein: " << fittedTracks.getEntries() << "\n";
  //unsigned int irep = 0;
  m_dataOut << "chi2totWerte: ";
  //cerr << "===== event3\n";
  for (int i = 0; i not_eq nFittedTracks; ++i) {
  GFTrack* aTrack = fittedTracks[i];
  //cerr << "===== event4\n";
  double chi2tot = aTrack->getChiSqu();
  m_dataOut << chi2tot << " ";
  int ndf = aTrack->getNDF();
  //int nHits = aTrack->getNumHits();
  //int nGFHits = ndf/2+5;// a hack because getNumHits() currently does not work
  double pValue = TMath::Prob(chi2tot, ndf); // actually the p value would be 1-TMath::Prob(chi2tot, ndf) but particle physicists want to have it this way.
  m_dataOut2 << chi2tot << '\t' << ndf << '\t' << pValue << '\n';
  //new(pValueData->AddrAt(i)) RooDouble(pValue);
  m_pValues(pValue);
  double absMom = aTrack->getMom().Mag(); //get momentum at current plane (should be the plane of the innermost hit)
  m_absMoms(absMom);
  m_absMomOut << absMom << "\n";
  if (m_testGeant3 == false){
  //cerr << "===== event5\n";
  TVector3 pos(0., 0., 0.); //origin
  TVector3 poca(0., 0., 0.); //point of closest approach
  TVector3 dirInPoca(0., 0., 0.); //direction of the track at the point of closest approach
  aTrack->getCardinalRep()->extrapolateToPoint(pos, poca,dirInPoca); //goto vertex postition (assume it is 0,0,0)
  GFDetPlane plane(poca, dirInPoca); //get plane through fitted vertex postition
  //double absMom = dirInPoca.Mag();
  double vertexAbsMom = aTrack->getMom(plane).Mag(); //get fitted momentum at fitted vertex
  m_vertexAbsMoms(vertexAbsMom);
  double x = poca.X();
  m_vertexX(x);
  double y = poca.Y();
  m_vertexY(y);
  double z = poca.Z();
  m_vertexZ(z);
  //TVector3 pos;
  //TVector3 mom;
  //TMatrixT<double> cov(6,6);
  //aTrack->getPosMomCov( pos, mom, cov);

  //now loop over all pxd and svd layers of one track an calculate smoothed standard scores

  //some variables needed in loop for pxd and svd
  TVector3 pLocalTrue;
  TMatrixT<double> smoothed_state;
  TMatrixT<double> smoothed_cov;
  TMatrixT<double> resTrue(5,1);//differents smoothed state vec minus true state vec
  TMatrixT<double> invCov; // will be filled with inverse of smoothed covarance matrix

  int hitLayerId = -1;
  bool correctTrackId;
  unsigned int iGFHit = 0;
  for (int iSiHit = 0; iSiHit not_eq nSiSimHits; ++iSiHit){

  if (iSiHit < nPxdSimHits){*/
  TMatrixT<double> trueState(5, 1);
  for (int iPart = 0; iPart not_eq nMcParticles; ++iPart) {
    const MCParticle* const aMcParticlePtr = aMcParticleArray[iPart];
    if (aMcParticlePtr->hasStatus(MCParticle::c_PrimaryParticle) == true) {
      //cout<< "primary particle" << "\n";

      double qTrue = aMcParticlePtr->getCharge();
      RelationIndex<MCParticle, PXDTrueHit>::range_from iterPair = relMcTrueHit.getFrom(aMcParticlePtr);
      while (iterPair.first not_eq iterPair.second) {
        const PXDTrueHit* const aSiTrueHitPtr = iterPair.first->to;
        float deltaE = aSiTrueHitPtr->getEnergyDep();
        int layerId = aSiTrueHitPtr->getSensorID().getLayer();
        int vecIndex = layerId - 1;
        m_deltaEsSiLayers[vecIndex].push_back(deltaE);
        //VxdID aSensorId = aSiTrueHitPtr->getSensorID();


        TVector3 pTrueIn = aSiTrueHitPtr->getEntryMomentum();
        TVector3 pTrueOut = aSiTrueHitPtr->getExitMomentum();
        TVector3 pTrue = aSiTrueHitPtr->getMomentum();
        //TVector3 deltaP = pTrueOut - pTrueIn;
        //pTrueIn.Print();
        //pTrueOut.Print();
        //deltaP.Print();
        //const VXD::SensorInfoBase& aCoordTrans = aGeoCach.getSensorInfo(aSensorId);
        //TVector3 pTrueInGlobal = aCoordTrans.vectorToGlobal(pTrueIn);
        //TVector3 pTrueOutGlobal = aCoordTrans.vectorToGlobal(pTrueOut);
        //pTrueInGlobal.Print();
        //pTrueOutGlobal.Print();
        //cout << "q,p " << qTrue << " "; pTrue.Print();
        //cout << "the others"; pTrueIn.Print(); pTrueOut.Print();

        /*trueState[0][0] = qTrue/pTrue.Mag();
        trueState[1][0] = pTrue[1]/pTrue[0];
        trueState[2][0] = pTrue[2]/pTrue[0];
        trueState[3][0] = aSiTrueHitPtr->getU();
        trueState[4][0] = aSiTrueHitPtr->getV();
        trueState.Print();*/
        m_deltadudwsSiLayers[vecIndex].push_back(pTrueOut[0] / pTrueOut[2] - pTrueIn[0] / pTrueIn[2]);
        m_deltadvdwsSiLayers[vecIndex].push_back(pTrueOut[1] / pTrueOut[2] - pTrueIn[1] / pTrueIn[2]);


        ++iterPair.first;
      }

    }

  }






  /*int trackId = aSiSimHitPtr->getTrackID();
  if (trackId == 1){
  hitLayerId = aSiSimHitPtr->getLayerID();
  int hitLadderId = aSiSimHitPtr->getLadderID();
  int hitSensorId = aSiSimHitPtr->getSensorID();
  pTrue = aSiSimHitPtr->getMomIn();
  geometry->masterToLocalVec(hitLayerId,hitLadderId,hitSensorId,pTrue,pLocalTrue);
  posInTrue = aSiSimHitPtr->getPosIn();
  posOutTrue = aSiSimHitPtr->getPosOut();
  correctTrackId = true;
  } else {
  correctTrackId = false;
  }*/
  /*
  } else {
  SVDSimHit* aSiSimHitPtr = aSvdSimHitArray[iSiHit-nPxdSimHits];
  int trackId = aSiSimHitPtr->getTrackID();
  if (trackId == 1){
  hitLayerId = aSiSimHitPtr->getLayerID();
  int hitLadderId = aSiSimHitPtr->getLadderID();
  int hitSensorId = aSiSimHitPtr->getSensorID();
  pTrue = aSiSimHitPtr->getMomIn();
  geometry->masterToLocalVec(hitLayerId,hitLadderId,hitSensorId,pTrue,pLocalTrue);
  posInTrue = aSiSimHitPtr->getPosIn();
  posOutTrue = aSiSimHitPtr->getPosOut();
  correctTrackId = true;
  } else {
  correctTrackId = false;
  }
  }
  if (correctTrackId == true){
  double qOverPTrue = charge/pTrue.Mag();
  ////XXX cout << "qOverPTrue "<< qOverPTrue << endl;
  double uTrue = (posInTrue[1] + posOutTrue[1])/2.0;
  ////XXX cout << "uTrue " << uTrue << endl;
  double vTrue = (posInTrue[2] + posOutTrue[2])/2.0;
  ////XXX cout << "vTrue " << vTrue << endl;
  double dudwTrue = pLocalTrue[1]/pLocalTrue[0];
  double dvdwTrue = pLocalTrue[2]/pLocalTrue[0];
  //XXX cout << "dudwTrue " << dudwTrue << " dvdwTrue " << dvdwTrue<< "\n";
  bool status = GFTools::getBiasedSmoothedData(aTrack, 0, iGFHit, smoothed_state, smoothed_cov);
  if (status == false){
  B2ERROR("GFTools::getBiasedSmoothedData called by trueHitTester reported an error. Event# " << eventMetaDataPtr->getEvent() << " Hit# " << iGFHit << "total chi2: " << chi2tot );
  ++m_failedSmootherCounter;
  ////cerr << "Track "; aTrack->Print(); //cerr << "hit# " << iGFHit; //cerr << " smoothed state";  smoothed_state.Print(); //cerr << "smoothed cov"; smoothed_cov.Print();
  } else {
  //cerr << "===== event7\n";
  resTrue[0][0] = smoothed_state[0][0] - qOverPTrue;
  resTrue[1][0] = smoothed_state[1][0] - dudwTrue;
  resTrue[2][0] = smoothed_state[2][0] - dvdwTrue;
  resTrue[3][0] = smoothed_state[3][0] - uTrue;
  resTrue[4][0] = smoothed_state[4][0] - vTrue;
  // now make chi2 = resTrue^T * invert(smoothed_cov) * resTrue
  TMatrixT<double> resTrueT(TMatrixT<double>::kTransposed, resTrue);

  GFTools::invertMatrix(smoothed_cov, invCov);

  double chi2 = (resTrueT*invCov*resTrue)[0][0]; //calculate the chi2 of the smoothed parameters with respect to the true values
  //XXX cout << "smoothed_state"; smoothed_state.Print();
  //XXX cout << "smoothed_cov"; smoothed_cov.Print();
  double z_qOverP = resTrue[0][0] / sqrt(smoothed_cov[0][0]);
  double z_dudw = resTrue[1][0] / sqrt(smoothed_cov[1][1]);
  double z_dvdw = resTrue[2][0]  / sqrt(smoothed_cov[2][2]);
  double z_u = resTrue[3][0] / sqrt(smoothed_cov[3][3]);
  double z_v = resTrue[4][0] / sqrt(smoothed_cov[4][4]);
  ////XXX cout << "all zs " << z_qOverP  << " " << z_u << " " << z_v << endl;
  int accuVecIndex = hitLayerId -1;
  m_z_qOverPsSiLayers[accuVecIndex](z_qOverP);
  m_z_dudwsSiLayers[accuVecIndex](z_dudw);
  m_z_dvdwsSiLayers[accuVecIndex](z_dvdw);
  m_z_usSiLayers[accuVecIndex](z_u);
  m_z_vsSiLayers[accuVecIndex](z_v);
  m_chi2sSiLayers[accuVecIndex](chi2);
  }
  ++iGFHit;
  }
  }
  }
  ++m_processedTracks;
  }
  m_dataOut << "\n";
  */
}

void trueHitTesterModule::endRun()
{
  /*  int nDigits = 4;
  //StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  if (m_failedSmootherCounter not_eq 0){
  B2WARNING("Smoothed states could not be extracted " << m_failedSmootherCounter << " times";)
  }
  if (m_processedTracks <= 1){
  B2WARNING("Only one or less events were processed. Statistics cannot be computed.");
  } else {
  //cout << "count m_pValues " << BA::count(m_pValues) << endl;
  double meanPValues = mean(m_pValues);
  double stdPValues = sqrt(variance(m_pValues));

  B2INFO("mean and std of p values from total chi2 of tracks. mean = " << meanPValues << " should be 0.5. std = " << stdPValues << " should be 0.288675");
  double meanAbsMom = mean(m_absMoms);
  //int Y = count(m_pValues);
  double stdAbsMom = sqrt(variance(m_absMoms));
  if (m_testGeant3 == false){
  B2INFO("mean of absolute track momentum at the innermost hit = " << meanAbsMom << " (only makes sence when all tracks have the same momentum) and std " << stdAbsMom);
  double meanVetexAbsMom = mean(m_vertexAbsMoms);
  double stdVertexAbsMom = sqrt(variance(m_vertexAbsMoms));
  B2INFO("mean of absolute track momentum at the vertex = " << meanVetexAbsMom << " (only makes sence when all tracks have the same momentum) and std " << stdVertexAbsMom);
  double meanX = mean(m_vertexX);
  double meanY = mean(m_vertexY);
  double meanZ = mean(m_vertexZ);
  B2INFO("mean of vertex position: " << meanX << " " << meanY << " " << meanZ << " ");
  B2INFO("standard scores and χ²s: q/p,\tdudw,\tdvdw,\tu,\tv,\tχ²,\t#");

  for (int l = 0; l not_eq m_nSiLayers; ++l){
  //cout << "l: " << l+1 << " count m_z_qOverPsSiLayers " << BA::count(m_z_qOverPsSiLayers[l]) << " count m_z_dudwsSiLayers: " << BA::count(m_z_dudwsSiLayers[l]) << " count m_z_dvdwsSiLayers: " << BA::count(m_z_dvdwsSiLayers[l]) << " count m_z_usSiLayers: " << BA::count(m_z_usSiLayers[l]) << " count m_z_vsSiLayers: " << BA::count(m_z_vsSiLayers[l]) << endl;
  double mean_z_qOverP = mean(m_z_qOverPsSiLayers[l]);
  double std_z_qOverP = sqrt(variance(m_z_qOverPsSiLayers[l]));
  double mean_z_dudw = mean(m_z_dudwsSiLayers[l]);
  double std_z_dudw = sqrt(variance(m_z_dudwsSiLayers[l]));
  double mean_z_dvdw = mean(m_z_dvdwsSiLayers[l]);
  double std_z_dvdw = sqrt(variance(m_z_dvdwsSiLayers[l]));
  double mean_z_u = mean(m_z_usSiLayers[l]);
  double std_z_u = sqrt(variance(m_z_usSiLayers[l]));
  double mean_z_v = mean(m_z_vsSiLayers[l]);
  double std_z_v = sqrt(variance(m_z_vsSiLayers[l]));
  double mean_chi2 = mean(m_chi2sSiLayers[l]);
  double std_chi2 = sqrt(variance(m_chi2sSiLayers[l]));
  //size_t
  B2INFO("layer: " <<l<< ", mean\t"<< fixed<<setprecision(nDigits) << mean_z_qOverP  <<"\t" << mean_z_dudw  <<"\t" << mean_z_dvdw  <<"\t" << mean_z_u<<"\t" << mean_z_v<<"\t" <<  mean_chi2);
  B2INFO("layer: " <<l<<", std\t"<< fixed<<setprecision(nDigits) << std_z_qOverP <<"\t" << std_z_dudw  <<"\t" << std_z_dvdw <<"\t" << std_z_u<<"\t" << std_z_v <<"\t" <<  std_chi2);
  }
  }
  }
  */
  /*
  vector<accumulator_set<float, stats< tag::mean, tag::variance(lazy)> > > m_deltaEsSiLayersStats(m_nSiLayers)
  for (int l = 0; l not_eq m_nSiLayers; ++l){
    std::for_each( m_deltaEsSiLayers[l].begin(), m_deltaEsSiLayers[l].end(), bind<void>( boost::ref(m_deltaEsSiLayersStats[l]), _1 ) );

  }
  */




//   StoreArray<MscAndEnLossData> mscDeltaEData("mscDeltaEDataAllLayers", DataStore::c_Run);
//
//   new(mscDeltaEData->AddrAt(0)) MscAndEnLossData();
//
//
//   //SimpleVec<float> aSimpleVec( m_deltaEsSiLayers[l]);
//
//   mscDeltaEData[0]->m_deltaEVecl1 = m_deltaEsSiLayers[0];
//   cout << mscDeltaEData[0]->m_deltaEVecl1.size() << "\n";
//   mscDeltaEData[0]->m_deltaEVecl2 = m_deltaEsSiLayers[1];
//
//   mscDeltaEData[0]->m_deltadudwVecl1 = m_deltadudwsSiLayers[0];
//   mscDeltaEData[0]->m_deltadudwVecl2 = m_deltadudwsSiLayers[1];
//   mscDeltaEData[0]->m_deltadvdwVecl1 = m_deltadvdwsSiLayers[0];
//   mscDeltaEData[0]->m_deltadvdwVecl2 = m_deltadvdwsSiLayers[1];


}


void trueHitTesterModule::terminate()
{

}
