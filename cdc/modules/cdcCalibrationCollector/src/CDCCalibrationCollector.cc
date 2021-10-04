/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "cdc/modules/cdcCalibrationCollector/CDCCalibrationCollector.h"
#include <cdc/translators/RealisticTDCCountTranslator.h>
#include <framework/datastore/RelationArray.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <genfit/TrackPoint.h>
#include <genfit/KalmanFitterInfo.h>
#include <genfit/MeasurementOnPlane.h>
#include <genfit/MeasuredStateOnPlane.h>

#include <Math/ProbFuncMathCore.h>

#include <cdc/dataobjects/WireID.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <TH1F.h>

//using namespace std;
using namespace Belle2;
using namespace CDC;
using namespace genfit;
using namespace TrackFindingCDC;


REG_MODULE(CDCCalibrationCollector)


CDCCalibrationCollectorModule::CDCCalibrationCollectorModule() : CalibrationCollectorModule()
{
  setDescription("Collector module for cdc calibration");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("recoTracksColName", m_recoTrackArrayName, "Name of collection hold genfit::Track", std::string(""));
  addParam("bField", m_bField, "If true -> #Params ==5 else #params ==4 for calculate P-Val", false);
  addParam("calExpectedDriftTime", m_calExpectedDriftTime, "if true module will calculate expected drift time, it take a time",
           true);
  addParam("storeTrackParams", m_storeTrackParams, "Store Track Parameter or not, it will be multicount for each hit", false);
  addParam("eventT0Extraction", m_eventT0Extraction, "use event t0 extract t0 or not", true);
  addParam("minimumPt", m_minimumPt, "Tracks with tranverse momentum smaller than this value will not used", 0.15);
  addParam("minimumNDF", m_minimumNDF, "Discard tracks whose degree-of-freedom below this value", 5.);
  addParam("isCosmic", m_isCosmic, "True when we process cosmic events, else False (collision)", m_isCosmic);
  addParam("effStudy", m_effStudy, "When true module collects info only  necessary for wire eff study", false);
}

CDCCalibrationCollectorModule::~CDCCalibrationCollectorModule()
{
}

void CDCCalibrationCollectorModule::prepare()
{
  m_Tracks.isRequired(m_trackArrayName);
  m_RecoTracks.isRequired(m_recoTrackArrayName);
  m_TrackFitResults.isRequired(m_trackFitResultArrayName);
  m_CDCHits.isRequired(m_cdcHitArrayName);
  m_CDCTracks.isRequired(m_cdcTrackVectorName);
  RelationArray relRecoTrackTrack(m_RecoTracks, m_Tracks, m_relRecoTrackTrackName);
  //Store names to speed up creation later
  m_relRecoTrackTrackName = relRecoTrackTrack.getName();

  if (!m_effStudy) { // by default collects calibration data
    auto m_tree  = new TTree(m_treeName.c_str(), "tree for cdc calibration");
    m_tree->Branch<Float_t>("x_mea", &x_mea);
    m_tree->Branch<Float_t>("x_u", &x_u);
    m_tree->Branch<Float_t>("x_b", &x_b);
    m_tree->Branch<Float_t>("alpha", &alpha);
    m_tree->Branch<Float_t>("theta", &theta);
    m_tree->Branch<Float_t>("t", &t);
    m_tree->Branch<UShort_t>("adc", &adc);
    //  m_tree->Branch<int>("boardID", &boardID);
    m_tree->Branch<UChar_t>("lay", &lay);
    m_tree->Branch<Float_t>("weight", &weight);
    m_tree->Branch<UShort_t>("IWire", &IWire);
    m_tree->Branch<Float_t>("Pval", &Pval);
    m_tree->Branch<Float_t>("ndf", &ndf);
    if (m_storeTrackParams) {
      m_tree->Branch<Float_t>("d0", &d0);
      m_tree->Branch<Float_t>("z0", &z0);
      m_tree->Branch<Float_t>("phi0", &phi0);
      m_tree->Branch<Float_t>("tanL", &tanL);
      m_tree->Branch<Float_t>("omega", &omega);
    }

    if (m_calExpectedDriftTime) { // expected drift time, calculated form xfit
      m_tree->Branch<Float_t>("t_fit", &t_fit);
    }

    registerObject<TTree>("tree", m_tree);
  }
  if (m_effStudy) { //if m_effStudy is changed to true prepares to only run wire efficiency study
    auto m_efftree  = new TTree(m_effTreeName.c_str(), "tree for wire efficiency");
    m_efftree->Branch<unsigned short>("layerID", &layerID);
    m_efftree->Branch<unsigned short>("wireID", &wireID);
    m_efftree->Branch<float>("z", &z);
    m_efftree->Branch<bool>("isFound", &isFound);

    registerObject<TTree>("efftree", m_efftree);
  }

  auto m_hNDF = new TH1F("hNDF", "NDF of fitted track;NDF;Tracks", 71, -1, 70);
  auto m_hPval = new TH1F("hPval", "p-values of tracks;pVal;Tracks", 1000, 0, 1);
  auto m_hEventT0 = new TH1F("hEventT0", "Event T0", 1000, -100, 100);
  auto m_hNTracks = new TH1F("hNTracks", "Number of tracks", 50, 0, 10);
  auto m_hOccupancy = new TH1F("hOccupancy", "occupancy", 100, 0, 1.0);

  registerObject<TH1F>("hNDF", m_hNDF);
  registerObject<TH1F>("hPval", m_hPval);
  registerObject<TH1F>("hEventT0", m_hEventT0);
  registerObject<TH1F>("hNTracks", m_hNTracks);
  registerObject<TH1F>("hOccupancy", m_hOccupancy);
}

void CDCCalibrationCollectorModule::collect()
{
  const RelationArray relTrackTrack(m_RecoTracks, m_Tracks, m_relRecoTrackTrackName);

  /* CDCHit distribution */
  //  make evt t0 incase we dont use evt t0
  evtT0 = 0;

  //reject events don't have eventT0
  if (m_eventT0Extraction) {
    // event with is fail to extract t0 will be exclude from analysis
    if (m_eventTimeStoreObject.isValid() && m_eventTimeStoreObject->hasEventT0()) {
      evtT0 =  m_eventTimeStoreObject->getEventT0();
      getObjectPtr<TH1F>("hEventT0")->Fill(evtT0);
    } else {
      return;
    }
  }
  // Collects the WireID and Layer of every hit in this event
  // Used in wire efficiency building
  std::vector<unsigned short> wiresInCDCTrack;

  for (CDCTrack& cdcTrack : *m_CDCTracks) {
    for (CDCRecoHit3D& cdcHit : cdcTrack) {
      unsigned short eWireID = cdcHit.getWire().getEWire();
      wiresInCDCTrack.push_back(eWireID);
    }
  }
  // WireID collection finished

  const int nTr = m_Tracks.getEntries();
  // Skip events which have number of charged tracks <= 1.
  int nCTracks  = 0;
  for (int i = 0; i < nTr; ++i) {
    const Belle2::Track* b2track = m_Tracks[i];
    const Belle2::TrackFitResult* fitresult = b2track->getTrackFitResultWithClosestMass(Const::muon);
    if (!fitresult) continue;

    short charge = fitresult->getChargeSign();
    if (fabs(charge) > 0) {
      nCTracks++;
    }
  }

  if (nCTracks <= 1) {
    return ;
  } else {
    getObjectPtr<TH1F>("hNTracks")->Fill(nCTracks);
  }

  const int nHits = m_CDCHits.getEntries();
  const int nWires = 14336;
  float oc = static_cast<float>(nHits) / static_cast<float>(nWires);
  getObjectPtr<TH1F>("hOccupancy")->Fill(oc);

  for (int i = 0; i < nTr; ++i) {
    const Belle2::Track* b2track = m_Tracks[i];
    const Belle2::TrackFitResult* fitresult = b2track->getTrackFitResultWithClosestMass(Const::muon);
    if (!fitresult) {
      B2WARNING("No track fit result found.");
      continue;
    }

    Belle2::RecoTrack* recoTrack = b2track->getRelatedTo<Belle2::RecoTrack>(m_recoTrackArrayName);
    if (!recoTrack) {
      B2WARNING("Can not access RecoTrack of this Belle2::Track");
      continue;
    }
    const genfit::FitStatus* fs = recoTrack->getTrackFitStatus();
    if (!fs) continue;
    ndf = fs->getNdf();
    if (!m_bField) {
      ndf += 1;
    }

    getObjectPtr<TH1F>("hPval")->Fill(Pval);
    getObjectPtr<TH1F>("hNDF")->Fill(ndf);
    B2DEBUG(99, "ndf = " << ndf);
    B2DEBUG(99, "Pval = " << Pval);

    if (ndf < m_minimumNDF) continue;
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
    if (fitresult->getTransverseMomentum() < m_minimumPt) continue;
    if (!m_effStudy) { // all harvest to fill the tree if collecting calibration info
      try {
        harvest(recoTrack);
      } catch (const genfit::Exception& e) {
        B2ERROR("Exception when harvest information from recotrack: " << e.what());
      }
    }
    if (m_effStudy) { // call buildEfficiencies for efficiency study
      // Request tracks coming from IP
      if (fitresult->getD0() > 2 || fitresult->getZ0() > 5) continue;
      const Helix helixFit = fitresult->getHelix();
      buildEfficiencies(wiresInCDCTrack, helixFit);
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
    if (!tp) continue;
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
        if (m_calExpectedDriftTime) { t_fit = cdcgeo.getDriftTime(abs(x_u), lay, lr, alpha , theta);}
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

const CDCWire& CDCCalibrationCollectorModule::getIntersectingWire(const TVector3& xyz, const CDCWireLayer& layer,
    const Helix& helixFit) const
{
  Vector3D crosspoint;
  if (layer.isAxial())
    crosspoint = Vector3D(xyz);
  else {
    const CDCWire& oneWire = layer.getWire(1);
    double newR = oneWire.getWirePos2DAtZ(xyz.z()).norm();
    double arcLength = helixFit.getArcLength2DAtCylindricalR(newR);
    TVector3 xyzOnWire = helixFit.getPositionAtArcLength2D(arcLength);
    crosspoint = Vector3D(xyzOnWire);
  }

  const CDCWire& wire = layer.getClosestWire(crosspoint);

  return wire;
}

void CDCCalibrationCollectorModule::buildEfficiencies(std::vector<unsigned short> wireHits, const Helix helixFit)
{
  static const TrackFindingCDC::CDCWireTopology& wireTopology = CDCWireTopology::getInstance();
  for (const CDCWireLayer& wireLayer : wireTopology.getWireLayers()) {
    const double radiusofLayer = wireLayer.getRefCylindricalR();
    //simple extrapolation of fit
    const double arcLength = helixFit.getArcLength2DAtCylindricalR(radiusofLayer);
    const TVector3 xyz = helixFit.getPositionAtArcLength2D(arcLength);
    if (!xyz.x()) continue;
    const CDCWire& wireIntersected = getIntersectingWire(xyz, wireLayer, helixFit);
    unsigned short crossedWire = wireIntersected.getEWire();
    unsigned short crossedCWire = wireIntersected.getNeighborCW()->getEWire();
    unsigned short crossedCCWire = wireIntersected.getNeighborCCW()->getEWire();

    if (find(wireHits.begin(), wireHits.end(), crossedWire) != wireHits.end()
        || find(wireHits.begin(), wireHits.end(), crossedCWire) != wireHits.end()
        || find(wireHits.begin(), wireHits.end(), crossedCCWire) != wireHits.end())
      isFound = true;
    else
      isFound = false;

    wireID = wireIntersected.getIWire();
    layerID = wireIntersected.getICLayer();
    z = xyz.z();
    getObjectPtr<TTree>("efftree")->Fill();
  }
}



