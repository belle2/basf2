/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 * Contributors: Guglielmo De Nardo                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclTrackShowerMatch/ECLTrackShowerMatchModule.h>

#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLHitAssignment.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <tracking/dataobjects/ExtHit.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/StoreArray.h>

#include <set>

using namespace std;
using namespace Belle2;

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

  for (const Track& track : tracks) {
    set<int> clid;
    for (const auto& extHit : track.getRelationsTo<ExtHit>()) {
      if (extHit.getPdgCode() != pdgCodePiPlus && extHit.getPdgCode() != pdgCodePiMinus) continue;
      if ((extHit.getDetectorID() != myDetID) || (extHit.getCopyID() == 0)) continue;
      const int cell = extHit.getCopyID() + 1;

      int showerid = -1;
      const auto iHA =
        find_if(eclHitAssignments.begin(), eclHitAssignments.end(),
      [&](const ECLHitAssignment & ha) { return ha.getCellId() == cell; }
               );
      if (iHA != eclHitAssignments.end()) showerid = iHA->getShowerId();
      if (showerid != -1) clid.insert(showerid);

    } // end loop on ExtHit

    for (const auto& id : clid) {
      const auto ish = find_if(eclRecShowers.begin(), eclRecShowers.end(),
      [&](const ECLShower & element) { return element.getShowerId() == id; }
                              );
      if (ish != eclRecShowers.end()) {
        const ECLShower* shower = &(*ish);
        track.addRelationTo(shower);
        ECLCluster* cluster = shower->getRelatedFrom<ECLCluster>();
        if (cluster != nullptr) {
          cluster->setisTrack(true);
          track.addRelationTo(cluster);
        }
      }
    } // end loop on shower id
  } // end loop on Tracks

}

void ECLTrackShowerMatchModule::endRun()
{
}

void ECLTrackShowerMatchModule::terminate()
{
}
