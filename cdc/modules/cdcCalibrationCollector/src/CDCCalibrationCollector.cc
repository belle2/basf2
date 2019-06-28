/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: CDC Group                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "cdc/modules/cdcCalibrationCollector/CDCCalibrationCollector.h"
#include <cdc/translators/RealisticTDCCountTranslator.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <genfit/TrackPoint.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/KalmanFitterInfo.h>
#include <genfit/MeasurementOnPlane.h>
#include <genfit/MeasuredStateOnPlane.h>
#include <Math/ProbFuncMathCore.h>

#include <cdc/dataobjects/WireID.h>
#include <cdc/geometry/CDCGeometryPar.h>


using namespace std;
using namespace Belle2;
using namespace CDC;
using namespace genfit;

REG_MODULE(CDCCalibrationCollector)

//                 Implementation

CDCCalibrationCollectorModule::CDCCalibrationCollectorModule() : CalibrationCollectorModule()
{
  setDescription("Collector module for cdc calibration");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("recoTracksColName", m_recoTrackArrayName, "Name of collection hold genfit::Track", std::string(""));
  addParam("bField", m_bField, "If true -> #Params ==5 else #params ==4 for calculate P-Val", false);
  addParam("calExpectedDriftTime", m_calExpectedDriftTime, "if true module will calculate expected drift time, it take a time",
           false);

  addParam("storeTrackParams", m_storeTrackParams, "Store Track Parameter or not, it will be multicount for each hit", false);
  addParam("eventT0Extraction", m_eventT0Extraction, "use event t0 extract t0 or not", false);
  addParam("minimumPt", m_minimumPt, "Tracks with tranverse momentum small than this will not recored", 0.);
  addParam("isCosmic", m_isCosmic, "True when we process cosmic events, else False (collision)", m_isCosmic);
}

CDCCalibrationCollectorModule::~CDCCalibrationCollectorModule()
{
}

void CDCCalibrationCollectorModule::prepare()
{
  StoreArray<Belle2::Track> storeTrack(m_trackArrayName);
  StoreArray<RecoTrack> recoTracks(m_recoTrackArrayName);
  StoreArray<Belle2::TrackFitResult> storeTrackFitResults(m_trackFitResultArrayName);
  StoreArray<Belle2::CDCHit> cdcHits(m_cdcHitArrayName);
  RelationArray relRecoTrackTrack(recoTracks, storeTrack, m_relRecoTrackTrackName);
  //Store names to speed up creation later
  m_recoTrackArrayName = recoTracks.getName();
  m_trackFitResultArrayName = storeTrackFitResults.getName();
  m_relRecoTrackTrackName = relRecoTrackTrack.getName();


  auto m_tree  = new TTree(m_treeName.c_str(), "tree for cdc calibration");
  m_tree->Branch<float>("x_mea", &x_mea);
  m_tree->Branch<float>("x_u", &x_u);
  m_tree->Branch<float>("x_b", &x_b);
  m_tree->Branch<float>("alpha", &alpha);
  m_tree->Branch<float>("theta", &theta);
  m_tree->Branch<float>("t", &t);
  m_tree->Branch<unsigned short>("adc", &adc);
  //  m_tree->Branch<int>("boardID", &boardID);
  m_tree->Branch<int>("lay", &lay);
  m_tree->Branch<float>("weight", &weight);
  m_tree->Branch<int>("IWire", &IWire);
  m_tree->Branch<float>("Pval", &Pval);
  m_tree->Branch<float>("ndf", &ndf);
  if (m_storeTrackParams) {
    m_tree->Branch<float>("d0", &d0);
    m_tree->Branch<float>("z0", &z0);
    m_tree->Branch<float>("phi0", &phi0);
    m_tree->Branch<float>("tanL", &tanL);
    m_tree->Branch<float>("omega", &omega);
  }

  if (m_calExpectedDriftTime) { // expected drift time, calculated form xfit
    m_tree->Branch<float>("t_fit", &t_fit);
  }

  auto m_hNDF = new TH1F("hNDF", "NDF of fitted track;NDF;Tracks", 71, -1, 70);
  auto m_hPval = new TH1F("hPval", "p-values of tracks;pVal;Tracks", 1000, 0, 1);
  auto m_hEventT0 = new TH1F("hEventT0", "Event T0", 1000, -100, 100);

  registerObject<TTree>("tree", m_tree);
  registerObject<TH1F>("hNDF", m_hNDF);
  registerObject<TH1F>("hPval", m_hPval);
  registerObject<TH1F>("hEventT0", m_hEventT0);
}

void CDCCalibrationCollectorModule::collect()
{
  const StoreArray<Belle2::Track> storeTrack(m_trackArrayName);
  const StoreArray<Belle2::TrackFitResult> storeTrackFitResults(m_trackFitResultArrayName);
  const StoreArray<Belle2::CDCHit> cdcHits(m_cdcHitArrayName);
  const StoreArray<Belle2::RecoTrack> recoTracks(m_recoTrackArrayName);
  const RelationArray relTrackTrack(recoTracks, storeTrack, m_relRecoTrackTrackName);

  /* CDCHit distribution */
  //  make evt t0 incase we dont use evt t0
  evtT0 = 0;
  const int nTr = recoTracks.getEntries();

  for (int i = 0; i < nTr; ++i) {
    RecoTrack* track = recoTracks[i];
    if (track->getDirtyFlag()) continue;
    if (!track->getTrackFitStatus()->isFitted()) continue;
    const genfit::FitStatus* fs = track->getTrackFitStatus();
    if (!fs || !fs->isFitConverged()) continue; //not fully convergence

    const Belle2::Track* b2track = track->getRelatedFrom<Belle2::Track>();
    if (!b2track) {B2DEBUG(99, "No relation found"); continue;}
    const Belle2::TrackFitResult* fitresult = b2track->getTrackFitResult(Const::muon);

    if (!fitresult) {
      B2WARNING("track was fitted but Relation not found");
      continue;
    }
    if (!m_bField) {
      ndf = fs->getNdf() + 1;
    } else {
      ndf = fs->getNdf();
    }

    getObjectPtr<TH1F>("hPval")->Fill(Pval);
    getObjectPtr<TH1F>("hNDF")->Fill(ndf);
    B2DEBUG(99, "ndf = " << ndf);
    B2DEBUG(99, "Pval = " << Pval);

    if (ndf < 15) continue;
    double Chi2 = fs->getChi2();
    Pval = std::max(0., ROOT::Math::chisquared_cdf_c(Chi2, ndf));
    //store track parameters

    d0 = fitresult->getD0();
    z0 = fitresult->getZ0();
    tanL = fitresult->getTanLambda();
    omega = fitresult->getOmega();
    phi0 = fitresult->getPhi0() * 180 / M_PI;

    // Rejection of suspicious cosmic tracks.
    // phi0 of cosmic track must be negative in our definition!
    if (m_isCosmic == true && phi0 > 0.0) continue;

    //cut at Pt

    if (fitresult->getMomentum().Perp() < m_minimumPt) continue;
    //reject events don't have eventT0
    if (m_eventT0Extraction) {
      // event with is fail to extract t0 will be exclude from analysis
      if (m_eventTimeStoreObject.isValid() && m_eventTimeStoreObject->hasEventT0()) {
        evtT0 =  m_eventTimeStoreObject->getEventT0();
        getObjectPtr<TH1F>("hEventT0")->Fill(evtT0);
      } else {
        continue;
      }
    }

    try {
      harvest(track);
    } catch (...) {
    }
  }
}

void CDCCalibrationCollectorModule::finish()
{
}

void CDCCalibrationCollectorModule::harvest(Belle2::RecoTrack* track)
{
  B2DEBUG(99, "start collect hit");
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  static CDC::RealisticTDCCountTranslator* tdcTrans = new RealisticTDCCountTranslator(true);

  for (const RecoHitInformation::UsedCDCHit* hit : track->getCDCHitList()) {
    const genfit::TrackPoint* tp = track->getCreatedTrackPoint(track->getRecoHitInformation(hit));
    lay = hit->getICLayer();
    IWire = hit->getIWire();
    adc = hit->getADCCount();
    unsigned short tdc = hit->getTDCCount();
    WireID wireid(lay, IWire);
    const genfit::KalmanFitterInfo* kfi = tp->getKalmanFitterInfo();
    if (!kfi) {B2DEBUG(199, "No Fitter Info: Layer " << hit->getICLayer()); continue;}
    for (unsigned int iMeas = 0; iMeas < kfi->getNumMeasurements(); ++iMeas) {
      if ((kfi->getWeights().at(iMeas))  > 0.5) {
        //  int boardID = cdcgeo.getBoardID(WireID(lay,IWire));
        const genfit::MeasuredStateOnPlane& mop = kfi->getFittedState();
        const TVector3 pocaOnWire = mop.getPlane()->getO();//Local wire position
        const TVector3 pocaOnTrack = mop.getPlane()->getU();//residual direction
        const TVector3 pocaMom = mop.getMom();
        alpha = cdcgeo.getAlpha(pocaOnWire, pocaMom) ;
        theta = cdcgeo.getTheta(pocaMom);
        x_mea = kfi->getMeasurementOnPlane(iMeas)->getState()(0);
        x_b = kfi->getFittedState(true).getState()(3);// x fit biased
        x_u = kfi->getFittedState(false).getState()(3);//x fit unbiased
        weight = kfi->getWeights().at(iMeas);

        int lr;
        if (x_u > 0) lr = 1;
        else lr = 0;

        //Convert to outgoing
        if (fabs(alpha) > M_PI / 2) {
          x_b *= -1;
          x_u *= -1;
        }
        x_mea = copysign(x_mea, x_b);
        lr = cdcgeo.getOutgoingLR(lr, alpha);
        theta = cdcgeo.getOutgoingTheta(alpha, theta);
        alpha = cdcgeo.getOutgoingAlpha(alpha);

        B2DEBUG(99, "x_unbiased " << x_u << " |left_right " << lr);
        if (m_calExpectedDriftTime) { t_fit = cdcgeo.getDriftTime(std::abs(x_u), lay, lr, alpha , theta);}
        alpha *= 180 / M_PI;
        theta *= 180 / M_PI;
        //estimate drift time
        t = tdcTrans->getDriftTime(tdc, wireid, mop.getTime(), pocaOnWire.Z(), adc);
        getObjectPtr<TTree>("tree")->Fill();
      } //NDF
      // }//end of if isU
    }//end of for
  }//end of for tp
}//end of func



