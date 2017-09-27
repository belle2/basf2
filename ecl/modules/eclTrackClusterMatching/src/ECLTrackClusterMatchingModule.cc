/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 * Contributors: Guglielmo De Nardo, Frank Meier                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclTrackClusterMatching/ECLTrackClusterMatchingModule.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <framework/datastore/RelationVector.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <set>

using namespace std;
using namespace Belle2;
using namespace ECL;

REG_MODULE(ECLTrackClusterMatching)

ECLTrackClusterMatchingModule::ECLTrackClusterMatchingModule() : Module()
{
  setDescription("Set the Track --> ECLShower and ECLCluster Relations.");
  setPropertyFlags(c_ParallelProcessingCertified);
}

ECLTrackClusterMatchingModule::~ECLTrackClusterMatchingModule()
{
}

void ECLTrackClusterMatchingModule::initialize()
{
  StoreArray<Track> tracks;
  StoreArray<ECLShower> eclShowers;
  StoreArray<ECLCluster> eclClusters;
  tracks.registerRelationTo(eclShowers);
  tracks.registerRelationTo(eclClusters);
}

void ECLTrackClusterMatchingModule::beginRun()
{
}

void ECLTrackClusterMatchingModule::event()
{
  StoreArray<Track> tracks;
  StoreArray<ECLShower> eclRecShowers;
  StoreArray<ECLCluster> eclClusters;
  StoreArray<ECLCalDigit> eclDigits;

  for (const Track& track : tracks) {

    //Unique shower ids related to this track
    set<int> uniqueShowerIds;

    // Find extrapolated track hits in the ECL, considering
    // only hit points where the track enters the crystal
    // note that more than one crystal belonging to more than one shower
    // can be found
    for (const auto& extHit : track.getRelationsTo<ExtHit>()) {
      if (checkPionECLEnterID(extHit)) continue;
      int copyid =  extHit.getCopyID();
      const int cell = copyid + 1;

      //Find ECLCalDigit with same cell ID as ExtHit
      const auto idigit = find_if(eclDigits.begin(), eclDigits.end(),
      [&](const ECLCalDigit & d) { return d.getCellId() == cell; }
                                 );
      //Couldn't find ECLCalDigit with same cell ID as the ExtHit
      if (idigit == eclDigits.end()) continue;

      //Save all unique shower IDs of the showers related to idigit
      for (auto& shower : idigit->getRelationsFrom<ECLShower>()) {
        bool inserted = (uniqueShowerIds.insert(shower.getUniqueId())).second;

        //If this track <-> shower relation hasn't been set yet, set it for the shower and the ECLCLuster
        if (!inserted) continue;

        shower.setIsTrack(true);
        track.addRelationTo(&shower);
        ECLCluster* cluster = shower.getRelatedFrom<ECLCluster>();
        if (cluster != nullptr) {
          cluster->setIsTrack(true);
          track.addRelationTo(cluster);
        }
      } //end loop on shower related to idigit
    } // end loop on ExtHit
  } // end loop on Tracks

  for (auto& shower : eclRecShowers) {
    // compute the distance from shower COG and the closest extrapolated track
    double dist = computeTrkMinDistance(shower, tracks);
    shower.setMinTrkDistance(dist);
    ECLCluster* cluster = shower.getRelatedFrom<ECLCluster>();
    if (cluster != nullptr)
      cluster->setMinTrkDistance(float(dist));

    // compute path lenghts on the energy weighted average crystals direction
    // and on the extrapolated track direction corresponding to the minimum
    // distance among the two lines.
    // if more than one track is related to a shower the one with highest momentum is used
    double lTrk, lShower;
    if (shower.getIsTrack()) {
      computeDepth(shower, lTrk, lShower);
      B2DEBUG(150, "shower depth: ltrk = " << lTrk << " lShower = " << lShower);
      shower.setTrkDepth(lTrk);
      shower.setShowerDepth(lShower);
      if (cluster != nullptr)
        cluster->setdeltaL(lTrk);
    }
  }
}

void ECLTrackClusterMatchingModule::endRun()
{
}

void ECLTrackClusterMatchingModule::terminate()
{
}

double ECLTrackClusterMatchingModule::computeTrkMinDistance(const ECLShower& shower, StoreArray<Track>& tracks) const
{
  double minDist(10000);
  TVector3 cryCenter;
  cryCenter.SetMagThetaPhi(shower.getR(), shower.getTheta(), shower.getPhi());

  for (const auto& track : tracks) {
    TVector3 trkpos(0, 0, 0);
    bool found(false);
    for (const auto& extHit : track.getRelationsTo<ExtHit>()) {
      if (checkPionECLEnterID(extHit)) continue;
      trkpos = extHit.getPosition();
      found = true;
      break;
    }
    if (! found) continue;
    double distance = (cryCenter - trkpos).Mag();
    if (distance < minDist) minDist = distance;
  }
  if (minDist > 9999) minDist = -1;
  return minDist;
}

void ECLTrackClusterMatchingModule::computeDepth(const ECLShower& shower, double& lTrk, double& lShower) const
{
  ECLGeometryPar* geometry = ECLGeometryPar::Instance();
  TVector3 avgDir(0, 0, 0), showerCenter, trkpos, trkdir;
  showerCenter.SetMagThetaPhi(shower.getR(), shower.getTheta(), shower.getPhi());

  auto relatedDigitsPairs = shower.getRelationsTo<ECLCalDigit>();
  for (unsigned int iRel = 0; iRel < relatedDigitsPairs.size(); iRel++) {
    const auto aECLCalDigit = relatedDigitsPairs.object(iRel);
    const auto weight = relatedDigitsPairs.weight(iRel);
    double energy = weight * aECLCalDigit->getEnergy();
    int cellid = aECLCalDigit->getCellId();
    TVector3 cvec   = geometry->GetCrystalVec(cellid - 1);
    avgDir += weight * energy * cvec;
  }
  double p = 0;
  const Track* selectedTrk = nullptr;
  for (const auto& track : shower.getRelationsFrom<Track>()) {
    const TrackFitResult* fit = track.getTrackFitResult(Const::pion);
    double cp = 0;
    if (fit != 0) cp = fit->getMomentum().Mag();
    if (cp > p) {
      selectedTrk = & track;
      p = cp;
    }
  }
  lTrk = 0;
  lShower = 0;
  if (selectedTrk == nullptr) return;
  bool found(false);
  for (const auto& extHit : selectedTrk->getRelationsTo<ExtHit>()) {
    if (checkPionECLEnterID(extHit)) continue;
    trkpos = extHit.getPosition();
    trkdir = extHit.getMomentum().Unit();
    found = true;
    break;
  }
  if (!found) return;
  TVector3 w0 = showerCenter - trkpos;
  double costh = avgDir * trkdir;
  double sin2th = 1 - costh * costh;
  lShower = costh * (w0 * trkdir) - w0 * avgDir;
  lShower /= sin2th;

  lTrk = w0 * trkdir - costh * (w0 * avgDir);
  lTrk /= sin2th;
}

bool ECLTrackClusterMatchingModule::checkPionECLEnterID(const ExtHit& extHit) const
{
  if (abs(extHit.getPdgCode()) != Const::pion.getPDGCode()) return true;
  else if ((extHit.getDetectorID() != Const::EDetector::ECL)) return true;
  else if (extHit.getStatus() != EXT_ENTER) return true;
  else if (extHit.getCopyID() == -1) return true;
  else return false;
}