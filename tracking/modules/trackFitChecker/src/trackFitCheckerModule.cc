/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFitChecker/trackFitCheckerModule.h>
//#include "RooDouble.h"


using namespace std;
using namespace Belle2;
using namespace boost::accumulators;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(trackFitChecker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

trackFitCheckerModule::trackFitCheckerModule() : Module()
{
  //Set module properties
  setDescription("trackFitCheckerMod module");

  //Parameter definition
  addParam("outputFileName", m_dataOutFileName, "Output file name", string("tfc.txt"));
  addParam("outputFileName2", m_dataOutFileName2, "Output file name2", string("tfcpvalues.txt"));
  addParam("cdcSimColName", m_cdcSimHitColName, "Name of collection of simulated CDC hits", string(DEFAULT_CDCSIMHITS));
  addParam("pxdSimColName", m_pxdSimHitColName, "Name of collection of simulated PXD hits", string(DEFAULT_PXDSIMHITS));
  addParam("svdSimColName", m_svdSimHitColName, "Name of collection of simulated SVD hits", string(DEFAULT_SVDSIMHITS));
  addParam("mcParticleColName", m_mcParticleColName, "Name of collection of particles created py pgun", string(DEFAULT_MCPARTICLES));
  addParam("fittedTracksColName", m_fittedTracksName, "Name of collection holding the results of Genfit", string("fittedTracks"));
}


trackFitCheckerModule::~trackFitCheckerModule()
{

}

void trackFitCheckerModule::initialize()
{
  m_dataOut.open(m_dataOutFileName.c_str());
  m_dataOut2.open(m_dataOutFileName2.c_str());

}


void trackFitCheckerModule::beginRun()
{

}


void trackFitCheckerModule::event()
{
  m_dataOut << "event()" << "\n";
  //simulated particles and hits
  StoreArray<MCParticle> aMcParticleArray(DEFAULT_MCPARTICLES);
  m_dataOut << "mcParticles " << aMcParticleArray->GetEntries() << "\n";
  StoreArray<PXDSimHit> aPxdSimHitArray(m_pxdSimHitColName.c_str());
  m_dataOut << "pxdSimHits " << aPxdSimHitArray->GetEntries() << "\n";
  StoreArray<SVDSimHit> aSvdSimHitArray(m_svdSimHitColName.c_str());
  m_dataOut << "svdSimHits " << aSvdSimHitArray->GetEntries() << "\n";
  StoreArray<CDCSimHit> aCdcSimHitArray(m_cdcSimHitColName.c_str());
  m_dataOut << "cdcSimHits " << aCdcSimHitArray->GetEntries() << "\n";
  //digitized hits
  StoreArray<PXDHit> aPxdHitArray(DEFAULT_PXDHITS);
  m_dataOut << "pxdHits " << aPxdHitArray->GetEntries() << "\n";
  StoreArray<SVDHit> aSvdHitArray(DEFAULT_SVDHITS);
  m_dataOut << "svdHits " << aSvdHitArray->GetEntries() << "\n";
  StoreArray<CDCHit> aCdcHitArray("CDCHits");
  m_dataOut << "cdcHits " << aCdcHitArray->GetEntries() << "\n";
  //all relations
  StoreArray<Relation> MCPartToPXDSimHits(DEFAULT_PXDSIMHITSREL);
  m_dataOut << "mcParticle->pxdSimHit " << MCPartToPXDSimHits->GetEntries() << "\n";
  StoreArray<Relation> MCPartToSVDSimHits(DEFAULT_SVDSIMHITSREL);
  m_dataOut << "mcParticle->svdSimHit " << MCPartToSVDSimHits->GetEntries() << "\n";
  StoreArray<Relation> MCPartToCDCSimHits(DEFAULT_MCPART_TO_CDCSIMHITS);
  m_dataOut << "mcParticle->cdcSimHit " << MCPartToCDCSimHits->GetEntries() << "\n";

  StoreArray<GFTrack> fittedTracks("fittedTracks");


  //StoreArray<RooDouble> pValueData("pValues");
  int nFittedTracks = fittedTracks.getEntries(); //number of fitted tracks in one event always 1 at the moment
  m_dataOut << "sollte 1 sein: " << fittedTracks.getEntries() << "\n";
  //unsigned int irep = 0;
  m_dataOut << "chi2totWerte: ";
  for (int i = 0; i not_eq nFittedTracks; ++i) {
    GFTrack* aTrack = fittedTracks[i];
    //int nHits = aTrack->getNumHits();
    double chi2tot = aTrack->getChiSqu();
    m_dataOut << chi2tot << " ";
    int ndf = aTrack->getNDF();
    double pValue = 1.0 - TMath::Prob(chi2tot, ndf);
    m_dataOut2 << chi2tot << '\t' << ndf << '\t' << pValue << '\n';
    //new(pValueData->AddrAt(i)) RooDouble(pValue);
    m_pValues(pValue);
    double absMom = aTrack->getMom().Mag();
    m_absMoms(absMom);
    double x = aTrack->getPos().X();
    m_vertexX(x);
    double y = aTrack->getPos().Y();
    m_vertexY(y);
    double z = aTrack->getPos().Z();
    m_vertexZ(z);

    //TVector3 pos;
    //TVector3 mom;
    //TMatrixT<double> cov(6,6);
    //aTrack->getPosMomCov( pos, mom, cov);


  }
  m_dataOut << "\n";
}

void trackFitCheckerModule::endRun()
{
  double meanPValues = mean(m_pValues);
  double stdPValues = sqrt(variance(m_pValues));

  B2INFO("mean and std of p values from total chi2 of tracks. mean = " << meanPValues << " should be 0.5. std = " << stdPValues << " should be 0.288675");
  double meanAbsMom = mean(m_absMoms);
  B2INFO("mean of absolut of intitial track momentum = " << meanAbsMom << " (only makes sence when all tracks have the same momentum)");
  double meanX = mean(m_vertexX);
  double meanY = mean(m_vertexY);
  double meanZ = mean(m_vertexZ);
  B2INFO("mean of vertex postition: " << meanX << " " << meanY << " " << meanZ << " ");

}


void trackFitCheckerModule::terminate()
{
  m_dataOut2.close();
  m_dataOut.close();
}
