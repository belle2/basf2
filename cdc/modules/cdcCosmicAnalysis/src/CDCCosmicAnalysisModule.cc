/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "cdc/modules/cdcCosmicAnalysis/CDCCosmicAnalysisModule.h"
#include <framework/geometry/BFieldManager.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/RelationArray.h>

#include <Math/ProbFuncMathCore.h>
#include "algorithm"

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
  setDescription("Module for harvesting parameters of the two half (up/down) of a cosmic track for performance study");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("RecoTracksColName", m_recoTrackArrayName, "Name of collectrion hold RecoTracks", std::string(""));
  addParam("Output", m_outputFileName, "output file name", string("twotracks.root"));
  addParam("EventT0Extraction", m_eventT0Extraction, "use event t0 extract t0 or not", true);
  addParam("treeName", m_treeName, "Output tree name", string("tree"));
  addParam("phi0InRad", m_phi0InRad, "Phi0 in unit of radian, true: rad, false: deg", true);
  addParam("StoreTrackParErrors", m_storeTrackParErrors,
           "Store Track Parameter errors (true) or not (false)", false);
}

CDCCosmicAnalysisModule::~CDCCosmicAnalysisModule()
{
}
void CDCCosmicAnalysisModule::initialize()
{
  // Register histograms (calls back defineHisto)
  m_Tracks.isRequired(m_trackArrayName);
  m_RecoTracks.isRequired(m_recoTrackArrayName);
  m_TrackFitResults.isRequired(m_trackFitResultArrayName);
  RelationArray relRecoTrackTrack(m_RecoTracks, m_Tracks, m_relRecoTrackTrackName);

  m_relRecoTrackTrackName = relRecoTrackTrack.getName();

  tfile = new TFile(m_outputFileName.c_str(), "RECREATE");
  //  tree = new TTree("treeTrk", "treeTrk");
  tree = new TTree(m_treeName.c_str(), m_treeName.c_str());
  tree->Branch("run", &run, "run/I");
  tree->Branch("evtT0", &evtT0, "evtT0/D");
  tree->Branch("charge", &charge, "charge/S");

  tree->Branch("Pval1", &Pval1, "Pval1/D");
  tree->Branch("ndf1", &ndf1, "ndf1/D");
  tree->Branch("Pt1", &Pt1, "Pt1/D");
  tree->Branch("D01", &D01, "D01/D");
  tree->Branch("Phi01", &Phi01, "Phi01/D");
  //  tree->Branch("Om1", &Om1, "Om1/D");
  tree->Branch("Z01", &Z01, "Z01/D");
  tree->Branch("tanLambda1", &tanLambda1, "tanLambda1/D");
  tree->Branch("posSeed1", "TVector3", &posSeed1);
  tree->Branch("Omega1", &Omega1, "Omega1/D");
  tree->Branch("Mom1", "TVector3", &Mom1);

  tree->Branch("Pval2", &Pval2, "Pval2/D");
  tree->Branch("ndf2", &ndf2, "ndf2/D");
  tree->Branch("Pt2", &Pt2, "Pt2/D");
  tree->Branch("D02", &D02, "D02/D");
  tree->Branch("Phi02", &Phi02, "Phi02/D");
  //  tree->Branch("Om2", &Om2, "Om2/D");
  tree->Branch("Z02", &Z02, "Z02/D");
  tree->Branch("tanLambda2", &tanLambda2, "tanLambda2/D");
  tree->Branch("posSeed2", "TVector3", &posSeed2);
  tree->Branch("Omega2", &Omega2, "Omega2/D");
  tree->Branch("Mom2", "TVector3", &Mom2);

  if (m_storeTrackParErrors) {
    tree->Branch("eD01", &eD01, "eD01/D");
    tree->Branch("ePhi01", &ePhi01, "ePhi01/D");
    tree->Branch("eOm1", &eOm1, "eOm1/D");
    tree->Branch("eZ01", &eZ01, "eZ01/D");
    tree->Branch("etanL1", &etanL1, "etanL1/D");

    tree->Branch("eD02", &eD02, "eD02/D");
    tree->Branch("ePhi02", &ePhi02, "ePhi02/D");
    tree->Branch("eOm2", &eOm2, "eOm2/D");
    tree->Branch("eZ02", &eZ02, "eZ02/D");
    tree->Branch("etanL2", &etanL2, "etanL2/D");
  }

}

void CDCCosmicAnalysisModule::beginRun()
{
  B2Vector3D pos(0, 0, 0);
  B2Vector3D bfield = BFieldManager::getFieldInTesla(pos);
  if (bfield.Z() > 0.5) {
    m_bField = true;
    B2INFO("CDCCosmicAnalysis: Magnetic field is ON");
  } else {
    m_bField = false;
    B2INFO("CDCCosmicAnalysis: Magnetic field is OFF");
  }
  B2INFO("CDCCosmicAnalysis: BField at (0,0,0)  = " << bfield.Mag());
}

void CDCCosmicAnalysisModule::event()
{
  if (m_EventMetaData.isValid())
    run = m_EventMetaData->getRun();

  evtT0 = 0;
  if (m_eventT0Extraction) {
    // event with is fail to extract event-t0 will be excluded
    if (m_eventTimeStoreObject.isValid() && m_eventTimeStoreObject->hasEventT0()) {
      evtT0 =  m_eventTimeStoreObject->getEventT0();
    } else {
      return;
    }
  }

  // Loop over Tracks
  int nTr = m_Tracks.getEntries();

  short charge2 = 0;
  short charge1 = 0;
  bool up(false), down(false);
  for (int i = 0; i < nTr; ++i) {
    const Belle2::Track* b2track = m_Tracks[i];
    const Belle2::TrackFitResult* fitresult;
    fitresult = b2track->getTrackFitResult(Const::muon);
    if (!fitresult) {
      B2WARNING("There is no track fit result for muon hypothesis; try with the closest mass hypothesis...");
      fitresult = b2track->getTrackFitResultWithClosestMass(Const::muon);
      if (!fitresult) {
        B2WARNING("There is also no track fit reslt for the other mass hypothesis");
        continue;
      }
    }
    const Belle2::RecoTrack* recoTrack = b2track->getRelatedTo<Belle2::RecoTrack>(m_recoTrackArrayName);
    if (!recoTrack) {
      B2WARNING("Can not access RecoTrack of this Belle2::Track");
      continue;
    }

    TVector3 posSeed = recoTrack->getPositionSeed();
    const genfit::FitStatus* fs = recoTrack->getTrackFitStatus();

    double ndf = fs->getNdf();
    if (!m_bField)  // in case of no magnetic field, #track par=4 instead of 5.
      ndf += 1;

    double Chi2 = fs->getChi2();
    double TrPval = std::max(0., ROOT::Math::chisquared_cdf_c(Chi2, ndf));
    double Phi0 = fitresult->getPhi0();
    if (m_phi0InRad == false) { // unit of degrees.
      Phi0 *=  180 / M_PI;
    }

    /*** Two track case.***/
    if ((posSeed.Y() > 0 && !up) ||
        (up && posSeed.Y()*posSeed1.Y() > 0 &&  ndf > ndf1)) {
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
      eOm1 = sqrt(fitresult->getCovariance5()[2][2]);
      Z01 = fitresult->getZ0();
      eZ01 = sqrt(fitresult->getCovariance5()[3][3]);
      tanLambda1 = fitresult->getTanLambda();
      etanL1 = sqrt(fitresult->getCovariance5()[4][4]);
      Pt1 = fitresult->getTransverseMomentum();
      ndf1 = ndf;
      Pval1 = TrPval;
      up = true;
    }

    if ((posSeed.Y() < 0 && !down) ||
        (down && posSeed.Y()*posSeed2.Y() > 0 && ndf > ndf2)) {
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
      eOm2 = sqrt(fitresult->getCovariance5()[2][2]);
      Z02 = fitresult->getZ0();
      eZ02 = sqrt(fitresult->getCovariance5()[3][3]);
      tanLambda2 = fitresult->getTanLambda();
      etanL2 = sqrt(fitresult->getCovariance5()[4][4]);
      Pt2 = fitresult->getTransverseMomentum();
      ndf2 = ndf;
      Pval2 = TrPval;
      down = true;
    }
  }
  if (m_bField && charge1 * charge2 == 0)  return;
  if (charge1 * charge2 >= 0 && up && down) {
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


