/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/modules/eclClusterProperties/ECLClusterPropertiesModule.h>

/* ECL headers. */
#include <ecl/geometry/ECLGeometryPar.h>

/* Basf2 headers. */
#include <framework/geometry/VectorUtil.h>

/* ROOT headers. */
#include <Math/Vector3D.h>

using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLClusterProperties);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLClusterPropertiesModule::ECLClusterPropertiesModule() : Module()
{
  // Set module properties
  setDescription("This module calculates some properties of ECL clusters.");
  setPropertyFlags(c_ParallelProcessingCertified);
  // Parameter definitions
  addParam("trackClusterRelationName", m_trackClusterRelationName, "Name of relation array between tracks and ECL clusters",
           std::string("AngularDistance"));
}

ECLClusterPropertiesModule::~ECLClusterPropertiesModule()
{
}

void ECLClusterPropertiesModule::initialize()
{
  m_tracks.isRequired();
  m_eclShowers.isRequired();
  m_eclClusters.isRequired();
  m_eclCalDigits.isRequired();
  m_extHits.isRequired();
}

void ECLClusterPropertiesModule::event()
{
  for (auto& shower : m_eclShowers) {
    // compute the distance from shower COG and the closest extrapolated track
    unsigned short trackID = std::numeric_limits<unsigned short>::max();
    double dist = computeTrkMinDistance(shower, m_tracks, trackID);
    shower.setMinTrkDistance(dist);
    ECLCluster* cluster = shower.getRelatedFrom<ECLCluster>();
    if (cluster != nullptr) {
      cluster->setMinTrkDistance(float(dist));
      cluster->setMinTrkDistanceID(trackID);
      // compute path lenghts on the energy weighted average crystals
      // direction and on the extrapolated track direction corresponding to
      // the minimum distance among the two lines. if more than one track is
      // related to a cluster the one with the highest momentum is used
      if (cluster->isTrack()) {
        double lTrk, lShower;
        computeDepth(shower, lTrk, lShower);
        B2DEBUG(29, "shower depth: ltrk = " << lTrk << " lShower = " << lShower);
        shower.setTrkDepth(lTrk);
        shower.setShowerDepth(lShower);
        cluster->setdeltaL(lTrk);
      }
    }
  }
}

double ECLClusterPropertiesModule::computeTrkMinDistance(const ECLShower& shower, StoreArray<Track>& tracks,
                                                         unsigned short& trackID) const
{
  double minDist(10000);
  ROOT::Math::XYZVector cryCenter;
  VectorUtil::setMagThetaPhi(
    cryCenter, shower.getR(), shower.getTheta(), shower.getPhi());
  Const::ChargedStable hypothesis = Const::pion;
  int pdgCode = abs(hypothesis.getPDGCode());
  for (const auto& track : tracks) {
    ROOT::Math::XYZVector trkpos;
    for (const auto& extHit : track.getRelationsTo<ExtHit>()) {
      if (abs(extHit.getPdgCode()) != pdgCode) continue;
      if ((extHit.getDetectorID() !=  Const::EDetector::ECL)) continue;
      if (extHit.getCopyID() == -1) continue;
      trkpos = extHit.getPosition();
      double distance = (cryCenter - trkpos).R();
      if (distance < minDist) {
        trackID = track.getArrayIndex();
        minDist = distance;
      }
    }
  }
  if (minDist > 9999) minDist = -1;
  return minDist;
}

void ECLClusterPropertiesModule::computeDepth(const ECLShower& shower, double& lTrk, double& lShower) const
{
  lTrk = 0;
  lShower = 0;
  ECLGeometryPar* geometry = ECLGeometryPar::Instance();
  ROOT::Math::XYZVector avgDir(0, 0, 0), showerCenter, trkpos, trkdir;
  VectorUtil::setMagThetaPhi(
    showerCenter, shower.getR(), shower.getTheta(), shower.getPhi());

  auto relatedDigitsPairs = shower.getRelationsTo<ECLCalDigit>();
  for (unsigned int iRel = 0; iRel < relatedDigitsPairs.size(); iRel++) {
    const auto aECLCalDigit = relatedDigitsPairs.object(iRel);
    const auto weight = relatedDigitsPairs.weight(iRel);
    double energy = weight * aECLCalDigit->getEnergy();
    int cellid = aECLCalDigit->getCellId();
    ROOT::Math::XYZVector cvec = geometry->GetCrystalVec(cellid - 1);
    avgDir += energy * cvec;
  }
  const ECLCluster* cluster = shower.getRelatedFrom<ECLCluster>();
  if (cluster == nullptr) return;
  const Track* selectedTrk = nullptr;
  double p = 0;
  for (const auto& track : cluster->getRelationsFrom<Track>("", m_trackClusterRelationName)) {
    const TrackFitResult* fit = track.getTrackFitResultWithClosestMass(Const::pion);
    double cp = 0;
    if (fit != 0) cp = fit->getMomentum().R();
    if (cp > p) {
      selectedTrk = &track;
      p = cp;
    }
  }
  if (selectedTrk == nullptr) return;
  bool found(false);
  for (const auto& extHit : selectedTrk->getRelationsTo<ExtHit>()) {
    if ((extHit.getDetectorID() !=  Const::EDetector::ECL)) continue;
    if (extHit.getStatus() != EXT_ENTER) continue;
    if (extHit.getCopyID() == -1) continue;
    trkpos = extHit.getPosition();
    trkdir = extHit.getMomentum().Unit();
    found = true;
    break;
  }
  if (!found) return;
  ROOT::Math::XYZVector w0 = showerCenter - trkpos;
  double costh = avgDir.Unit().Dot(trkdir);
  double sin2th = 1 - costh * costh;
  lShower = costh * w0.Dot(trkdir) - w0.Dot(avgDir.Unit());
  lShower /= sin2th;

  lTrk = w0.Dot(trkdir) - costh * w0.Dot(avgDir.Unit());
  lTrk /= sin2th;
}
