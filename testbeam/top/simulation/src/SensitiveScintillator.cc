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
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <TVector3.h>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace TOPTB {

    SensitiveScintillator::SensitiveScintillator(int detectorID, EDetectorType type):
      Simulation::SensitiveDetectorBase("TOP", SensitiveScintillator::Other),
      m_detectorID(detectorID), m_type(type), m_energyDeposit(0),
      m_meanTime(0), m_meanX(0), m_meanY(0), m_trackID(0)
    {
      /*
      // registration
      StoreArray<TOPBarHit>::registerPersistent();
      RelationArray::registerPersistent<MCParticle, TOPBarHit>();

      // additional registration of MCParticle relation (required for correct relations)
      StoreArray<MCParticle> particles;
      StoreArray<TOPBarHit>  barhits;
      RelationArray  relation(particles, barhits);
      registerMCParticleRelation(relation, RelationArray::c_deleteElement);
      */
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
      }

      // update energy deposit (note: unit is MeV) and other privates

      double energyLoss = aStep->GetTotalEnergyDeposit();
      m_energyDeposit += energyLoss;
      m_meanTime += track.GetGlobalTime() * energyLoss;
      const G4ThreeVector& position = track.GetPosition();
      G4ThreeVector localPosition =
        track.GetTouchableHandle()->GetHistory()->
        GetTopTransform().TransformPoint(position);
      m_meanX += localPosition.x() * energyLoss;
      m_meanY += localPosition.y() * energyLoss;

      // save hit if track leaves volume or is killed

      if (track.GetNextVolume() != track.GetVolume() ||
          track.GetTrackStatus() >= fStopAndKill) {
        if (m_energyDeposit > 0) {
          int channelID = 0;
          if (m_type == c_sciFi)
            channelID = track.GetTouchableHandle()->GetReplicaNumber();
          m_meanTime /= m_energyDeposit;
          m_meanX /= m_energyDeposit;
          m_meanY /= m_energyDeposit;

          //--> not finished yet!

        }

        m_trackID = 0;
      }

      return true;
    }


  } // end of namespace TOPTB
} // end of namespace Belle2
