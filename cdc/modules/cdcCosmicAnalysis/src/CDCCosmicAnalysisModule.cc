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
  addParam("RecoTracksColName", m_recoTrackArrayName, "Name of collectrion hold RecoTracks", std::string(""));
  addParam("Output", m_OutputFileName, "output file name", string("twotrack.root"));
  addParam("noBFit", m_noBFit, "If true -> #Params ==4, #params ==5 for calculate P-Val", true);
  addParam("EventT0Extraction", m_EventT0Extraction, "use event t0 extract t0 or not", false);
  addParam("treeName", m_treeName, "Output tree name", string("tree"));
  addParam("phi0InRad", m_phi0InRad, "Phi0 in unit of radian, true: rad, false: deg", true);
  addParam("qam", m_qam, "Output QAM histograms", false);
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
  //  tree = new TTree("treeTrk", "treeTrk");
  tree = new TTree(m_treeName.c_str(), m_treeName.c_str());
  tree->Branch("evtT0", &evtT0, "evtT0/D");
  tree->Branch("charge", &charge, "charge/S");
  tree->Branch("Pval1", &Pval1, "Pval1/D");
  tree->Branch("ndf1", &ndf1, "ndf1/D");
  tree->Branch("Pt1", &Pt1, "Pt1/D");
  tree->Branch("D01", &D01, "D01/D");
  tree->Branch("Phi01", &Phi01, "Phi01/D");
  tree->Branch("Om1", &Om1, "Om1/D");
  tree->Branch("Z01", &Z01, "Z01/D");
  tree->Branch("tanLambda1", &tanLambda1, "tanLambda1/D");
  tree->Branch("posSeed1", "TVector3", &posSeed1);
  tree->Branch("Omega1", &Omega1, "Omega1/D");
  tree->Branch("Mom1", "TVector3", &Mom1);

  tree->Branch("eD01", &eD01, "eD01/D");
  tree->Branch("ePhi01", &ePhi01, "ePhi01/D");
  tree->Branch("eOm1", &eOm1, "eOm1/D");
  tree->Branch("eZ01", &eZ01, "eZ01/D");
  tree->Branch("etanL1", &etanL1, "etanL1/D");
  tree->Branch("Pval2", &Pval2, "Pval2/D");
  tree->Branch("ndf2", &ndf2, "ndf2/D");
  tree->Branch("Pt2", &Pt2, "Pt2/D");
  tree->Branch("D02", &D02, "D02/D");
  tree->Branch("Phi02", &Phi02, "Phi02/D");
  tree->Branch("Om2", &Om2, "Om2/D");
  tree->Branch("Z02", &Z02, "Z02/D");
  tree->Branch("tanLambda2", &tanLambda2, "tanLambda2/D");
  tree->Branch("eD02", &eD02, "eD02/D");
  tree->Branch("ePhi02", &ePhi02, "ePhi02/D");
  tree->Branch("eOm2", &eOm2, "eOm2/D");
  tree->Branch("eZ02", &eZ02, "eZ02/D");
  tree->Branch("etanL2", &etanL2, "etanL2/D");
  tree->Branch("posSeed2", "TVector3", &posSeed2);
  tree->Branch("Omega2", &Omega2, "Omega2/D");
  tree->Branch("Mom2", "TVector3", &Mom2);

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
    if (m_eventTimeStoreObject.isValid() && m_eventTimeStoreObject->hasEventT0()) {
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
    if (m_phi0InRad == false) { // unit of degrees.
      Phi0 *=  180 / M_PI;
    }
    TVector3 posSeed = track->getPositionSeed();

    /*** Two track case.***/
    if (nfitted == 1) {
      charge1 = fitresult->getChargeSign();
      posSeed1 = posSeed;
      D01 = fitresult->getD0();
      eD01 = sqrt(fitresult->getCovariance5()[0][0]);
      Phi01 = Phi0;
      ePhi01 = sqrt(fitresult->getCovariance5()[1][1]);
      if (m_phi0InRad == false) { // unit of degrees.
        ePhi01 *=  180 / M_PI;
      }

      Omega1 = fitresult->getOmega();
      Mom1 = fitresult->getMomentum();
      Om1 = fitresult->getOmega();

      eOm1 = sqrt(fitresult->getCovariance5()[2][2]);
      Z01 = fitresult->getZ0();
      eZ01 = sqrt(fitresult->getCovariance5()[3][3]);
      tanLambda1 = fitresult->getTanLambda();
      etanL1 = sqrt(fitresult->getCovariance5()[4][4]);
      Pt1 = fitresult->getTransverseMomentum();
      ndf1 = ndf;
      Pval1 = TrPval;
      n += 1;
    }
    if (nfitted == 2) {
      charge2 = fitresult->getChargeSign();
      posSeed2 = posSeed;
      D02 = fitresult->getD0();
      eD02 = sqrt(fitresult->getCovariance5()[0][0]);
      Phi02 = Phi0;
      ePhi02 = sqrt(fitresult->getCovariance5()[1][1]);
      if (m_phi0InRad == false) { // unit of degrees.
        ePhi02 *=  180 / M_PI;
      }
      Omega2 = fitresult->getOmega();
      Mom2 = fitresult->getMomentum();
      Om2 = fitresult->getOmega();
      eOm2 = sqrt(fitresult->getCovariance5()[2][2]);
      Z02 = fitresult->getZ0();
      eZ02 = sqrt(fitresult->getCovariance5()[3][3]);
      tanLambda2 = fitresult->getTanLambda();
      etanL2 = sqrt(fitresult->getCovariance5()[4][4]);
      Pt2 = fitresult->getTransverseMomentum();
      ndf2 = ndf;
      Pval2 = TrPval;
      n += 1;
    }
  }

  if (n == 2 && store && charge1 * charge2 >= 0) {
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
  if (m_qam == true) {
    createQAMHist(tree);
  }
  tfile->Close();
}


