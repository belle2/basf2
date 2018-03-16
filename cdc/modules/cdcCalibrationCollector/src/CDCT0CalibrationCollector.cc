/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: CDC Group                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "cdc/modules/cdcCalibrationCollector/CDCT0CalibrationCollector.h"

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
#include "TH1F.h"
#include <cdc/dataobjects/WireID.h>
#include <cdc/geometry/CDCGeometryPar.h>

using namespace std;
using namespace Belle2;
using namespace CDC;
using namespace genfit;

REG_MODULE(CDCT0CalibrationCollector)

CDCT0CalibrationCollectorModule::CDCT0CalibrationCollectorModule() : CalibrationCollectorModule()
{
  setDescription("Collector module for cdc T0 calibration");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("RecoTracksColName", m_recoTrackArrayName, "Name of collection hold genfit::Track", std::string(""));
  addParam("BField", m_BField, "If true -> #Params ==5 else #params ==4 for calculate P-Val", false);
  addParam("EventT0Extraction", m_EventT0Extraction, "use event t0 extract t0 or not", false);
  addParam("MinimumPt", m_MinimumPt, "Tracks whose Pt lower than this value will not be recoreded", 0.);
  addParam("PvalCut", m_PvalCut, "Tracks which Pval small than this will not be recoreded", 0.);
  addParam("NDFCut", m_ndfCut, "Tracks which NDF small than this will not be recoreded", 0.);
  addParam("Xmin", m_xmin, "minimum dift length", 0.1);
}

CDCT0CalibrationCollectorModule::~CDCT0CalibrationCollectorModule()
{
}

void CDCT0CalibrationCollectorModule::prepare()
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

  auto m_hNDF = new TH1D("hNDF", "NDF of fitted track;NDF;Tracks", 71, -1, 70);
  auto m_hPval = new TH1D("hPval", "p-values of tracks;pVal;Tracks", 1000, 0, 1);

  registerObject<TH1D>("hNDF", m_hNDF);
  registerObject<TH1D>("hPval", m_hPval);

  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  for (int i = 0; i < 56; ++i) {
    double R = cdcgeo.senseWireR(i);
    double nW = cdcgeo.nWiresInLayer(i);
    halfCSize[i] = M_PI * R / nW;
  }

  auto m_hTotal = new TH1F("hTotal", "hTotal", 30, -15, 15);
  registerObject<TH1F>("hTotal", m_hTotal);
  //for each channel
  TH1F* m_h1[56][385];
  TH1F* m_hT0b[300];
  for (int il = 0; il < 56; il++) {
    const int nW = cdcgeo.nWiresInLayer(il);
    for (int ic = 0; ic < nW; ++ic) {
      m_h1[il][ic] = new TH1F(Form("hdT_lay%d_cell%d", il, ic), Form("Lay%d_cell%d", il, ic), 30, -15, 15);
      registerObject<TH1F>(Form("hdT_lay%d_cell%d", il, ic), m_h1[il][ic]);
    }
  }
  //for each board
  for (int ib = 0; ib < 300; ++ib) {
    m_hT0b[ib] = new TH1F(Form("hT0b%d", ib), Form("boardID_%d", ib), 100, -20, 20);
    registerObject<TH1F>(Form("hT0b%d", ib), m_hT0b[ib]);
  }

}

void CDCT0CalibrationCollectorModule::collect()
{
  const StoreArray<Belle2::Track> storeTrack(m_trackArrayName);
  const StoreArray<Belle2::TrackFitResult> storeTrackFitResults(m_trackFitResultArrayName);
  const StoreArray<Belle2::CDCHit> cdcHits(m_cdcHitArrayName);
  const StoreArray<Belle2::RecoTrack> recoTracks(m_recoTrackArrayName);
  const RelationArray relTrackTrack(recoTracks, storeTrack, m_relRecoTrackTrackName);

  /* CDCHit distribution */
  //  make evt t0 in case we don't use evt t0
  double evtT0 = 0;
  const int nTr = recoTracks.getEntries();

  for (int i = 0; i < nTr; ++i) {
    RecoTrack* track = recoTracks[i];
    if (track->getDirtyFlag()) continue;
    if (!track->getTrackFitStatus()->isFitted()) continue;
    const genfit::FitStatus* fs = track->getTrackFitStatus();
    if (!fs || !fs->isFitConverged()) continue; //not fully converged

    const Belle2::Track* b2track = track->getRelatedFrom<Belle2::Track>();
    if (!b2track) {
      B2DEBUG(99, "No relation found");
      continue;
    }

    const Belle2::TrackFitResult* fitresult = b2track->getTrackFitResult(Const::muon);

    if (!fitresult) {
      B2WARNING("track was fitted but Relation not found");
      continue;
    }

    double ndf = 0;
    double Pval = 0;
    if (m_BField) {
      ndf = fs->getNdf();
      Pval = fs->getPVal();
    } else {
      ndf = fs->getNdf() + 1;
      double Chi2 = fs->getChi2();
      Pval = std::max(0., ROOT::Math::chisquared_cdf_c(Chi2, ndf));
    }

    getObjectPtr<TH1D>("hPval")->Fill(Pval);
    getObjectPtr<TH1D>("hNDF")->Fill(ndf);

    if (Pval < m_PvalCut || ndf < m_ndfCut) continue;
    //cut at Pt
    if (fitresult->getMomentum().Perp() < m_MinimumPt) continue;
    //reject events don't have eventT0
    if (m_EventT0Extraction) {
      // event with is fail to extract t0 will be exclude from analysis
      if (m_eventTimeStoreObject.isValid() && m_eventTimeStoreObject->hasEventT0()) {
        evtT0 =  m_eventTimeStoreObject->getEventT0();
      } else {
        continue;
      }
    }

    B2DEBUG(99, "start collect hit");
    static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();

    for (const RecoHitInformation::UsedCDCHit* hit : track->getCDCHitList()) {
      const genfit::TrackPoint* tp = track->getCreatedTrackPoint(track->getRecoHitInformation(hit));
      int lay = hit->getICLayer();
      int IWire = hit->getIWire();
      unsigned short tdc = hit->getTDCCount();
      unsigned short adc = hit->getADCCount();
      WireID wireid(lay, IWire);
      const genfit::KalmanFitterInfo* kfi = tp->getKalmanFitterInfo();
      if (!kfi) {B2DEBUG(199, "No Fitter Info: Layer " << hit->getICLayer()); continue;}
      for (unsigned int iMeas = 0; iMeas < kfi->getNumMeasurements(); ++iMeas) {
        if ((kfi->getWeights().at(iMeas))  > 0.5) {
          int boardID = cdcgeo.getBoardID(WireID(lay, IWire));
          const genfit::MeasuredStateOnPlane& mop = kfi->getFittedState();
          const TVector3 pocaOnWire = mop.getPlane()->getO();//Local wire position
          //    const TVector3 pocaOnTrack = mop.getPlane()->getU();//residual direction
          const TVector3 pocaMom = mop.getMom();
          double alpha = cdcgeo.getAlpha(pocaOnWire, pocaMom) ;
          double theta = cdcgeo.getTheta(pocaMom);
          double x_u = kfi->getFittedState(false).getState()(3);//x fit unbiased
          //    double weight = kfi->getWeights().at(iMeas);

          double xmax = halfCSize[lay] - 0.1;
          if ((fabs(x_u) < m_xmin) || (fabs(x_u) > xmax)) continue;

          int lr;
          if (x_u > 0) lr = 1;
          else lr = 0;

          //Convert to outgoing
          if (fabs(alpha) > M_PI / 2) {
            x_u *= -1;
          }

          lr = cdcgeo.getOutgoingLR(lr, alpha);
          theta = cdcgeo.getOutgoingTheta(alpha, theta);
          alpha = cdcgeo.getOutgoingAlpha(alpha);

          double t_fit = cdcgeo.getDriftTime(std::abs(x_u), lay, lr, alpha , theta);
          //estimate drift time
          double dt_flight;
          double dt_prop;
          double t = cdcgeo.getT0(wireid) - tdc * cdcgeo.getTdcBinWidth(); // - dt_flight - dt_prop;
          dt_flight = mop.getTime();
          if (dt_flight < 50) {
            t -= dt_flight;
          } else {
            continue;
          }

          double z = pocaOnWire.Z();
          TVector3 m_backWirePos = cdcgeo.wireBackwardPosition(wireid, CDCGeometryPar::c_Aligned);
          double z_prop = z - m_backWirePos.Z();
          B2DEBUG(99, "z_prop = " << z_prop << " |z " << z << " |back wire poss: " << m_backWirePos.Z());
          dt_prop = z_prop * cdcgeo.getPropSpeedInv(lay);
          if (z_prop < 240) {
            t -= dt_prop;
          } else {
            continue;
          }
          // Time Walk
          t -= cdcgeo.getTimeWalk(wireid, adc);
          // substract event t0;
          t -= evtT0;

          getObjectPtr<TH1F>(Form("hdT_lay%d_cell%d", lay, IWire))->Fill(t - t_fit);
          getObjectPtr<TH1F>(Form("hT0b%d", boardID))->Fill(t - t_fit);
          getObjectPtr<TH1F>("hTotal")->Fill(t - t_fit);
        } //NDF
        // }//end of if isU
      }//end of for
    }//end of for tp
  }//end of func
}

void CDCT0CalibrationCollectorModule::finish()
{
}


