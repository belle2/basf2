/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 * Contributors: Guglielmo De Nardo                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclTrackShowerMatch/ECLTrackShowerMatchModule.h>
#include <ecl/dataobjects/ECLHitAssignment.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <tracking/dataobjects/ExtHit.h>
#include <framework/gearbox/Const.h>

#include <framework/logging/Logger.h>

#include <set>

using namespace std;
using namespace Belle2;
using namespace ECL;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
//Note that the 'Module' part of the class name is missing, this is also the way it
//will be called in the module list.
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
  StoreArray<ECLHitAssignment> eclHitAssignments;
  StoreArray<ECLShower> eclRecShowers;
  StoreArray<ECLCluster> eclClusters;

  Const::EDetector myDetID = Const::EDetector::ECL;
  const int pdgCodePiPlus = Const::pion.getPDGCode();
  const int pdgCodePiMinus = -1 * Const::pion.getPDGCode();
  ECLGeometryPar* geometry = ECLGeometryPar::Instance();
  for (const Track& track : tracks) {
    set<int> clid;
    B2DEBUG(150, "*** TRACK ****");
    for (const auto& extHit : track.getRelationsTo<ExtHit>()) {
      if (extHit.getPdgCode() != pdgCodePiPlus && extHit.getPdgCode() != pdgCodePiMinus) continue;
      if ((extHit.getDetectorID() != myDetID) || (extHit.getCopyID() == 0)) continue;
      if (extHit.getStatus() != EXT_ENTER) continue;
      const int cell = extHit.getCopyID() + 1;
      TVector3 cpos   = geometry->GetCrystalPos(cell);
      TVector3 trkpos = extHit.getPosition();
      int showerid = -1;
      const auto iHA =
        find_if(eclHitAssignments.begin(), eclHitAssignments.end(),
      [&](const ECLHitAssignment & ha) { return ha.getCellId() == cell; }
               );
      if (iHA != eclHitAssignments.end()) {
        showerid = iHA->getShowerId();
        clid.insert(showerid);

        B2DEBUG(150, "******** EXT HIT ********");
        B2DEBUG(150, "Status: " << extHit.getStatus());
        B2DEBUG(150, "cellid: " << cell);
        B2DEBUG(150, "ctheta  :" << cpos.Theta());
        B2DEBUG(150, "trktheta:" << trkpos.Theta());
        B2DEBUG(150, "cphi  :" << cpos.Phi());
        B2DEBUG(150, "trkphi:" << trkpos.Phi());
        B2DEBUG(150, "showid: " << showerid);
        B2DEBUG(150, "tof   : " << extHit.getTOF());
      }
    } // end loop on ExtHit


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
          cluster->setisTrack(true);
          track.addRelationTo(cluster);
        }
      }
    } // end loop on shower id

    B2DEBUG(150, "************************");
  } // end loop on Tracks

  for (auto& shower : eclRecShowers) {
    double dist = computeTrkMinDistance(shower, tracks);
    shower.setMinTrkDistance(float(dist));
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
  TVector3 cryFaceCenter;
  cryFaceCenter.SetMagThetaPhi(shower.getR(), shower.getTheta(), shower.getPhi());
  for (const auto& track : tracks) {
    TVector3 trkpos(0, 0, 0);
    bool found(false);
    for (const auto& extHit : track.getRelationsTo<ExtHit>()) {
      if (extHit.getPdgCode() != Const::pion.getPDGCode() && extHit.getPdgCode() != -Const::pion.getPDGCode()) continue;
      if ((extHit.getDetectorID() !=  Const::EDetector::ECL) || (extHit.getCopyID() == 0)) continue;
      if (extHit.getStatus() != EXT_ENTER) continue;
      trkpos = extHit.getPosition();
      found = true;
      break;
    }
    if (! found) continue;
    double distance = (cryFaceCenter - trkpos).Mag();
    if (distance < minDist) minDist = distance;
  }
  if (minDist > 9999) minDist = -1;
  return minDist;
}
