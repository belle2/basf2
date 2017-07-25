/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: CDC Group                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "cdc/modules/cdcCosmicAnalysis/CDCCosmicAnalysisModule.h"
#include <framework/gearbox/Const.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/KalmanFitterInfo.h>
#include <set>
#include <boost/format.hpp>
#include <boost/foreach.hpp>

#include "TMath.h"
#include <Math/ProbFuncMathCore.h>
#include "iostream"
#include "TVector3.h"
using namespace std;
using namespace Belle2;
using namespace CDC;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCCosmicAnalysis)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCCosmicAnalysisModule::CDCCosmicAnalysisModule() : Module()
{
  setDescription("Module for save two tracks in cdc-top test");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("RecoTracksColName", m_recoTrackArrayName, "Name of collectrion hold genfit::Track", std::string(""));
  addParam("Output", m_OutputFileName, "xt file name", string("xt.root"));
  addParam("noBFit", m_noBFit, "If true -> #Params ==4, #params ==5 for calculate P-Val", true);
  addParam("EventT0Extraction", m_EventT0Extraction, "use event t0 extract t0 or not", false);
}

CDCCosmicAnalysisModule::~CDCCosmicAnalysisModule()
{
}

void CDCCosmicAnalysisModule::initialize()
{
  // Register histograms (calls back defineHisto)
  StoreArray<Belle2::Track> storeTrack(m_trackArrayName);
  StoreArray<RecoTrack> recoTracks(m_recoTrackArrayName);
  StoreArray<Belle2::TrackFitResult> storeTrackFitResults(m_trackFitResultArrayName);
  RelationArray relRecoTrackTrack(recoTracks, storeTrack, m_relRecoTrackTrackName);

  m_recoTrackArrayName = recoTracks.getName();
  m_trackFitResultArrayName = storeTrackFitResults.getName();
  m_relRecoTrackTrackName = relRecoTrackTrack.getName();

  tfile = new TFile(m_OutputFileName.c_str(), "RECREATE");
  tree = new TTree("tree", "tree");

  tree->Branch("evtT0", &evtT0, "evtT0/D");
  tree->Branch("charge", &charge, "charge/S");

  tree->Branch("Pval1", &Pval1, "Pval1/D");
  tree->Branch("ndf1", &ndf1, "ndf1/D");
  tree->Branch("Phi01", &Phi01, "Phi01/D");
  tree->Branch("tanLambda1", &tanLambda1, "tanLambda1/D");
  tree->Branch("D01", &D01, "D01/D");
  tree->Branch("Z01", &Z01, "Z01/D");
  tree->Branch("Omega1", &Omega1, "Omega1/D");
  tree->Branch("Mom1", "TVector3", &Mom1);
  tree->Branch("posSeed1", "TVector3", &posSeed1);

  tree->Branch("Pval2", &Pval2, "Pval2/D");
  tree->Branch("ndf2", &ndf2, "ndf2/D");
  tree->Branch("Phi02", &Phi02, "Phi02/D");
  tree->Branch("tanLambda2", &tanLambda2, "tanLambda2/D");
  tree->Branch("D02", &D02, "D02/D");
  tree->Branch("Z02", &Z02, "Z02/D");
  tree->Branch("Omega2", &Omega2, "Omega2/D");
  tree->Branch("Mom2", "TVector3", &Mom2);
  tree->Branch("posSeed2", "TVector3", &posSeed2);

}

void CDCCosmicAnalysisModule::beginRun()
{
}

void CDCCosmicAnalysisModule::event()
{
  const StoreArray<Belle2::Track> storeTrack(m_trackArrayName);
  const StoreArray<Belle2::TrackFitResult> storeTrackFitResults(m_trackFitResultArrayName);
  const StoreArray<Belle2::RecoTrack> recoTracks(m_recoTrackArrayName);
  const RelationArray relTrackTrack(recoTracks, storeTrack, m_relRecoTrackTrackName);

  //temporary add here, but should be outsidel
  bool store = true;
  evtT0 = 0;
  if (m_EventT0Extraction) {
    // event with is fail to extract t0 will be exclude from analysis
    if (m_eventTimeStoreObject.isValid() && m_eventTimeStoreObject->hasDoubleEventT0()) {
      evtT0 =  m_eventTimeStoreObject->getEventT0();
    } else {
      store = false;
    }
  }

  // Loop over Recotracks
  int nTr = recoTracks.getEntries();
  int nfitted = 0;
  int n = 0;
  short charge2 = 0;
  short charge1 = 0;

  for (int i = 0; i < nTr; ++i) {
    const RecoTrack* track = recoTracks[i];
    if (!track->getTrackFitStatus()->isFitted()) {
      m_fitstatus = 2;
      continue;
    }
    const genfit::FitStatus* fs = track->getTrackFitStatus();
    if (!fs || !fs->isFitConverged()) {//not fully convergence
      m_fitstatus = 1;
      continue;
    }
    m_fitstatus = 0;
    nfitted = nfitted + 1;

    /** find results in track fit results**/
    const Belle2::Track* b2track = track->getRelatedFrom<Belle2::Track>();
    if (!b2track) {B2DEBUG(99, "No relation found"); continue;}
    fitresult = b2track->getTrackFitResult(Const::muon);

    if (!fitresult) {
      B2WARNING("track was fitted but Relation not found");
      continue;
    }
    double ndf;
    if (m_noBFit) { // in case of no magnetic field, NDF=4 instead of 5.
      ndf = fs->getNdf() + 1;
    } else {
      ndf = fs->getNdf();
    }
    double Chi2 = fs->getChi2();
    double TrPval = std::max(0., ROOT::Math::chisquared_cdf_c(Chi2, ndf));
    double Phi0 = fitresult->getPhi0();
    TVector3 posSeed = track->getPositionSeed();

    /*** Two track case.***/
    if (nfitted == 1) {
      charge1 = fitresult->getChargeSign();
      posSeed1 = posSeed;
      Phi01 = Phi0;
      tanLambda1 = fitresult->getTanLambda();
      Z01 = fitresult->getZ0();
      D01 = fitresult->getD0();
      Omega1 = fitresult->getOmega();
      Mom1 = fitresult->getMomentum();

      ndf1 = ndf;
      Pval1 = TrPval;
      n += 1;
    }
    if (nfitted == 2) {
      charge2 = fitresult->getChargeSign();
      posSeed2 = posSeed;
      Phi02 = Phi0;
      tanLambda2 = fitresult->getTanLambda();
      Z02 = fitresult->getZ0();
      D02 = fitresult->getD0();
      ndf2 = ndf;
      Omega2 = fitresult->getOmega();
      Mom2 = fitresult->getMomentum();
      Pval2 = TrPval;
      n += 1;
    }
  }
  if (n == 2 && store && charge1 * charge2 > 0) {
    charge = charge1;
    tree->Fill();
  }
}

void CDCCosmicAnalysisModule::endRun()
{
}

void CDCCosmicAnalysisModule::terminate()
{
  tfile->cd();
  tree->Write();
  tfile->Close();
}
