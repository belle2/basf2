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

    SensitiveScintillator::SensitiveScintillator(int detectorID):
      Simulation::SensitiveDetectorBase("TOP", SensitiveScintillator::Other),
      m_detectorID(detectorID), m_energyDeposit(0), m_channelID(0), m_trackID(0)
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
        m_channelID = track.GetTouchableHandle()->GetReplicaNumber();
      }

      // update energy deposit (note: unit is MeV)
      m_energyDeposit += aStep->GetTotalEnergyDeposit();

      // save hit if track leaves volume or is killed
      if (track.GetNextVolume() != track.GetVolume() ||
          track.GetTrackStatus() >= fStopAndKill) {
        if (m_energyDeposit > 0) {


          //--> test
          const G4ThreeVector& r = track.GetPosition();
          G4ThreeVector rl = track.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(r);

          cout << m_detectorID << " ";
          cout << m_channelID << " ";
          cout << m_trackID << " ";
          cout << " " << r.x();
          cout << " " << r.y();
          cout << " " << r.z();
          cout << " " << rl.x();
          cout << " " << rl.y();
          cout << " " << rl.z();
          cout << endl;

          /*
          cout << "detector" << m_detectorID;
          cout << " chID=" << chID;
          cout << " trackID=" << m_trackID;
          cout << " energyDeposit=" << m_energyDeposit;
          cout << " " << r.x();
          cout << " " << r.y();
          cout << " " << r.z();
          cout << " " << rl.x();
          cout << " " << rl.y();
          cout << " " << rl.z();
          cout << endl;
          */
          //<-- endtest
        }

        m_trackID = 0;
      }

      return true;
    }


  } // end of namespace TOPTB
} // end of namespace Belle2
