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
  addParam("RecoTracksColName", m_recoTrackArrayName, "Name of collectrion hold genfit::Track", std::string(""));
  addParam("Output", m_OutputFileName, "xt file name", string("xt.root"));
  addParam("noBFit", m_noBFit, "If true -> #Params ==4, #params ==5 for calculate P-Val", true);
  //  addParam("TriggerPos", m_TriggerPos, "Trigger position use for cut and reconstruct Trigger image", std::vector<double> { -0.6, -13.25, 17.3});
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
  tree = new TTree("treeTrk", "treeTrk");
  tree->Branch("Pval1", &Pval1, "Pval1/D");
  tree->Branch("ndf1", &ndf1, "ndf1/D");
  tree->Branch("Phi01", &Phi01, "Phi01/D");
  tree->Branch("tanLambda1", &tanLambda1, "tanLambda1/D");
  tree->Branch("D01", &D01, "D01/D");
  tree->Branch("Z01", &Z01, "Z01/D");

  tree->Branch("Pval2", &Pval2, "Pval2/D");
  tree->Branch("ndf2", &ndf2, "ndf2/D");
  tree->Branch("Phi02", &Phi02, "Phi02/D");
  tree->Branch("tanLambda2", &tanLambda2, "tanLambda2/D");
  tree->Branch("D02", &D02, "D02/D");
  tree->Branch("Z02", &Z02, "Z02/D");

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


  // Loop over Recotracks
  int nTr = recoTracks.getEntries();
  int nfitted = 0;
  int n = 0;
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
    if (m_noBFit) {ndf = fs->getNdf() + 1;} // incase no Magnetic field, NDF=4;
    else {ndf = fs->getNdf();}
    double Chi2 = fs->getChi2();
    double TrPval = std::max(0., ROOT::Math::chisquared_cdf_c(Chi2, ndf));
    double Phi0 = fitresult->getPhi0() * 180 / M_PI;

    /*** Two track case.***/
    if (nfitted == 1) {
      Phi01 = Phi0;
      tanLambda1 = fitresult->getTanLambda();
      Z01 = fitresult->getZ0();
      D01 = fitresult->getD0();
      ndf1 = ndf;
      Pval1 = TrPval;
      n += 1;
    }
    if (nfitted > 1 && Phi0 * Phi01 < 0) {
      Phi02 = Phi0;
      tanLambda2 = fitresult->getTanLambda();
      Z02 = fitresult->getZ0();
      D02 = fitresult->getD0();
      ndf2 = ndf;
      Pval2 = TrPval;
      n += 1;
    }
  }
  if (n == 2) tree->Fill();
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


TVector3 CDCCosmicAnalysisModule::getTriggerHitPosition(const Helix h, double yofcounter = -13.25)
{
  //  double yofcounter = -16.25+3.;
  double d0 = h.getD0();
  double z0 = h.getZ0();
  double tanlambda = h.getTanLambda();
  double phi0 = h.getPhi0();
  double l = (yofcounter + d0 * cos(phi0)) / sin(phi0);
  //  double t = (yofcounter -h.getPerigeeY()-d0*sin(phi0))/cos(phi0);
  double xofcounter = l * cos(phi0) + d0 * sin(phi0);
  double zofcounter = fma(l, tanlambda, z0);
  return TVector3(xofcounter, yofcounter, zofcounter);
}

