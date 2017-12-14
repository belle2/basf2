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
  addParam("RecoTracksColName", m_recoTrackArrayName, "Name of collection hold genfit::Track", std::string(""));
  addParam("BField", m_BField, "If true -> #Params ==5 else #params ==4 for calculate P-Val", false);
  addParam("calExpectedDriftTime", m_calExpectedDriftTime, "if true module will calculate expected drift time, it take a time",
           false);
  addParam("StoreTrackParams", m_StoreTrackParams, "Store Track Parameter or not, it will be multicount for each hit", true);
  addParam("EventT0Extraction", m_EventT0Extraction, "use event t0 extract t0 or not", false);
  addParam("MinimumPt", m_MinimumPt, "Tracks with tranverse momentum small than this will not recored", 0.);


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
  m_tree->Branch<double>("x_mea", &x_mea);
  m_tree->Branch<double>("x_u", &x_u);
  m_tree->Branch<double>("x_b", &x_b);
  m_tree->Branch<double>("alpha", &alpha);
  m_tree->Branch<double>("theta", &theta);
  m_tree->Branch<double>("t", &t);
  m_tree->Branch<unsigned short>("adc", &adc);
  //  m_tree->Branch<int>("boardID", &boardID);
  m_tree->Branch<int>("lay", &lay);
  m_tree->Branch<double>("weight", &weight);
  m_tree->Branch<int>("IWire", &IWire);
  m_tree->Branch<double>("Pval", &Pval);
  m_tree->Branch<double>("ndf", &ndf);
  if (m_StoreTrackParams) {
    m_tree->Branch<double>("d0", &d0);
    m_tree->Branch<double>("z0", &z0);
    m_tree->Branch<double>("phi0", &phi0);
    m_tree->Branch<double>("tanL", &tanL);
    m_tree->Branch<double>("omega", &omega);
  }

  if (m_calExpectedDriftTime) { // expected drift time, calculated form xfit
    m_tree->Branch<double>("t_fit", &t_fit);
  }

  auto m_hNDF = new TH1D("hNDF", "NDF of fitted track;NDF;Tracks", 71, -1, 70);
  auto m_hPval = new TH1D("hPval", "p-values of tracks;pVal;Tracks", 1000, 0, 1);

  registerObject<TTree>("tree", m_tree);
  registerObject<TH1D>("hNDF", m_hNDF);
  registerObject<TH1D>("hPval", m_hPval);
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
  int nTr = recoTracks.getEntries();

  for (int i = 0; i < nTr; ++i) {
    RecoTrack* track = recoTracks[i];
    if (track->getDirtyFlag()) continue;
    if (!track->getTrackFitStatus()->isFitted()) continue;
    const genfit::FitStatus* fs = track->getTrackFitStatus();
    if (!fs || !fs->isFitConverged()) continue; //not fully convergence

    const Belle2::Track* b2track = track->getRelatedFrom<Belle2::Track>();
    if (!b2track) {B2DEBUG(99, "No relation found"); continue;}
    const Belle2::TrackFitResult*    fitresult = b2track->getTrackFitResult(Const::muon);

    if (!fitresult) {
      B2WARNING("track was fitted but Relation not found");
      continue;
    }
    if (!m_BField) {ndf = fs->getNdf() + 1;} // incase no Magnetic field, Npars = 4;
    else {ndf = fs->getNdf();}
    if (ndf < 15) continue;
    double Chi2 = fs->getChi2();
    Pval = std::max(0., ROOT::Math::chisquared_cdf_c(Chi2, ndf));
    //store track parameters
    if (m_StoreTrackParams) {
      d0 = fitresult->getD0();
      z0 = fitresult->getZ0();
      tanL = fitresult->getTanLambda();
      omega = fitresult->getOmega();
      phi0 = fitresult->getPhi0() * 180 / M_PI;
    }
    getObjectPtr<TH1D>("hPval")->Fill(Pval);
    getObjectPtr<TH1D>("hNDF")->Fill(ndf);
    B2DEBUG(99, "ndf = " << ndf);
    B2DEBUG(99, "Pval = " << Pval);
    //cut at Pt
    if (fitresult->getMomentum().Perp() < m_MinimumPt) continue;
    //reject events don't have eventT0
    if (m_EventT0Extraction) {
      // event with is fail to extract t0 will be exclude from analysis
      if (m_eventTimeStoreObject.isValid() && m_eventTimeStoreObject->hasDoubleEventT0()) {
        evtT0 =  m_eventTimeStoreObject->getEventT0();
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
        double dt_flight;
        double dt_prop;
        t = cdcgeo.getT0(wireid) - tdc * cdcgeo.getTdcBinWidth(); // - dt_flight - dt_prop;
        dt_flight = mop.getTime();
        if (dt_flight < 50) {t -= dt_flight;}
        double z = pocaOnWire.Z();
        TVector3 m_backWirePos = cdcgeo.wireBackwardPosition(wireid, CDCGeometryPar::c_Aligned);
        double z_prop = z - m_backWirePos.Z();
        B2DEBUG(99, "z_prop = " << z_prop << " |z " << z << " |back wire poss: " << m_backWirePos.Z());
        dt_prop = z_prop * cdcgeo.getPropSpeedInv(lay);
        if (z_prop < 240) {t -= dt_prop;}
        // Time Walk
        t -= cdcgeo.getTimeWalk(wireid, adc);
        // substract event t0;
        t -= evtT0;

        getObjectPtr<TTree>("tree")->Fill();
      } //NDF
      // }//end of if isU
    }//end of for
  }//end of for tp
}//end of func



