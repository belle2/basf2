/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/modules/cdcCalibrationCollector/CDCBadWireCollector.h>
#include <cdc/translators/RealisticTDCCountTranslator.h>
#include <framework/datastore/RelationArray.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <genfit/TrackPoint.h>
#include <genfit/KalmanFitterInfo.h>
#include <genfit/MeasurementOnPlane.h>
#include <genfit/MeasuredStateOnPlane.h>
//#include <Math/ProbFuncMathCore.h>
#include <cdc/dataobjects/WireID.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <TH1F.h>

//using namespace std;
using namespace Belle2;
using namespace CDC;
using namespace genfit;
using namespace TrackFindingCDC;


REG_MODULE(CDCBadWireCollector);


CDCBadWireCollectorModule::CDCBadWireCollectorModule() : CalibrationCollectorModule()
{
  setDescription("Collector module for cdc calibration");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("minimumPt", m_minimumPt, "Tracks with tranverse momentum smaller than this value will not used", 1.0);
  addParam("minimumNDF", m_minimumNDF, "Discard tracks whose degree-of-freedom below this value", 5.);
}

CDCBadWireCollectorModule::~CDCBadWireCollectorModule()
{
}

void CDCBadWireCollectorModule::prepare()
{
  m_Tracks.isRequired(m_trackArrayName);
  m_TrackFitResults.isRequired(m_trackFitResultArrayName);
  m_CDCHits.isRequired(m_cdcHitArrayName);
  m_CDCTracks.isRequired(m_cdcTrackVectorName);

  auto m_efftree  = new TTree(m_effTreeName.c_str(), "tree for wire efficiency");
  m_efftree->Branch<unsigned short>("layerID", &layerID);
  m_efftree->Branch<unsigned short>("wireID", &wireID);
  m_efftree->Branch<float>("z", &z);
  m_efftree->Branch<bool>("isFound", &isFound);
  registerObject<TTree>("efftree", m_efftree);

  auto m_hNDF = new TH1F("hNDF", "NDF of fitted track;NDF;Tracks", 71, -1, 70);
  auto m_hPval = new TH1F("hPval", "p-values of tracks;pVal;Tracks", 1000, 0, 1);

  registerObject<TH1F>("hNDF", m_hNDF);
  registerObject<TH1F>("hPval", m_hPval);
}

void CDCBadWireCollectorModule::collect()
{
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
  for (int i = 0; i < nTr; ++i) {
    const Belle2::Track* b2track = m_Tracks[i];
    const Belle2::TrackFitResult* fitresult = b2track->getTrackFitResultWithClosestMass(Const::muon);
    if (!fitresult) {
      B2WARNING("No track fit result found.");
      continue;
    }

    double ndf = fitresult->getNDF();
    double pval = fitresult->getPValue();
    double pt = fitresult->getTransverseMomentum();
    double d0 = fitresult->getD0();
    double z0 = fitresult->getZ0();
    getObjectPtr<TH1F>("hPval")->Fill(pval);
    getObjectPtr<TH1F>("hNDF")->Fill(ndf);

    // Cut on NDF, Pt
    if (ndf < m_minimumNDF) continue;
    if (pt < m_minimumPt) continue;

    // Request tracks coming from IP
    if (d0 > 2 || z0 > 5) continue;

    const Helix helixFit = fitresult->getHelix();
    buildEfficiencies(wiresInCDCTrack, helixFit);
  }
}

void CDCBadWireCollectorModule::finish() {}
const CDCWire& CDCBadWireCollectorModule::getIntersectingWire(const TVector3& xyz, const CDCWireLayer& layer,
    const Helix& helixFit) const
{
  Vector3D crosspoint;
  if (layer.isAxial())
    crosspoint = Vector3D(xyz);
  else {
    const CDCWire& oneWire = layer.getWire(1);
    double newR = oneWire.getWirePos2DAtZ(xyz.Z()).norm();
    double arcLength = helixFit.getArcLength2DAtCylindricalR(newR);
    TVector3 xyzOnWire = B2Vector3D(helixFit.getPositionAtArcLength2D(arcLength));
    crosspoint = Vector3D(xyzOnWire);
  }
  const CDCWire& wire = layer.getClosestWire(crosspoint);
  return wire;
}

void CDCBadWireCollectorModule::buildEfficiencies(std::vector<unsigned short> wireHits, const Helix helixFit)
{
  static const TrackFindingCDC::CDCWireTopology& wireTopology = CDCWireTopology::getInstance();
  for (const CDCWireLayer& wireLayer : wireTopology.getWireLayers()) {
    const double radiusofLayer = wireLayer.getRefCylindricalR();
    //simple extrapolation of fit
    const double arcLength = helixFit.getArcLength2DAtCylindricalR(radiusofLayer);
    const TVector3 xyz = B2Vector3D(helixFit.getPositionAtArcLength2D(arcLength));
    if (!xyz.X()) continue;
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
    z = xyz.Z();
    getObjectPtr<TTree>("efftree")->Fill();
  }
}



