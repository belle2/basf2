/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 * Contributors: Guglielmo De Nardo (denardo@na.infn.it)                  *
 *               Torben Ferber (torben.ferber@desy.de)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
//This module
#include <ecl/modules/eclTrackShowerMatch/ECLTrackShowerMatchModule.h>

//Framework
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/RelationVector.h>

//ECL
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLShower.h>

//MDST
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>

//tracking
#include <tracking/dataobjects/ExtHit.h>

using namespace std;
using namespace Belle2;

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
  m_tracks.isRequired();
  m_eclShowers.isRequired();
  m_eclClusters.isRequired();
  m_eclCalDigits.isRequired();
  m_extHits.isRequired();

  m_tracks.registerRelationTo(m_eclShowers);
  m_tracks.registerRelationTo(m_eclClusters);
}

void ECLTrackShowerMatchModule::beginRun()
{
}

void ECLTrackShowerMatchModule::event()
{
  Const::EDetector myDetID = Const::EDetector::ECL;
  Const::ChargedStable hypothesis = Const::pion;
  int pdgCode = abs(hypothesis.getPDGCode());

  for (const Track& track : m_tracks) {

    //Unique shower ids related to this track
    set<int> uniqueShowerIds;

    //Needed to make sure that we match one shower at most
    set<int> uniquehypothesisIds;
    vector<int> hypothesisIds;
    vector<double> energies;
    vector<int> arrayIndexes;

    // Find extrapolated track hits in the ECL, considering
    // only hit points where the track enters the crystal
    // note that more than on crystal belonging to more than one shower
    // can be found
    for (const auto& extHit : track.getRelationsTo<ExtHit>()) {
      if (abs(extHit.getPdgCode()) != pdgCode) continue;
      if ((extHit.getDetectorID() != myDetID)) continue;
      if (extHit.getStatus() != EXT_ENTER) continue;
      int copyid =  extHit.getCopyID();
      if (copyid == -1) continue;
      const int cell = copyid + 1;

      //Find ECLCalDigit with same cell ID as ExtHit
      const auto idigit = find_if(m_eclCalDigits.begin(), m_eclCalDigits.end(),
      [&](const ECLCalDigit & d) { return d.getCellId() == cell; }
                                 );
      //Couldn't find ECLCalDigit with same cell ID as the ExtHit
      if (idigit == m_eclCalDigits.end()) continue;

      //Save all unique shower IDs of the showers related to idigit
      for (auto& shower : idigit->getRelationsFrom<ECLShower>()) {
        bool inserted = (uniqueShowerIds.insert(shower.getUniqueId())).second;

        //If this track <-> shower relation hasn't been set yet, set it for the shower and the ECLCLuster
        if (!inserted) continue;

        hypothesisIds.push_back(shower.getHypothesisId());
        energies.push_back(shower.getEnergy());
        arrayIndexes.push_back(shower.getArrayIndex());
        uniquehypothesisIds.insert(shower.getHypothesisId());

        B2DEBUG(29, shower.getArrayIndex() << " "  << shower.getHypothesisId() << " " << shower.getEnergy() << " " <<
                shower.getConnectedRegionId());

      } //end loop on shower related to idigit
    } // end loop on ExtHit

    // only set the relation for the highest energetic shower per hypothesis
    for (auto hypothesisId : uniquehypothesisIds) {
      double highestEnergy = 0.0;
      int arrayindex = -1;

      for (unsigned ix = 0; ix < energies.size(); ix++) {
        if (hypothesisIds[ix] == hypothesisId and energies[ix] > highestEnergy) {
          highestEnergy = energies[ix];
          arrayindex = arrayIndexes[ix];
        }
      }

      // if we find a shower, take that one by directly acessing the store array
      if (arrayindex > -1) {
        auto shower = m_eclShowers[arrayindex];
        shower->setIsTrack(true);
        track.addRelationTo(shower);
        B2DEBUG(29, shower->getArrayIndex() << " "  << shower->getIsTrack());

        // there is a 1:1 relation, just set the relation for the corresponding cluster as well
        ECLCluster* cluster = shower->getRelatedFrom<ECLCluster>();
        if (cluster != nullptr) {
          cluster->setIsTrack(true);
          track.addRelationTo(cluster);
        }
      }
    }


  } // end loop on Tracks
}

void ECLTrackShowerMatchModule::endRun()
{
}

void ECLTrackShowerMatchModule::terminate()
{
}
