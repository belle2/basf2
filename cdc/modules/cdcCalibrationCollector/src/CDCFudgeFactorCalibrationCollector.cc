/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "cdc/modules/cdcCalibrationCollector/CDCFudgeFactorCalibrationCollector.h"
#include "analysis/utility/PCmsLabTransform.h"
#include <mdst/dataobjects/HitPatternVXD.h>
#include <mdst/dataobjects/EventLevelTrackingInfo.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/variables/PIDVariables.h>

#include <TH1F.h>
#include <TH2F.h>

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
  addParam("DiMuonListName",  m_DiMuonListName, "name of the di-muon list", std::string("vpho:mumu"));
  addParam("GammaListName",  m_GammaListName, "name of the gamma list", std::string("gamma:HLT"));

}

CDCFudgeFactorCalibrationCollectorModule::~CDCFudgeFactorCalibrationCollectorModule()
{
}

void CDCFudgeFactorCalibrationCollectorModule::prepare()
{
  m_Tracks.isRequired(m_trackArrayName);
  m_TrackFitResults.isRequired(m_trackFitResultArrayName);
  m_DiMuonList.isRequired(m_DiMuonListName);

  if (m_StoreNtuple) {
    auto m_tree  = new TTree(m_treeName.c_str(), "tree for cdc calibration");
    m_tree->Branch<Int_t>("exp_run", &expRun);
    m_tree->Branch<Float_t>("pt_pos", &ptPos);
    m_tree->Branch<Float_t>("pt_neg", &ptNeg);
    m_tree->Branch<Float_t>("pz_pos", &pzPos);
    m_tree->Branch<Float_t>("pz_neg", &pzNeg);

    m_tree->Branch<Float_t>("pt_pos_cm", &ptPosCm);
    m_tree->Branch<Float_t>("pt_neg_cm", &ptNegCm);
    m_tree->Branch<Float_t>("pz_pos_cm", &pzPosCm);
    m_tree->Branch<Float_t>("pz_neg_cm", &pzNegCm);

    m_tree->Branch<Float_t>("theta_pos_cm", &thetaPosCm);
    m_tree->Branch<Float_t>("theta_neg_cm", &thetaNegCm);

    m_tree->Branch<Float_t>("phi0_pos_cm",  &phi0PosCm);
    m_tree->Branch<Float_t>("theta_neg_cm", &phi0NegCm);

    m_tree->Branch<Float_t>("Pval_pos", &pvalPos);
    m_tree->Branch<Float_t>("Pval_neg", &pvalNeg);

    m_tree->Branch<Float_t>("ndf_pos", &ndfPos);
    m_tree->Branch<Float_t>("ndf_neg", &ndfNeg);

    m_tree->Branch<Float_t>("ncdc_pos", &ncdcPos);
    m_tree->Branch<Float_t>("ncdc_neg", &ncdcNeg);
    m_tree->Branch<Float_t>("npxd_pos", &npxdPos);
    m_tree->Branch<Float_t>("npxd_neg", &npxdNeg);
    m_tree->Branch<Float_t>("nsvd_pos", &nsvdPos);
    m_tree->Branch<Float_t>("nsvd_neg", &nsvdNeg);

    m_tree->Branch<Float_t>("nextra_cdchit", &nExtraCDCHits);
    m_tree->Branch<Float_t>("ecl_track", &eclTrack);
    m_tree->Branch<Float_t>("ecl_neutral", &eclNeutral);

    m_tree->Branch<Float_t>("d0_pos", &d0Pos);
    m_tree->Branch<Float_t>("d0_neg", &d0Neg);
    m_tree->Branch<Float_t>("z0_pos", &z0Pos);
    m_tree->Branch<Float_t>("z0_neg", &z0Neg);
    m_tree->Branch<Float_t>("d0ip_pos", &d0ipPos);
    m_tree->Branch<Float_t>("d0ip_neg", &d0ipNeg);
    m_tree->Branch<Float_t>("z0ip_pos", &z0ipPos);
    m_tree->Branch<Float_t>("z0ip_neg", &z0ipNeg);
    m_tree->Branch<Float_t>("muid_pos", &muidPos);
    m_tree->Branch<Float_t>("muid_neg", &muidNeg);
    m_tree->Branch<Float_t>("eid_pos", &eidPos);
    m_tree->Branch<Float_t>("eid_neg", &eidNeg);

    registerObject<TTree>(m_treeName.c_str(), m_tree);
  }
  auto m_hEventT0 = new TH1F("hEventT0", "Event T0", 200, -100, 100);
  auto m_hExtraCDCHit = new TH1F("hExtraCDCHit", "Extra cdc hits", 500, 0, 5000);
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
  registerObject<TH1F>("hExtraCDCHit", m_hExtraCDCHit);
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
  int nCandidates = m_DiMuonList->getListSize();
  B2DEBUG(29, "Number of muon canndiate:" << nCandidates);
  if (nCandidates < 1) return;

  // event with is fail to extract t0 will be exclude from analysis
  if (m_eventTimeStoreObject.isValid() && m_eventTimeStoreObject->hasEventT0()) {
    getObjectPtr<TH1F>("hEventT0")->Fill(m_eventTimeStoreObject->getEventT0());
  } else {
    return;
  }
  //store run/exp info
  StoreObjPtr<EventMetaData> m_EventMetaData;   /**< Event metadata. */
  int run = m_EventMetaData->getRun();
  int exp = m_EventMetaData->getExperiment();
  expRun = exp * 1000000 + run;

  /************************************/
  /// Calculate total energy of Ecl for neutral
  StoreObjPtr<ParticleList> gamma_list(m_GammaListName);
  int nG = gamma_list->getListSize();
  B2DEBUG(29, "Number of gamma: " << nG);
  for (int i = 0; i < nG; ++i) {
    Particle* gamma = gamma_list->getParticle(i);
    eclNeutral += gamma->getEnergy();
  }
  B2DEBUG(29, "Sum of neutral ECL " << eclNeutral);
  /************************************/
  PCmsLabTransform T;
  //now start to collect dimuon parameters
  double   thetaPos(0), thetaNeg(0);
  int charge_sum = 0;

  for (int i = 0; i < nCandidates; ++i) {
    Particle* part = m_DiMuonList->getParticle(i);
    //vertex from vertex fit
    ROOT::Math::XYZVector v0Vertex = part->getVertex();

    //loop over its daugter
    for (int j = 0; j < 2; ++j) {
      //we have two daugters.
      const Particle* d0 = part->getDaughter(j);
      short chg =  d0->getCharge();

      const Belle2::TrackFitResult* fitresult  = d0->getTrackFitResult();
      if (!fitresult) {
        B2WARNING("No track fit result found.");
        break;
      }
      //get Cluter Energy
      eclTrack +=  d0->getECLClusterEnergy();
      double muid = Variable::muonID(d0);
      double eid = Variable::electronID(d0);

      if (chg > 0) {
        ndfPos = fitresult->getNDF();
        pvalPos = fitresult->getPValue();
        ptPos = fitresult->getTransverseMomentum();
        pzPos = fitresult->getMomentum().Z();
        d0Pos  = fitresult->getD0();
        z0Pos  = fitresult->getZ0();
        thetaPos = fitresult->getMomentum().Theta() * 180 / M_PI;
        ncdcPos = fitresult->getHitPatternCDC().getNHits();
        nsvdPos = fitresult->getHitPatternVXD().getNSVDHits();
        npxdPos = fitresult->getHitPatternVXD().getNPXDHits();
        ROOT::Math::PxPyPzEVector P4_pos = T.rotateLabToCms() * fitresult->get4Momentum();
        ptPosCm = P4_pos.Pt();
        thetaPosCm = P4_pos.Theta() * 180 / M_PI;
        phi0PosCm = P4_pos.Phi() * 180 / M_PI;
        UncertainHelix helix_pos = fitresult->getUncertainHelix();
        helix_pos.passiveMoveBy(v0Vertex);
        d0ipPos = helix_pos.getD0();
        z0ipPos = helix_pos.getZ0();
        muidPos = muid;  eidPos = eid;
      } else if (chg < 0) {
        ndfNeg = fitresult->getNDF();
        pvalNeg = fitresult->getPValue();
        ptNeg = fitresult->getTransverseMomentum();
        pzNeg = fitresult->getMomentum().Z();
        d0Neg  = fitresult->getD0();
        z0Neg  = fitresult->getZ0();
        thetaNeg = fitresult->getMomentum().Theta() * 180 / M_PI;
        ncdcNeg = fitresult->getHitPatternCDC().getNHits();
        nsvdNeg = fitresult->getHitPatternVXD().getNSVDHits();
        npxdNeg = fitresult->getHitPatternVXD().getNPXDHits();

        ROOT::Math::PxPyPzEVector P4_neg = T.rotateLabToCms() * fitresult->get4Momentum();
        ptNegCm = P4_neg.Pt();
        thetaNegCm = P4_neg.Theta() * 180 / M_PI;
        phi0NegCm = P4_neg.Phi() * 180 / M_PI;
        UncertainHelix helix_neg = fitresult->getUncertainHelix();
        helix_neg.passiveMoveBy(v0Vertex);
        d0ipNeg = helix_neg.getD0();
        z0ipNeg = helix_neg.getZ0();
        muidNeg = muid;
        eidNeg = eid;
      } else {
        continue;
      }
      //count #good charged track and sum of charged
      charge_sum += chg;
    }
    //save extra cdc hit for background monitoring
    StoreObjPtr<EventLevelTrackingInfo> elti;
    if (!elti) nExtraCDCHits = -1;
    else nExtraCDCHits = elti->getNCDCHitsNotAssigned();
    getObjectPtr<TH1F>("hExtraCDCHit")->Fill(nExtraCDCHits);
    // cut good charged track
    if (charge_sum != 0) {continue;}
    //cut according to the Line400 in the plotMumu_4ff.C
    if (ptPos < 2.5  || ptNeg < 2.5
        || thetaPos < 45 || thetaPos > 125
        || thetaNeg < 45 || thetaNeg > 125) {return;}

    // cut on Energy deposite in ECL
    // Keep the same as in the dimuon study script,
    // Although it is not necessary to keep this as cut on Etot is enough
    double eclTot = eclNeutral + eclTrack;
    if (eclTot > 2 || eclTrack > 2) return;

    //  B2INFO("Total Eecl_track = "<< Eecl_trk);
    double dPt = (ptPos - ptNeg) / sqrt(2);
    double dD0 = (d0Pos + d0Neg) / sqrt(2);
    double dZ0 = (z0Pos - z0Neg) / sqrt(2);
    double Pt_cm = (ptPosCm + ptNegCm) / 2;
    double dPt_cm = (ptPosCm - ptNegCm) / sqrt(2);
    double dPhi0_cm = (180 - fabs(phi0PosCm - phi0NegCm)) / sqrt(2);
    double dTheta_cm = (180 - fabs(thetaPosCm + thetaNegCm)) / sqrt(2);

    //require back to back
    if (fabs(dPhi0_cm) > m_minCollinearityPhi0 || fabs(dTheta_cm) > m_minCollinearityTheta) return;

    getObjectPtr<TH1F>("hPval_pos")->Fill(pvalPos);
    getObjectPtr<TH1F>("hPval_neg")->Fill(pvalNeg);
    getObjectPtr<TH1F>("hNDF_pos")->Fill(ndfPos);
    getObjectPtr<TH1F>("hNDF_neg")->Fill(ndfNeg);
    getObjectPtr<TH1F>("hnCDC_pos")->Fill(ncdcPos);
    getObjectPtr<TH1F>("hnCDC_neg")->Fill(ncdcNeg);

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
}

void CDCFudgeFactorCalibrationCollectorModule::finish()
{
}
