/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Frank Meier                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclClusterProperties/ECLClusterPropertiesModule.h>
#include <ecl/geometry/ECLGeometryPar.h>

using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLClusterProperties)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLClusterPropertiesModule::ECLClusterPropertiesModule() : Module()
{
  // Set module properties
  setDescription("This module calculates some properties of ECL clusters.");
  setPropertyFlags(c_ParallelProcessingCertified);
  // Parameter definitions

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

void ECLClusterPropertiesModule::beginRun()
{
}

void ECLClusterPropertiesModule::event()
{
  for (auto& shower : m_eclShowers) {
    // compute the distance from shower COG and the closest extrapolated track
    double dist = computeTrkMinDistance(shower, m_tracks);
    shower.setMinTrkDistance(dist);
    ECLCluster* cluster = shower.getRelatedFrom<ECLCluster>();
    if (cluster != nullptr) {
      cluster->setMinTrkDistance(float(dist));
      // compute path lenghts on the energy weighted average crystals
      // direction and on the extrapolated track direction corresponding to
      // the minimum distance among the two lines. if more than one track is
      // related to a cluster the one with the highest momentum is used
      double lTrk, lShower;
      if (cluster->isTrack()) {
        computeDepth(shower, lTrk, lShower);
        B2DEBUG(150, "shower depth: ltrk = " << lTrk << " lShower = " << lShower);
        shower.setTrkDepth(lTrk);
        shower.setShowerDepth(lShower);
        cluster->setdeltaL(lTrk);
      }
    }
  }
}

void ECLClusterPropertiesModule::endRun()
{
}

void ECLClusterPropertiesModule::terminate()
{
}

double ECLClusterPropertiesModule::computeTrkMinDistance(const ECLShower& shower, StoreArray<Track>& tracks) const
{
  double minDist(10000);
  TVector3 cryCenter;
  cryCenter.SetMagThetaPhi(shower.getR(), shower.getTheta(), shower.getPhi());
  Const::ChargedStable hypothesis = Const::pion;
  int pdgCode = abs(hypothesis.getPDGCode());
  for (const auto& track : tracks) {
    TVector3 trkpos(0, 0, 0);
    for (const auto& extHit : track.getRelationsTo<ExtHit>()) {
      if (abs(extHit.getPdgCode()) != pdgCode) continue;
      if ((extHit.getDetectorID() !=  Const::EDetector::ECL)) continue;
      if (extHit.getCopyID() == -1) continue;
      trkpos = extHit.getPosition();
      double distance = (cryCenter - trkpos).Mag();
      if (distance < minDist) minDist = distance;
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
  TVector3 avgDir(0, 0, 0), showerCenter, trkpos, trkdir;
  showerCenter.SetMagThetaPhi(shower.getR(), shower.getTheta(), shower.getPhi());

  auto relatedDigitsPairs = shower.getRelationsTo<ECLCalDigit>();
  for (unsigned int iRel = 0; iRel < relatedDigitsPairs.size(); iRel++) {
    const auto aECLCalDigit = relatedDigitsPairs.object(iRel);
    const auto weight = relatedDigitsPairs.weight(iRel);
    double energy = weight * aECLCalDigit->getEnergy();
    int cellid = aECLCalDigit->getCellId();
    TVector3 cvec = geometry->GetCrystalVec(cellid - 1);
    avgDir += weight * energy * cvec; // CHECK: energy already contains weight, needed here again?
  }
  const ECLCluster* cluster = shower.getRelatedFrom<ECLCluster>();
  if (cluster == nullptr) return;
  const Track* selectedTrk = nullptr;
  double p = 0;
  for (const auto& track : cluster->getRelationsFrom<Track>()) {
    const TrackFitResult* fit = track.getTrackFitResult(Const::pion);
    double cp = 0;
    if (fit != 0) cp = fit->getMomentum().Mag();
    if (cp > p) {
      selectedTrk = &track;
      p = cp;
    }
  }
  if (selectedTrk == nullptr) return;
  bool found(false);
  Const::ChargedStable hypothesis = Const::pion;
  int pdgCode = abs(hypothesis.getPDGCode());
  for (const auto& extHit : selectedTrk->getRelationsTo<ExtHit>()) {
    if (abs(extHit.getPdgCode()) != pdgCode) continue;
    if ((extHit.getDetectorID() !=  Const::EDetector::ECL)) continue;
    if (extHit.getStatus() != EXT_ENTER) continue;
    if (extHit.getCopyID() == -1) continue;
    trkpos = extHit.getPosition();
    trkdir = extHit.getMomentum().Unit();
    found = true;
    break;
  }
  if (!found) return;
  TVector3 w0 = showerCenter - trkpos;
  double costh = avgDir * trkdir; // CHECK: how/where is avgDir being normalized?
  double sin2th = 1 - costh * costh;
  lShower = costh * (w0 * trkdir) - w0 * avgDir;
  lShower /= sin2th;

  lTrk = w0 * trkdir - costh * (w0 * avgDir);
  lTrk /= sin2th;
}

/*void ECLClusterPropertiesModule::computeDepth(const ECLShower& shower, double& lTrk, double& lShower) const
{
  lTrk = 0;
  lShower = 0;
  TVector3 showerCenter, enter, poca;
  showerCenter.SetMagThetaPhi(shower.getR(), shower.getTheta(), shower.getPhi());
  const ECLCluster* cluster = shower.getRelatedFrom<ECLCluster>();
  if (cluster == nullptr) return;
  const Track* selectedTrk = nullptr;
  double p = 0;
  for (const auto& track : cluster->getRelationsFrom<Track>()) {
    const TrackFitResult* fit = track.getTrackFitResult(Const::pion);
    double cp = 0;
    if (fit != 0) cp = fit->getMomentum().Mag();
    if (cp > p) {
      selectedTrk = &track;
      p = cp;
    }
  }
  if (selectedTrk == nullptr) return;
  bool found_enter(false), found_poca(false);
  Const::ChargedStable hypothesis = Const::pion;
  int pdgCode = abs(hypothesis.getPDGCode());
  for (const auto& extHit : selectedTrk->getRelationsTo<ExtHit>()) {
    if (abs(extHit.getPdgCode()) != pdgCode) continue;
    if ((extHit.getDetectorID() !=  Const::EDetector::ECL)) continue;
    if (extHit.getCopyID() == -1) continue;
    if (extHit.getStatus() == EXT_ENTER) {
      enter = extHit.getPosition();
      found_enter = true;
    }
    if (extHit.getStatus() == EXT_ECLDL) {
      poca = extHit.getPosition();
      found_poca = true;
    }
    if (found_enter && found_poca) break;
  }
  if (!(found_enter && found_poca)) return;
  lShower = (showerCenter - poca).Mag();
  lTrk = (enter -poca).Mag();
}*/