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
#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/dataobjects/ECLShower.h>

//MDST
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/TrackFitResult.h>

//tracking
#include <tracking/dataobjects/ExtHit.h>

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
    //    std::set<int>::iterator it;
    //for (it = uniquehypothesisIds.begin(); it != uniquehypothesisIds.end(); ++it) {
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

  for (auto& shower : m_eclShowers) {
    // compute the distance from shower COG and the closest extrapolated track
    double dist = computeTrkMinDistance(shower, m_tracks);
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
      B2DEBUG(29, "shower depth: ltrk = " << lTrk << " lShower = " << lShower);
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
  Const::ChargedStable hypothesis = Const::pion;
  int pdgCode = abs(hypothesis.getPDGCode());
  for (const auto& track : tracks) {
    TVector3 trkpos(0, 0, 0);
    bool found(false);
    for (const auto& extHit : track.getRelationsTo<ExtHit>()) {
      if (abs(extHit.getPdgCode()) != pdgCode) continue;
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
    const TrackFitResult* fit = track.getTrackFitResultWithClosestMass(Const::pion);
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
  double costh = avgDir * trkdir;
  double sin2th = 1 - costh * costh;
  lShower = costh * (w0 * trkdir) - w0 * avgDir;
  lShower /= sin2th;

  lTrk = w0 * trkdir - costh * (w0 * avgDir);
  lTrk /= sin2th;
}
