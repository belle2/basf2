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
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <tracking/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <G4ParticleTable.hh>

#include <set>
//avoid having to wrap everything in the namespace explicitly
//only permissible in .cc files!

using namespace std;
using namespace Belle2;
// using namespace ECL;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
//Note that the 'Module' part of the class name is missing, this is also the way it
//will be called in the module list.
REG_MODULE(ECLTrackShowerMatch)

ECLTrackShowerMatchModule::ECLTrackShowerMatchModule() : Module()
{
  setDescription("Set the Track --> ECLShower Relation.");
}


ECLTrackShowerMatchModule::~ECLTrackShowerMatchModule()
{
}


void ECLTrackShowerMatchModule::initialize()
{
  RelationArray::registerPersistent<Track, ECLShower>();
}

void ECLTrackShowerMatchModule::beginRun()
{
}

void ECLTrackShowerMatchModule::event()
{
  StoreArray<Track> Tracks;
  StoreArray<ECLHitAssignment> eclHitAssignmentArray;
  StoreArray<ECLShower> eclRecShowerArray;

  ExtDetectorID myDetID = EXT_ECL;
  int pdgCodePiP = G4ParticleTable::GetParticleTable()->FindParticle("pi+")->GetPDGEncoding();
  int pdgCodePiM = G4ParticleTable::GetParticleTable()->FindParticle("pi-")->GetPDGEncoding();

  for (int t = 0; t < Tracks.getEntries(); ++t) {
    const Track* track = Tracks[t];
    set<int> clid;
    for (auto extHit : track->getRelationsTo<ExtHit>()) {
      if (extHit.getPdgCode() != pdgCodePiP && extHit.getPdgCode() != pdgCodePiM) continue;
      if ((extHit.getDetectorID() != myDetID) || (extHit.getCopyID() == 0)) continue;
      int cell = extHit.getCopyID() + 1;

      int showerid = -1;
      auto iHA =
        find_if(eclHitAssignmentArray.begin(), eclHitAssignmentArray.end(),
      [&](const ECLHitAssignment & ha) { return ha.getCellId() == cell; }
               );
      if (iHA != eclHitAssignmentArray.end()) showerid = iHA -> getShowerId();
      if (showerid != -1) clid.insert(showerid);

    } // end loop on ExtHit

    for (auto id : clid) {
      auto ish = find_if(eclRecShowerArray.begin(), eclRecShowerArray.end(),
      [&](const ECLShower & element) { return element.GetShowerId() == id; }
                        );
      if (ish != eclRecShowerArray.end()) track->addRelationTo(&(*ish));
    } // end loop on crystal id

  } // end loop on Tracks

}

void ECLTrackShowerMatchModule::endRun()
{
}

void ECLTrackShowerMatchModule::terminate()
{
}
