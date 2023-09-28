/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "cdc/modules/cdcCalibrationCollector/CDCFudgeFactorCalibrationCollector.h"
#include "analysis/utility/PCmsLabTransform.h"
#include <framework/dataobjects/Helix.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dbobjects/BeamSpot.h>
#include <Math/ProbFuncMathCore.h>

#include <TH1F.h>
#include <TH2F.h>
#include <TMath.h>

using namespace std;
using namespace Belle2;
using namespace CDC;

REG_MODULE(CDCFudgeFactorCalibrationCollector);

CDCFudgeFactorCalibrationCollectorModule::CDCFudgeFactorCalibrationCollectorModule() : CalibrationCollectorModule()
{
  setDescription("Collector module for cdc fudege calibration");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("StoreNtuple", m_StoreNtuple, "Store ntuple other studies", true);
  addParam("MinColinearityTheta", m_minCollinearityTheta, "cut on colinear of the two track by theta", 10.);
  addParam("MinColinearityPhi0",  m_minCollinearityPhi0, "cut on colinear of the two track by phi0", 10.);

}

CDCFudgeFactorCalibrationCollectorModule::~CDCFudgeFactorCalibrationCollectorModule()
{
}

void CDCFudgeFactorCalibrationCollectorModule::prepare()
{
  m_Tracks.isRequired(m_trackArrayName);
  //  m_Vpho_list.isRequired(m_PlistName);
  m_TrackFitResults.isRequired(m_trackFitResultArrayName);

  if (m_StoreNtuple) {
    auto m_tree  = new TTree(m_treeName.c_str(), "tree for cdc calibration");
    m_tree->Branch<Float_t>("pt_pos", &Pt_pos);
    m_tree->Branch<Int_t>("exp_run", &exp_run);
    m_tree->Branch<Float_t>("pt_neg", &Pt_neg);
    m_tree->Branch<Float_t>("pt_pos_cm", &Pt_pos_cm);
    m_tree->Branch<Float_t>("pt_neg_cm", &Pt_neg_cm);
    m_tree->Branch<Float_t>("theta_pos_cm", &Theta_pos_cm);
    m_tree->Branch<Float_t>("theta_neg_cm", &Theta_neg_cm);
    m_tree->Branch<Float_t>("phi0_pos_cm",  &Phi0_pos_cm);
    m_tree->Branch<Float_t>("theta_neg_cm", &Phi0_neg_cm);

    m_tree->Branch<Float_t>("Pval_pos", &Pval_pos);
    m_tree->Branch<Float_t>("Pval_neg", &Pval_neg);
    m_tree->Branch<Float_t>("ndf_pos", &ndf_pos);
    m_tree->Branch<Float_t>("ndf_neg", &ndf_neg);
    m_tree->Branch<Float_t>("d0_pos", &D0_pos);
    m_tree->Branch<Float_t>("d0_neg", &D0_neg);
    m_tree->Branch<Float_t>("z0_pos", &Z0_pos);
    m_tree->Branch<Float_t>("z0_neg", &Z0_neg);
    m_tree->Branch<Float_t>("d0ip_pos", &D0ip_pos);
    m_tree->Branch<Float_t>("d0ip_neg", &D0ip_neg);
    m_tree->Branch<Float_t>("z0ip_pos", &Z0ip_pos);
    m_tree->Branch<Float_t>("z0ip_neg", &Z0ip_neg);


    registerObject<TTree>(m_treeName.c_str(), m_tree);
  }
  auto m_hEventT0 = new TH1F("hEventT0", "Event T0", 1000, -100, 100);
  auto m_hNDF_pos = new TH1F("hNDF_pos", "NDF of positive track;NDF;Tracks", 71, -1, 70);
  auto m_hNDF_neg = new TH1F("hNDF_neg", "NDF of negative track;NDF;Tracks", 71, -1, 70);
  auto m_hPval_pos = new TH1F("hPval_pos", "p-values of pos tracks;pVal;Tracks", 1000, 0, 1);
  auto m_hPval_neg = new TH1F("hPval_neg", "p-values of neg tra cks;pVal;Tracks", 1000, 0, 1);
  auto m_hnCDC_pos = new TH1F("hnCDC_pos", "nCDC hit of positive track ; nCDC  ; Tracks", 71, -1, 70);
  auto m_hnCDC_neg = new TH1F("hnCDC_neg", "nCDC hit of negative track ; nCDC  ; Tracks", 71, -1, 70);



  auto m_hdPt = new TH1F("hdPt", "#DeltaP_{t} ; #DeltaP_{t} ; Events ", 200, -0.5, 0.5);
  auto m_hdD0 = new TH1F("hdD0", "#DeltaD_{0} ; #DeltaD_{0} ; Events ", 200, -0.1, 0.1);
  auto m_hdZ0 = new TH1F("hdZ0", "#DeltaZ_{0} ; #DeltaD_{0} ; Events ", 200, -1.5, 1.5);

  auto m_hdPt_cm = new TH1F("hdPt_cm", "#DeltaP_{t} in c.m frame ; #DeltaP_{t} (c.m) ; Events ", 200, -0.5, 0.5);
  auto m_hdPtPt_cm = new TH2F("hdPtPt_cm", "#DeltaP_{t}:P_{t} in c.m frame ;P_{t} GeV/c  ; #DeltaP_{t} (c.m) ", 50, 2., 7., 200, -0.5,
                              0.5);
  auto m_hdPhi0_cm = new TH1F("hdPhi0_cm", "#DeltaPhi_{0}  in c.m frame ; #Delta#Phi_{0} in c.m ; Events ", 200, -0.7, 0.7);
  auto m_hdTheta_cm = new TH1F("hdTheta_cm", "#Delta#theta in c.m frame ; #Delta#theta in c.m ; Events ", 200, -3.0, 3.0);

  registerObject<TH1F>("hEventT0", m_hEventT0);
  registerObject<TH1F>("hNDF_pos", m_hNDF_pos);
  registerObject<TH1F>("hNDF_neg", m_hNDF_neg);
  registerObject<TH1F>("hnCDC_pos", m_hnCDC_pos);
  registerObject<TH1F>("hnCDC_neg", m_hnCDC_neg);

  registerObject<TH1F>("hPval_pos", m_hPval_pos);
  registerObject<TH1F>("hPval_neg", m_hPval_neg);


  registerObject<TH1F>("hdPt", m_hdPt);
  registerObject<TH1F>("hdD0", m_hdD0);
  registerObject<TH1F>("hdZ0", m_hdZ0);

  registerObject<TH1F>("hdPt_cm", m_hdPt_cm);
  registerObject<TH2F>("hdPtPt_cm", m_hdPtPt_cm);
  registerObject<TH1F>("hdPhi0_cm", m_hdPhi0_cm);
  registerObject<TH1F>("hdTheta_cm", m_hdTheta_cm);
}

void CDCFudgeFactorCalibrationCollectorModule::collect()
{


  //  make evt t0 incase we dont use evt t0
  double evtT0 = 0;
  // event with is fail to extract t0 will be exclude from analysis
  if (m_eventTimeStoreObject.isValid() && m_eventTimeStoreObject->hasEventT0()) {
    evtT0 =  m_eventTimeStoreObject->getEventT0();
    getObjectPtr<TH1F>("hEventT0")->Fill(evtT0);
  } else {
    return;
  }
  //store run/exp info
  StoreObjPtr<EventMetaData> m_EventMetaData;   /**< Event metadata. */
  int run = m_EventMetaData->getRun();
  int exp = m_EventMetaData->getExperiment();
  exp_run = exp * 1000000 + run;

  /*******************************************/
  /*            VERTEX information           */
  /*******************************************/
  static DBObjPtr<BeamSpot> beamSpotDB;
  const TVector3 IP = beamSpotDB->getIPPosition();

  /************************************/
  /// Calculate total energy of Ecl for neutral
  StoreArray<Belle2::ECLCluster> eclCluster;
  int nG = eclCluster.getEntries();
  //  TLorentzVector gamma_cms;
  double Eecl_neutral = 0;
  for (int i = 0; i < nG; ++i) {
    ECLCluster* ecl = eclCluster[i];
    //    double Eecl->get4Vector().E();
    if (not ecl->isNeutral()) continue;
    if (not ecl->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) continue;
    double Ecluster = ecl->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons);
    if (Ecluster > 0.1) {
      Eecl_neutral += Ecluster;
    }
  }
  //  B2INFO("Sum of neutral ECL "<<Eecl_neutral);
  /************************************/

  const int nTr = m_Tracks.getEntries();
  PCmsLabTransform T;
  //now start to collect dimuon parameters
  double   theta_pos, theta_neg;
  int nCDC_pos, nCDC_neg;
  int nCTracks  = 0;
  int charge_sum = 0;
  double  Eecl_trk;
  for (int i = 0; i < nTr; ++i) {
    const Belle2::Track* b2track = m_Tracks[i];
    const Belle2::TrackFitResult* fitresult = b2track->getTrackFitResultWithClosestMass(Const::muon);
    if (!fitresult) {
      B2WARNING("No track fit result found.");
      continue;
    }
    // cut on D0 and Z0
    short chg = fitresult->getChargeSign();
    double D0  = fitresult->getD0();
    double  Z0 = fitresult->getZ0();
    if (fabs(D0) > 2 || fabs(Z0) > 4) continue;

    ECLCluster* ecl = b2track->getRelatedTo<ECLCluster>();
    if (ecl) {
      if (not ecl->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) continue;
      double Eecl = ecl->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons);
      //      B2INFO("ECL found with E: "<< Eecl);
      Eecl_trk += Eecl;
    }

    if (chg > 0) {
      ndf_pos = fitresult->getNDF();
      Pval_pos = fitresult->getPValue();
      Pt_pos = fitresult->getTransverseMomentum();
      D0_pos  = fitresult->getD0();
      Z0_pos  = fitresult->getZ0();
      theta_pos = fitresult->getMomentum().Theta() * 180 / M_PI;
      nCDC_pos = fitresult->getHitPatternCDC().getNHits();
      ROOT::Math::PxPyPzEVector P4_pos = T.rotateLabToCms() * fitresult->get4Momentum();
      Pt_pos_cm = P4_pos.Pt();
      Theta_pos_cm = P4_pos.Theta() * 180 / M_PI;
      Phi0_pos_cm = P4_pos.Phi() * 180 / M_PI;
      Helix helix  = fitresult->getHelix();
      helix.passiveMoveBy(IP.X(), IP.Y(), IP.Z());
      D0ip_pos = helix.getD0();
      Z0ip_pos = helix.getZ0();
    } else if (chg < 0) {
      ndf_neg = fitresult->getNDF();
      Pval_neg = fitresult->getPValue();
      Pt_neg = fitresult->getTransverseMomentum();
      D0_neg  = fitresult->getD0();
      Z0_neg  = fitresult->getZ0();
      theta_neg = fitresult->getMomentum().Theta() * 180 / M_PI;
      nCDC_neg = fitresult->getHitPatternCDC().getNHits();
      ROOT::Math::PxPyPzEVector P4_neg = T.rotateLabToCms() * fitresult->get4Momentum();
      Pt_neg_cm = P4_neg.Pt();
      Theta_neg_cm = P4_neg.Theta() * 180 / M_PI;
      Phi0_neg_cm = P4_neg.Phi() * 180 / M_PI;
      Helix helix  = fitresult->getHelix();
      helix.passiveMoveBy(IP.X(), IP.Y(), IP.Z());
      D0ip_neg = helix.getD0();
      Z0ip_neg = helix.getZ0();

    } else {
      continue;
    }
    //count #good charged track and sum of charged
    nCTracks++;
    charge_sum += chg;
  }
  // cut good charged track
  if (nCTracks != 2 || charge_sum != 0) {
    return ;
  }

  //cut according to the Line400 in the plotMumu_4ff.C
  if (Pt_pos < 2.5  || Pt_neg < 2.5
      || theta_pos < 45 || theta_pos > 125
      || theta_neg < 45 || theta_neg > 125) {return;}

  // cut on Energy deposite in ECL
  double Eecl_tot = Eecl_neutral + Eecl_trk;
  if (Eecl_tot > 2 || Eecl_trk > 2) return;

  //  B2INFO("Total Eecl_track = "<< Eecl_trk);
  double dPt = (Pt_pos - Pt_neg) / sqrt(2);
  double dD0 = (D0_pos + D0_neg) / sqrt(2);
  double dZ0 = (Z0_pos - Z0_neg) / sqrt(2);
  double Pt_cm = (Pt_pos_cm + Pt_neg_cm) / 2;
  double dPt_cm = (Pt_pos_cm - Pt_neg_cm) / sqrt(2);
  double dPhi0_cm = (180 - fabs(Phi0_pos_cm - Phi0_neg_cm)) / sqrt(2);
  double dTheta_cm = (180 - fabs(Theta_pos_cm + Theta_neg_cm)) / sqrt(2);

  B2DEBUG(199, "ECL neutral - trk:" << Eecl_neutral << "  -  " << Eecl_trk);
  B2DEBUG(199, "Pos: theta phi0  :" << Theta_pos_cm << "  - " << Phi0_pos_cm);
  B2DEBUG(199, "Neg: theta phi0  :" << Theta_neg_cm << "  - " << Phi0_neg_cm);
  B2DEBUG(199, "DeltaPhi  Theta  :" << dPhi0_cm << "  " << dTheta_cm);

  //require back to back
  if (fabs(dPhi0_cm) > m_minCollinearityPhi0 || fabs(dTheta_cm) > m_minCollinearityTheta) return;

  getObjectPtr<TH1F>("hPval_pos")->Fill(Pval_pos);
  getObjectPtr<TH1F>("hPval_neg")->Fill(Pval_neg);
  getObjectPtr<TH1F>("hNDF_pos")->Fill(ndf_pos);
  getObjectPtr<TH1F>("hNDF_neg")->Fill(ndf_neg);
  getObjectPtr<TH1F>("hnCDC_pos")->Fill(nCDC_pos);
  getObjectPtr<TH1F>("hnCDC_neg")->Fill(nCDC_neg);

  getObjectPtr<TH1F>("hdPt")->Fill(dPt);
  getObjectPtr<TH1F>("hdD0")->Fill(dD0);
  getObjectPtr<TH1F>("hdZ0")->Fill(dZ0);

  getObjectPtr<TH1F>("hdPt_cm")->Fill(dPt_cm);
  getObjectPtr<TH2F>("hdPtPt_cm")->Fill(Pt_cm, dPt_cm);
  getObjectPtr<TH1F>("hdPhi0_cm")->Fill(dPhi0_cm);
  getObjectPtr<TH1F>("hdTheta_cm")->Fill(dTheta_cm);

  if (m_StoreNtuple) {
    getObjectPtr<TTree>(m_treeName.c_str())->Fill();
  }

}

void CDCFudgeFactorCalibrationCollectorModule::finish()
{
}
