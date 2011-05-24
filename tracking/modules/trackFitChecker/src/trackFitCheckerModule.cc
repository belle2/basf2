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

#include <numeric>

using namespace std;
using namespace Belle2;

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
  addParam("outputFileName", m_dataOutFileName, "Output file name", string("fuckingShit.txt"));
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
  m_dataOut.open("trackFitCheckerTests.txt");

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
  B2INFO("sollte 1 sein: " << fittedTracks.getEntries())

  int nFittedTracks = fittedTracks.getEntries(); //number of fitted tracks in one event always 1 at the moment

  for (int i = 0; i not_eq nFittedTracks; ++i) {
    GFTrack* aTrack = fittedTracks[i];
    int nHits = aTrack->getNumHits();
    TMatrixT<double> smoothed_state;
    TMatrixT<double> smoothed_cov;
    double chi2tot = aTrack->getChiSqu();
    double ndf = aTrack->getNDF();
    double pValue = TMath::Prob(chi2tot, ndf);
    m_pValues.push_back(pValue);
    /*for ( int j = 0; j not_eq nHits; ++j ){
      bool status = GFTools::getSmoothedData(aTrack, 0, j, smoothed_state, smoothed_cov);
      B2INFO("did getSmoothedData work? " << status << " was it j? " << j);
      smoothed_state.Print();
      smoothed_cov.Print();
    }*/
  }




}


void trackFitCheckerModule::endRun()
{
  vector<double>::size_type nPValues = m_pValues.size();
  double meanPValues = accumulate(m_pValues.begin(), m_pValues.end(), 0.0) / double(nPValues);
  double stdPValues = 0.0;
  for (int i = 0; i not_eq nPValues; ++i) {
    double diffSq = m_pValues[i] - meanPValues;
    diffSq = diffSq * diffSq;
    stdPValues += diffSq;
  }
  stdPValues = sqrt(stdPValues) / (nPValues - 1);
  //B2INFO("mean and std of p values from total chi2 of tracks. mean = " << meanPValues << " should be 0.5. std = " << stdPValues << "should be 0.288675" );

  cout << "mean and std of p values from total chi2 of tracks. mean = " << meanPValues << " should be 0.5. std = " << stdPValues << "should be 0.288675" << endl;
}


void trackFitCheckerModule::terminate()
{

}
