/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dong Van Thanh                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "cdc/modules/cdcCRTest/CDCCRTestModule.h"
#include <TTree.h>
#include <framework/gearbox/Const.h>
#include <framework/core/HistoModule.h>
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
#include <set>
#include <boost/format.hpp>
#include <boost/foreach.hpp>

#include "TDirectory.h"
#include "TMath.h"
#include <Math/ProbFuncMathCore.h>
#include "iostream"

using namespace std;
using namespace Belle2;
using namespace CDC;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCCRTest)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCCRTestModule::CDCCRTestModule() : HistoModule()
//CDCCRTestModule::CDCCRTestModule() :Module()
{
  setDescription("CDC Cosmic ray test module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("CorrectToF", m_ToF, "if true, time of flight will take acount in t", true);
  addParam("CorrectToP", m_ToP, "if true, time of Propagation will take acount in t", true);
  addParam("RecoTracksColName", m_recoTrackArrayName, "Name of collectrion hold genfit::Track", std::string(""));
  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "Track fit results histograms will be put into this directory", std::string("trackfit"));
  addParam("fillExpertHistograms", m_fillExpertHistos,
           "Fill additional histograms", false);
  addParam("noBFit", m_noBFit, "If true -> #Params ==4, #params ==5 for calculate P-Val", true);
  addParam("plotResidual", m_plotResidual, "plot biased residual, normalized res and xtplot for all layer", false);
  addParam("calExpectedDriftTime", m_calExpectedDriftTime, "if true module will calculate expected drift time, it take a time",
           false);
  addParam("hitEfficiency", m_hitEfficiency, "calculate hit efficiency true:yes false:No", false);
  addParam("TriggerPos", m_TriggerPos, "Trigger position use for cut and reconstruct Trigger image", std::vector<double> { -0.6, -13.25, 17.3});
  addParam("IwireLow", m_low, "Lower boundary of hit dist. Histogram", std::vector<int> {0, 0, 0, 0, 0, 0, 0, 0, 0});
  addParam("IwireUpper", m_up, "Upper boundary of hit dist. Histogram", std::vector<int> {161, 161, 193, 225, 257, 289, 321, 355, 385});
  addParam("CorrectToFofIncomingTrack", m_IncomingToF, "If true, tof of tracks which are phi0>0 will be invert", true);
  // addParam("plotResidual",m_plotHitDistribution,"plot biased residual, normalized res and xtplot for all layer",true);
}

CDCCRTestModule::~CDCCRTestModule()
{
  m_allHistos.clear();
}
//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------
void CDCCRTestModule::defineHisto()
{
  tree  = new TTree("tree", "tree");
  tree->Branch("x_u", &x_u, "x_u/D");
  tree->Branch("x_b", &x_b, "x_b/D");
  tree->Branch("z_b", &z_b, "z_b/D");
  tree->Branch("z_u", &z_u, "z_u/D");
  tree->Branch("alpha", &alpha, "alpha/D");
  tree->Branch("theta", &theta, "theta/D");
  tree->Branch("z_prop", &z_prop, "z_prop/D");
  tree->Branch("r_flight", &r_flight, "r_flight/D");
  tree->Branch("t", &t, "t/D");
  tree->Branch("dt_prop", &dt_prop, "dt_prop/D");
  tree->Branch("dt_flight", &dt_flight, "dt_flight/D");
  if (m_calExpectedDriftTime) {tree->Branch("t_fit", &t_fit, "t_fit/D");}           // expected drift time, calculated form xfit
  tree->Branch("tdc", &tdc, "tdc/I");
  tree->Branch("adc", &adc, "adc/s");
  tree->Branch("boardID", &boardID, "boardID/I");
  tree->Branch("lay", &lay, "lay/I");
  tree->Branch("res_b", &res_b, "res_b/D");
  tree->Branch("res_u", &res_u, "res_u/D");
  tree->Branch("weight", &weight, "weight/D");
  tree->Branch("res_b_err", &res_b_err, "res_b_err/D");
  tree->Branch("res_u_err", &res_u_err, "res_u_err/D");
  tree->Branch("absRes_u", &absRes_u, "absRes_u/D");
  tree->Branch("absRes_b", &absRes_b, "absRes_b/D");
  tree->Branch("IWire", &IWire, "IWire/I");
  tree->Branch("Pval", &Pval, "Pval/D");
  tree->Branch("numhits", &numhits, "numhits/I");
  tree->Branch("ndf", &ndf, "ndf/I");
  tree->Branch("trigHitPos_x", &trigHitPos_x, "trigHitPos_x/D");
  tree->Branch("trigHitPos_z", &trigHitPos_z, "trigHitPos_z/D");
  tree->Branch("trighit", &trighit, "trighit/I");
  // int N =m_Nchannel;//Number of Wire per Layer used;
  TDirectory* oldDir = gDirectory;
  TDirectory* histDir = oldDir->mkdir(m_histogramDirectoryName.c_str());
  histDir->cd();
  m_hNTracks = getHist("hNTracks", "number of tracks", 3, 0, 3);
  m_hNTracks->GetXaxis()->SetBinLabel(1, "fitted, converged");
  m_hNTracks->GetXaxis()->SetBinLabel(2, "fitted, not converged");
  m_hNTracks->GetXaxis()->SetBinLabel(3, "TrackCand, but no Track");
  m_hNDF = getHist("hNDF", "NDF of fitted track;NDF;Tracks", 71, -1, 70);
  m_hNHits = getHist("hNHits", "#hit of fitted track;#hit;Tracks", 61, -1, 70);
  m_hNHits_trackcand = getHist("hNHits_trackcand", "#hit of track candidate;#hit;Tracks", 71, -1, 70);
  m_hNTracksPerEvent = getHist("hNTracksPerEvent", "#tracks/Event;#Tracks;Event", 20, 0, 20);
  m_hNTracksPerEventFitted = getHist("hNTracksPerEventFitted", "#tracks/Event After Fit;#Tracks;Event", 20, 0, 20);
  m_hE1Dist = getHist("hE1Dist", "Energy Dist. in case 1track/evt; E(Gev);Tracks", 100, 0, 20);
  m_hE2Dist = getHist("hE2Dist", "Energy Dist. in case 2track/evt; E(Gev);Tracks", 100, 0, 20);
  m_hChi2 = getHist("hChi2", "#chi^{2} of tracks;#chi^{2};Tracks", 400, 0, 400);
  m_hPhi0 = getHist("hPhi0", "#Phi_{0} of tracks;#phi_{0} (Degree);Tracks", 400, -190, 190);
  m_hAlpha = getHist("hAlpha", "#alpha Dist.;#alpha (Degree);Hits", 360, -90, 90);
  m_hTheta = getHist("hTheta", "#theta Dist.;#theta (Degree);Hits", 360, 0, 180);
  m_hPval = getHist("hPval", "p-values of tracks;pVal;Tracks", 1000, 0, 1);

  m_hTriggerHitZX =  getHist("TriggerHitZX", "Hit Position on trigger counter;z(cm);x(cm)", 300, -100, 100, 120, -15, 15);
  m_h2DHitDistInCDCHit =  getHist("2DHitDistInCDCHit", " CDCHit;WireID;LayerID", m_up.at(8) - m_low.at(0), m_low.at(0), m_up.at(8),
                                  56, 0, 56);
  m_h2DHitDistInTrCand =  getHist("2DHitDistInTrCand", "Track Cand ;WireID;LayerID", m_up.at(8) - m_low.at(0), m_low.at(0),
                                  m_up.at(8), 56, 0, 56);
  m_h2DHitDistInTrack =   getHist("2DHitDistInTrack", "Fitted Track ;WireID;LayerID", m_up.at(8) - m_low.at(0), m_low.at(0),
                                  m_up.at(8), 56, 0, 56);
  if (m_fillExpertHistos) {
    m_hNDFChi2 = getHist("hNDFChi2", "#chi^{2} of tracks;NDF;#chi^{2};Tracks", 8, 0, 8, 800, 0, 200);
    m_hNDFPval = getHist("hNDFPval", "p-values of tracks;NDF;pVal;Tracks", 8, 0, 8, 100, 0, 1);
  }
  int sl;
  for (int i = 0; i < 56; ++i) {
    int iLayer = i + firstLayer;
    std::string title, name;
    if (iLayer < 8) {sl = 0;} else { sl = floor((iLayer - 8) / 6) + 1;}
    m_hHitDistInCDCHit[i] = getHist(Form("hHitDistInCDCHit_layer%d", iLayer), Form("Hit Dist. ICLayer_%d;WireID;#Hits", iLayer),
                                    m_up.at(sl) - m_low.at(sl), m_low.at(sl), m_up.at(sl));
    m_hHitDistInCDCHit[i]->SetLineColor(kGreen);
    m_hHitDistInTrCand[i] = getHist(Form("hHitDistInTrCand_layer%d", iLayer), Form("Hit Dist. ICLayer_%d;WireID;#Hits", iLayer),
                                    m_up.at(sl) - m_low.at(sl), m_low.at(sl), m_up.at(sl));
    m_hHitDistInTrCand[i]->SetLineColor(kRed);
    m_hHitDistInTrack[i] = getHist(Form("hHitDistInTrack_layer%d", iLayer), Form("Hit Dist. ICLayer_%d;WireID;#Hits", iLayer),
                                   m_up.at(sl) - m_low.at(sl), m_low.at(sl), m_up.at(sl));
    const  double normResRange = 20;
    const double residualRange = 0.3;
    if (m_plotResidual) {
      name = (boost::format("hist_ResidualsU%1%") % iLayer).str();
      title = (boost::format("unnormalized, unbiased residuals in layer %1%;cm;Tracks") % iLayer).str();
      m_hResidualU[i] = getHist(name, title, 500, -residualRange, residualRange);

      name = (boost::format("hNormalizedResidualsU%1%") % iLayer).str();
      title = (boost::format("normalized, unbiased residuals in layer %1%;NDF;#sigma (cm);Tracks") % iLayer).str();
      m_hNormalizedResidualU[i] = getHist(name, title, 500, -normResRange, normResRange);

      name = (boost::format("DxDt%1%") % iLayer).str();
      title = (boost::format("Drift Length vs Drift time at Layer_%1%;Drift Length (cm);Drift time (ns)") % iLayer).str();
      m_hDxDt[i] = getHist(name, title, 200, -1, 1, 450, -50, 400);
    }
    if (m_fillExpertHistos) {
      name = (boost::format("hNDFResidualsU%1%") % iLayer).str();
      title = (boost::format("unnormalized, unbiased residuals along U in layer %1%;NDF;cm;Tracks") % iLayer).str();
      m_hNDFResidualU[i] = getHist(name, title, 8, 0, 8, 1000, -residualRange, residualRange);

      name = (boost::format("hNDFNormalizedResidualsU%1%") % iLayer).str();
      title = (boost::format("normalized, unbiased residuals in layer %1%;NDF;#sigma (cm);Tracks") % iLayer).str();
      m_hNDFNormalizedResidualU[i] = getHist(name, title, 8, 0, 8, 1000, -normResRange, normResRange);
    }
  }
  oldDir->cd();


}

void CDCCRTestModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM
  StoreArray<Belle2::Track> storeTrack(m_trackArrayName);
  StoreArray<RecoTrack> recoTracks(m_recoTrackArrayName);
  StoreArray<Belle2::TrackFitResult> storeTrackFitResults(m_trackFitResultArrayName);
  StoreArray<Belle2::CDCHit> cdcHits(m_cdcHitArrayName);
  RelationArray relRecoTrackTrack(recoTracks, storeTrack, m_relRecoTrackTrackName);
  //Store names to speed up creation later
  m_recoTrackArrayName = recoTracks.getName();
  m_trackFitResultArrayName = storeTrackFitResults.getName();
  m_relRecoTrackTrackName = relRecoTrackTrack.getName();

  for (size_t i = 0; i < m_allHistos.size(); ++i) {
    m_allHistos[i]->Reset();
  }

  B2INFO("Trigger Position (" << m_TriggerPos.at(0) << " ," << m_TriggerPos.at(1) << " ," << m_TriggerPos.at(2) << ")");
}

void CDCCRTestModule::beginRun()
{
}

void CDCCRTestModule::event()
{
  const StoreArray<Belle2::Track> storeTrack(m_trackArrayName);
  const StoreArray<Belle2::TrackFitResult> storeTrackFitResults(m_trackFitResultArrayName);
  const StoreArray<Belle2::CDCHit> cdcHits(m_cdcHitArrayName);
  const StoreArray<Belle2::RecoTrack> recoTracks(m_recoTrackArrayName);
  const RelationArray relTrackTrack(recoTracks, storeTrack, m_relRecoTrackTrackName);

  /* CDCHit distribution */
  for (int i = 0; i < cdcHits.getEntries(); ++i) {
    Belle2::CDCHit* hit = cdcHits[i];
    m_hHitDistInCDCHit[getICLayer(hit->getISuperLayer(), hit->getILayer())]->Fill(hit->getIWire());
    m_h2DHitDistInCDCHit->Fill(hit->getIWire(), getICLayer(hit->getISuperLayer(), hit->getILayer()));
  }

  // Loop over Recotracks
  int nTr = recoTracks.getEntries();
  m_hNTracksPerEvent->Fill(nTr);

  int nfitted = 0;

  for (int i = 0; i < nTr; ++i) {
    const RecoTrack* track = recoTracks[i];
    m_hNHits_trackcand->Fill(track->getNumberOfCDCHits());
    getHitDistInTrackCand(track);

    if (!track->getTrackFitStatus()->isFitted()) {
      m_hNTracks->Fill("TrackCand, but no Track", 1.0);
      continue;
    }

    const genfit::FitStatus* fs = track->getTrackFitStatus();
    if (!fs || !fs->isFitConverged()) {//not fully convergence
      m_hNTracks->Fill("fitted, not converged", 1.0);
      B2DEBUG(99, "------Fitted but not converged");
      continue;
    }

    m_hNTracks->Fill("fitted, converged", 1.0);
    B2DEBUG(99, "-------Fittted and Conveged");

    nfitted = nfitted + 1;
    /** find results in track fit results**/
    const Belle2::Track* b2track = track->getRelatedFrom<Belle2::Track>();
    if (!b2track) {B2DEBUG(99, "No relation found"); continue;}
    fitresult = b2track->getTrackFitResult(Const::muon);

    if (!fitresult) {
      B2WARNING("track was fitted but Relation not found");
      continue;
    }

    if (nTr == 1) m_hE1Dist->Fill(fitresult->getEnergy());
    if (nTr == 2) m_hE2Dist->Fill(fitresult->getEnergy());

    if (m_noBFit) {ndf = fs->getNdf() + 1;} // incase no Magnetic field, NDF=4;
    else {ndf = fs->getNdf();}
    Chi2 = fs->getChi2();
    TrPval = std::max(0., ROOT::Math::chisquared_cdf_c(Chi2, ndf));
    Phi0 = fitresult->getPhi0() * 180 / M_PI;

    m_hPhi0->Fill(Phi0);
    m_hPval->Fill(TrPval);
    m_hNDF->Fill(ndf);
    m_hChi2->Fill(fs->getChi2());
    if (m_fillExpertHistos) {
      m_hNDFChi2->Fill(ndf, fs->getChi2());
      m_hNDFPval->Fill(ndf, TrPval);
    }
    try {
      plotResults(track);
    } catch (...) {
    }
  }
  m_hNTracksPerEventFitted->Fill(nfitted);
}

void CDCCRTestModule::endRun()
{
}

void CDCCRTestModule::terminate()
{
}

void CDCCRTestModule::plotResults(const Belle2::RecoTrack* track)
{
  const  Helix h = fitresult->getHelix();
  trigHitPos = getTriggerHitPosition(h, m_TriggerPos.at(1));
  trigHitPos_x = trigHitPos.X(); trigHitPos_z = trigHitPos.Z();
  m_hTriggerHitZX->Fill(trigHitPos.Z(), trigHitPos.X());
  bool hittrig = (fabs(trigHitPos.X() - m_TriggerPos.at(0)) < 3.5 + 0.5
                  && fabs(trigHitPos.Z() - m_TriggerPos.at(2)) < 0.5 * 12.5 + 0.75) ? true : false;
  //  bool hitCathode =(fabs(trigHitPos.X()-m_TriggerPos.at(0)) < 2.6 && fabs(trigHitPos.Z()-(m_TriggerPos.at(2)-27.3+1.75)) <1.77) ? true : false;
  if (hittrig) {trighit = 1;}
  else {trighit = 0;}

  if (m_hitEfficiency) {
    if (ndf > 12 && TrPval > 0.001) {HitEfficiency(h);}
  }

  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  tdcBinWidth = cdcgeo.getTdcBinWidth();
  m_hNHits->Fill(track->getNumberOfCDCHits());

  std::vector<genfit::TrackPoint*> tps = track->getHitPointsWithMeasurement();
  numhits = tps.size();

  BOOST_FOREACH(genfit::TrackPoint * tp, tps) {
    if (!tp->hasRawMeasurements())
      continue;

    const genfit::AbsMeasurement* raw = tp->getRawMeasurement(0);
    const CDCRecoHit* rawCDC = dynamic_cast<const CDCRecoHit*>(raw);
    if (rawCDC) {
      WireID wireid = rawCDC->getWireID();
      t0 =  cdcgeo.getT0(rawCDC->getWireID());

      const genfit::KalmanFitterInfo* kfi = tp->getKalmanFitterInfo();
      if (!kfi) continue;

      for (unsigned int iMeas = 0; iMeas < kfi->getNumMeasurements(); ++iMeas) {
        if ((kfi->getWeights().at(iMeas))  > 0.5) {
          const genfit::MeasurementOnPlane& residual_b = kfi->getResidual(iMeas, true);
          const genfit::MeasurementOnPlane& residual_u = kfi->getResidual(iMeas, false);
          lay = wireid.getICLayer();
          IWire = wireid.getIWire();
          m_hHitDistInTrack[lay]->Fill(IWire);
          m_h2DHitDistInTrack->Fill(IWire, lay);

          boardID = cdcgeo.getBoardID(wireid);
          lr = rawCDC->getLeftRightResolution();
          unsigned short lr1;
          if (lr == 1) {lr1 = lr;}
          else {lr1 = 0;}
          B2DEBUG(199, "left right from RawCDC " << lr << " |converted to " << lr1);
          Pval = TrPval;
          tdc = rawCDC->getCDCHit()->getTDCCount();
          adc = rawCDC->getCDCHit()->getADCCount();
          x_b = kfi->getFittedState(true).getState()(3);// x mea for biased
          res_b = residual_b.getState()(0);
          x_u = kfi->getFittedState(false).getState()(3);//x mea for unbiased
          res_u = residual_u.getState()(0);
          //B2INFO("resi V " <<residual.getState()(1));
          //    weight_res = residual.getWeight();
          absRes_b = std::abs(x_b + res_b) - std::abs(x_b);
          absRes_u = std::abs(x_u + res_u) - std::abs(x_u);
          weight = residual_u.getWeight();
          res_b_err = std::sqrt(residual_b.getCov()(0, 0));
          res_u_err = std::sqrt(residual_u.getCov()(0, 0));

          const genfit::MeasuredStateOnPlane& mop = kfi->getFittedState();
          // uses the plane determined by the track fit.
          // bool blr =true;
          //if (lr==1) blr=true;
          // if (lr==-1) blr=false;
          //          B2Vector3D pocaOnWire = mop.getPlane()->getO();
          const TVector3 pocaOnWire = mop.getPlane()->getO();
          const TVector3 pocaMom = mop.getMom();
          alpha = cdcgeo.getAlpha(pocaOnWire, pocaMom) * 180 / M_PI;
          theta = cdcgeo.getTheta(pocaMom) * 180 / M_PI;
          m_hAlpha->Fill(alpha);
          m_hTheta->Fill(theta);

          t = cdcgeo.getT0(wireid) - tdc * cdcgeo.getTdcBinWidth(); // - dt_flight - dt_prop;

          //estimate length for propagation
          double z = pocaOnWire.Z();
          TVector3 m_backWirePos = cdcgeo.wireBackwardPosition(wireid, CDCGeometryPar::c_Aligned);
          z_prop = z - m_backWirePos.Z();
          B2DEBUG(99, "z_prop = " << z_prop << " |z " << z << " |back wire poss: " << m_backWirePos.Z());
          //flight length for ToF correction
          const double* rinnerlayer = cdcgeo.innerRadiusWireLayer();
          const double* routerlayer = cdcgeo.outerRadiusWireLayer();

          double rcell = (rinnerlayer[wireid.getICLayer()] + routerlayer[wireid.getICLayer()]) / 2;
          double s2D = h.getArcLength2DAtCylindricalR(rcell);
          r_flight = s2D * hypot(1, h.getTanLambda());
          dt_flight = mop.getTime();
          if (m_IncomingToF && Phi0 > 0.) {dt_flight *= -1; }

          if (r_flight < 500 && m_ToF) {t -= dt_flight;}
          //    else{B2WARNING("Flight length larger than cdc volume "<<r_flight); dt_flight}

          /**Propagation Time**/
          dt_prop = z_prop * cdcgeo.getPropSpeedInv(lay);


          if (z_prop < 240 && m_ToP) {t -= dt_prop;}

          t -= cdcgeo.getTimeWalk(wireid, adc);

          if (m_calExpectedDriftTime) { t_fit = cdcgeo.getDriftTime(std::abs(x_u), lay, lr1, alpha * M_PI / 180, theta * M_PI / 180);}
          //    t = getCorrectedDriftTime(wireid, tdc, adc, z, z0);
          B2DEBUG(1, "MoP->getTime: " << mop.getTime() << " flight time:" << dt_flight << " proptime: " << dt_prop << " Driftime: " << t);
          tree->Fill();

          if (m_plotResidual) {
            m_hDxDt[lay]->Fill(x_u, t);
            m_hResidualU[lay]->Fill(res_b, weight);
            m_hNormalizedResidualU[lay]->Fill(res_b / sqrt(residual_b.getCov()(0, 0)), weight);
          }
          if (m_fillExpertHistos) {
            m_hNDFResidualU[lay]->Fill(ndf, res_b, weight);
            m_hNDFResidualU[lay]->Fill(ndf, res_b);
            m_hNDFNormalizedResidualU[lay]->Fill(ndf, res_b / std::sqrt(residual_b.getCov()(0, 0)), weight);
          }
        } //NDF
        // }//end of if isU
      }//end of for
    }//end of rawCDC
  }//end of for tp
}//end of func


double CDCCRTestModule::getCorrectedDriftTime(Belle2::WireID wireid, unsigned short tdc, unsigned short adc , double z, double z0)
{
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  double t0 =  cdcgeo.getT0(wireid);
  double  tdcBinWidth = cdcgeo.getTdcBinWidth();
  double DriftT = t0 - tdcBinWidth * tdc;
  TVector3 m_backWirePos = cdcgeo.wireBackwardPosition(wireid, CDCGeometryPar::c_Aligned);
  const double* rinnerlayer = cdcgeo.innerRadiusWireLayer();
  const double* routerlayer = cdcgeo.outerRadiusWireLayer();
  double  rcell = (rinnerlayer[wireid.getICLayer()] + routerlayer[wireid.getICLayer()]) / 2;
  double  flightLength = sqrt(rcell * rcell + (z - z0) * (z - z0));
  if (flightLength > 200) {
    B2WARNING("flight Length from closet point to hit may be larger than det size " << flightLength
              << "| rcell :" << rcell
              << "| z: " << z);
  }
  B2DEBUG(99, "| rcell :" << rcell << "| z: " << z << " |flightLength" << flightLength);
  //subtract distance divided by speed of electric signal in the wire from the drift time.
  DriftT -= (z - m_backWirePos.Z()) * cdcgeo.getPropSpeedInv(wireid.getICLayer());
  //Correct time of Flight, tentative using distance form hit to IP divide V
  //  DriftT -= mop.getTime();//trackTime
  DriftT -= flightLength / 30; //assume vflight=30cm/ns; flight from IP to this wire;
  if (false) {DriftT -= cdcgeo.getTimeWalk(wireid, adc);}
  return DriftT;
}

void CDCCRTestModule::getHitDistInTrackCand(const RecoTrack* track)
{
  //  std::vector< Belle2::RecoTrack::UsedCDCHit*>cdchits = track->getCDCHitList();
  //    StoreArray<Belle2::CDCHit> cdcHits;
  //  std::vector<int> cdchit_indicates = cand->getHitIDs(Belle2::Const::CDC);
  //  B2DEBUG(199,"numhit in trackcand: "<<cdchit_indicates.size());
  BOOST_FOREACH(const RecoHitInformation::UsedCDCHit * cdchit, track->getCDCHitList()) {
    //  const Belle2::CDCHit* cdchit = cdcHits[ic];
    int iclay = getICLayer(cdchit->getISuperLayer(), cdchit->getILayer());
    B2DEBUG(99, "In TrackCand: ICLayer: " << iclay << "IWire: " << cdchit->getIWire());
    m_hHitDistInTrCand[iclay - firstLayer]->Fill(cdchit->getIWire());
    m_h2DHitDistInTrCand->Fill(cdchit->getIWire(), iclay);
  }
}

TVector3 CDCCRTestModule::getTriggerHitPosition(const Helix h, double yofcounter = -13.25)
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
void CDCCRTestModule::HitEfficiency(const Helix h)
{
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  const double* rinnerlayer = cdcgeo.innerRadiusWireLayer();
  const double* routerlayer = cdcgeo.outerRadiusWireLayer();
  for (int i = 0; i < 56; ++i) {
    double  rcell = (rinnerlayer[i] + routerlayer[i]) / 2;
    double arcL = h.getArcLength2DAtCylindricalR(rcell);
    const TVector3 hitpos = h.getPositionAtArcLength2D(arcL);
    int cellID = cdcgeo.cellId(i, hitpos);
    B2INFO("Hit at LayerID - CellID: " << i << "-" << cellID);
  }
}
