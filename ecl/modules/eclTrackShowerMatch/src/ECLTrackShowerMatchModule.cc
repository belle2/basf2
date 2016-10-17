/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 * Contributors: Guglielmo De Nardo (denardo@na.infn.it)                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclTrackShowerMatch/ECLTrackShowerMatchModule.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/ExtHit.h>
#include <framework/datastore/RelationVector.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <set>

using namespace std;
using namespace Belle2;
using namespace ECL;

REG_MODULE(ECLTrackShowerMatch)

ECLTrackShowerMatchModule::ECLTrackShowerMatchModule() : Module()
{
  setDescription("Set the Track --> ECLShower and ECLCluster Relations.");
  setPropertyFlags(c_ParallelProcessingCertified);
}

ECLTrackShowerMatchModule::~ECLTrackShowerMatchModule()
{
}

void ECLTrackShowerMatchModule::initialize()
{
  StoreArray<Track> tracks;
  StoreArray<ECLShower> eclShowers;
  StoreArray<ECLCluster> eclClusters;
  tracks.registerRelationTo(eclShowers);
  tracks.registerRelationTo(eclClusters);
}

void ECLTrackShowerMatchModule::beginRun()
{
}

void ECLTrackShowerMatchModule::event()
{
  StoreArray<Track> tracks;
  StoreArray<ECLShower> eclRecShowers;
  StoreArray<ECLCluster> eclClusters;
  StoreArray<ECLCalDigit> eclDigits;

  Const::EDetector myDetID = Const::EDetector::ECL;
  const int pdgCodePiPlus = Const::pion.getPDGCode();
  const int pdgCodePiMinus = -1 * Const::pion.getPDGCode();
  ECLGeometryPar* geometry = ECLGeometryPar::Instance();
  for (const Track& track : tracks) {
    set<int> clid;

    // Find extrapolated track hits in the ECL, considering
    // only hit points where the track enters the crystal
    // note that more than on crystal belonging to more than one shower
    // can be found
    for (const auto& extHit : track.getRelationsTo<ExtHit>()) {
      if (extHit.getPdgCode() != pdgCodePiPlus && extHit.getPdgCode() != pdgCodePiMinus) continue;
      if ((extHit.getDetectorID() != myDetID)) continue;
      if (extHit.getStatus() != EXT_ENTER) continue;
      int copyid =  extHit.getCopyID();
      if (copyid == -1) continue;
      const int cell = copyid + 1;
      TVector3 cpos   = geometry->GetCrystalPos(copyid);
      TVector3 trkpos = extHit.getPosition();
      const auto idigit = find_if(eclDigits.begin(), eclDigits.end(),
      [&](const ECLCalDigit & d) { return d.getCellId() == cell; }
                                 );
      if (idigit != eclDigits.end()) {
        RelationVector<ECLShower> eclShower = idigit->getRelationsFrom<ECLShower>();
        for (const auto& sh : eclShower) clid.insert(sh.getShowerId());
      }
    } // end loop on ExtHit

    // Make a track --> shower relation for every shower that has a digit been hit.
    for (const auto& id : clid) {
      const auto ish = find_if(eclRecShowers.begin(), eclRecShowers.end(),
      [&](const ECLShower & element) { return element.getShowerId() == id; }
                              );
      if (ish != eclRecShowers.end()) {
        ish->setIsTrack(true);
        const ECLShower* shower = &(*ish);
        track.addRelationTo(shower);
        ECLCluster* cluster = shower->getRelatedFrom<ECLCluster>();
        if (cluster != nullptr) {
          cluster->setIsTrack(true);
          track.addRelationTo(cluster);
        }
      }
    } // end loop on shower id
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

void ECLTrackShowerMatchModule::endRun()
{
}

void ECLTrackShowerMatchModule::terminate()
{
}

double ECLTrackShowerMatchModule::computeTrkMinDistance(const ECLShower& shower, StoreArray<Track>& tracks) const
{
  double minDist(10000);
  TVector3 cryCenter;
  cryCenter.SetMagThetaPhi(shower.getR(), shower.getTheta(), shower.getPhi());

  for (const auto& track : tracks) {
    TVector3 trkpos(0, 0, 0);
    bool found(false);
    for (const auto& extHit : track.getRelationsTo<ExtHit>()) {
      if (extHit.getPdgCode() != Const::pion.getPDGCode() && extHit.getPdgCode() != -Const::pion.getPDGCode()) continue;
      if ((extHit.getDetectorID() !=  Const::EDetector::ECL)) continue;
      if (extHit.getStatus() != EXT_ENTER) continue;
      if (extHit.getCopyID() == -1) continue;
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

void ECLTrackShowerMatchModule::computeDepth(const ECLShower& shower, double& lTrk, double& lShower) const
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
    if (cp > p) selectedTrk = & track;
  }
  lTrk = 0;
  lShower = 0;
  if (selectedTrk == nullptr) return;
  bool found(false);
  for (const auto& extHit : selectedTrk->getRelationsTo<ExtHit>()) {
    if (extHit.getPdgCode() != Const::pion.getPDGCode() && extHit.getPdgCode() != -Const::pion.getPDGCode()) continue;
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
  double costh = avgDir * trkdir;
  double sin2th = 1 - costh * costh;
  lShower = costh * (w0 * trkdir) - w0 * avgDir;
  lShower /= sin2th;

  lTrk = w0 * trkdir - costh * (w0 * avgDir);
  lTrk /= sin2th;
}

