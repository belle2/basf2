/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <testbeam/top/simulation/SensitiveScintillator.h>

#include <simulation/kernel/UserInfo.h>
#include <G4Step.hh>
#include <G4Track.hh>
#include <G4UnitsTable.hh>
#include <G4ParticleDefinition.hh>
#include <G4ParticleTypes.hh>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <testbeam/top/dataobjects/TOPTBSimHit.h>


#include <TVector3.h>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace TOPTB {

    SensitiveScintillator::SensitiveScintillator(int detectorID, EDetectorType type):
      Simulation::SensitiveDetectorBase("TOP", Const::invalidDetector),
      m_detectorID(detectorID), m_type(type), m_energyDeposit(0),
      m_meanTime(0), m_meanX(0), m_meanY(0), m_meanZ(0), m_trackID(0)
    {

      StoreArray<MCParticle> mcParticles;

      StoreArray<TOPTBSimHit> simHits;
      simHits.registerInDataStore();
      mcParticles.registerRelationTo(simHits);

      RelationArray  relation(mcParticles, simHits);
      registerMCParticleRelation(relation);

    }


    bool SensitiveScintillator::step(G4Step* aStep, G4TouchableHistory*)
    {

      G4Track& track = *aStep->GetTrack();

      // reset energy deposit if trackID changed

      if (m_trackID != track.GetTrackID()) {
        m_trackID = track.GetTrackID();
        m_energyDeposit = 0;
        m_meanTime = 0;
        m_meanX = 0;
        m_meanY = 0;
        m_meanZ = 0;
      }

      // update energy deposit (note: unit is MeV)

      double energyLoss = aStep->GetTotalEnergyDeposit();
      m_energyDeposit += energyLoss;
      double time = track.GetGlobalTime() - 0.5 * aStep->GetDeltaTime();
      m_meanTime += (time * energyLoss);
      const G4ThreeVector& position = track.GetPosition() -
                                      0.5 * aStep->GetDeltaPosition();
      G4ThreeVector localPosition =
        track.GetTouchableHandle()->GetHistory()->
        GetTopTransform().TransformPoint(position);
      m_meanX += (localPosition.x() * Unit::mm * energyLoss);
      m_meanY += (localPosition.y() * Unit::mm * energyLoss);
      m_meanZ += (localPosition.z() * Unit::mm * energyLoss);

      // save hit if track leaves volume or is killed and energy deposit is > 0

      bool detected = false;
      if (track.GetNextVolume() != track.GetVolume() ||
          track.GetTrackStatus() >= fStopAndKill) {
        if (m_energyDeposit > 0) {
          int channelID = 0;
          if (m_type == c_sciFi)
            channelID = track.GetTouchableHandle()->GetReplicaNumber();
          m_meanTime /= m_energyDeposit;
          m_meanX /= m_energyDeposit;
          m_meanY /= m_energyDeposit;
          m_meanZ /= m_energyDeposit;

          StoreArray<TOPTBSimHit> hits;
          hits.appendNew(m_detectorID,
                         m_type,
                         channelID,
                         m_energyDeposit,
                         m_meanTime,
                         m_meanX,
                         m_meanY,
                         m_meanZ);
          StoreArray<MCParticle> particles;
          RelationArray relation(particles, hits);
          int lastHit = hits.getEntries() - 1;
          relation.add(m_trackID, lastHit);
          detected = true;
        }

        m_trackID = 0;
      }

      return detected;
    }

  } // end of namespace TOPTB
} // end of namespace Belle2
