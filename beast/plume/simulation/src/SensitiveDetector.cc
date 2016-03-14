/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle, Jerome Baudot                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/plume/simulation/SensitiveDetector.h>
#include <beast/plume/dataobjects/PlumeSimHit.h>
#include <beast/plume/dataobjects/PlumeHit.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>

#include <G4Track.hh>
#include <G4Step.hh>

#include <iostream>
using namespace std;


namespace Belle2 {
  /** Namespace to encapsulate code needed for the PLUME detector */
  namespace plume {

    SensitiveDetector::SensitiveDetector():
      Simulation::SensitiveDetectorBase("PlumeSensitiveDetector", Const::invalidDetector)
    {
      current_trackID = 0;
      current_trackVertex_x = 0.;
      current_trackVertex_y = 0.;
      current_trackVertex_z = 0.;
      current_pdgID = 0;
      current_sensorID = 0;
      current_posIN_u = 0.; // mm  z_global for each sensor (-1 -> + 1 mm)
      current_posIN_v = 0.;  // -5 -> +5 mm
      current_posIN_w = 0.;   // w (epi layer + foam width + etc.)
      current_posIN_x = 0.;
      current_posIN_y = 0.;
      current_posIN_z = 0.;
      current_posOUT_u = 0.;
      current_posOUT_v = 0.;
      current_posOUT_w = 0.;
      current_posOUT_x = 0.;
      current_posOUT_y = 0.;
      current_posOUT_z = 0.;
      current_momentum_x = 0.;  // GeV
      current_momentum_y = 0.;
      current_momentum_z = 0.;
      current_energyDep = 0.;  // GeV
      current_nielDep = 0.;
      current_thetaAngle = 0.;   // local sensor frame - degree
      current_phiAngle = 0.;
      current_globalTime = 0.;

      //Make sure all collections are registered
      StoreArray<MCParticle>   mcParticles;
      StoreArray<PlumeSimHit>  simHits;
      StoreArray<PlumeHit>     plumeHits;

      RelationArray relMCSimHit(mcParticles, simHits);
      RelationArray relMCplumeHit(mcParticles, plumeHits);

      //Register all collections we want to modify and require those we want to use
      mcParticles.registerInDataStore();
      simHits.registerInDataStore();
      plumeHits.registerInDataStore();
      relMCSimHit.registerInDataStore();
      relMCplumeHit.registerInDataStore();

      //Register the Relation so that the TrackIDs get replaced by the actual
      //MCParticle indices after simulating the events. This is needed as
      //secondary particles might not be stored so everything relating to those
      //particles will be attributed to the last saved mother particle
      registerMCParticleRelation(relMCSimHit);
      registerMCParticleRelation(relMCplumeHit);
    }

    SensitiveDetector::~SensitiveDetector()
    {

    }


    bool SensitiveDetector::step(G4Step* step, G4TouchableHistory*)
    {

      //Get track information, and pre- and post-step
      const G4Track& track = *step->GetTrack();
      const G4StepPoint& preStepPoint = *step->GetPreStepPoint();
      const G4StepPoint& postStepPoint = *step->GetPostStepPoint();


      // If new track, store general information from this first step
      if (current_trackID != track.GetTrackID()) { // if new track
        current_trackID = track.GetTrackID();
        current_trackVertex_x = track.GetVertexPosition().x();
        current_trackVertex_y = track.GetVertexPosition().y();
        current_trackVertex_z = track.GetVertexPosition().z();
        current_pdgID = track.GetDefinition()->GetPDGEncoding();
        current_sensorID = track.GetVolume()->GetCopyNo();

        // since this is first step in volume store track incidence and momentum and reset energy loss
        G4ThreeVector preStepPointPosition = preStepPoint.GetPosition();
        G4ThreeVector trackMomentum = track.GetMomentum();
        current_energyDep = 0.;
        current_nielDep = 0.;

        current_posIN_x = preStepPointPosition.x();  //mm
        current_posIN_y = preStepPointPosition.y();
        current_posIN_z = preStepPointPosition.z();
        current_momentum_x = trackMomentum.x();  // MeV
        current_momentum_y = trackMomentum.y();
        current_momentum_z = trackMomentum.z();

        // We want the track angles of incidence on the local volume and the local position
        G4TouchableHandle theTouchable = preStepPoint.GetTouchableHandle();
        G4ThreeVector worldDirection = preStepPoint.GetMomentumDirection();
        G4ThreeVector localDirection = theTouchable->GetHistory()->GetTopTransform().TransformAxis(worldDirection);
        current_thetaAngle = localDirection.theta();
        current_phiAngle = localDirection.phi();

        G4ThreeVector localINPosition = theTouchable->GetHistory()->GetTopTransform().TransformPoint(preStepPointPosition);
        current_posIN_v = localINPosition.x();
        current_posIN_u = localINPosition.y();
        current_posIN_w = localINPosition.z();

        current_globalTime = step->GetPreStepPoint()->GetGlobalTime(); // not sure this is relevant for PLUME?

      }  // end if new track

      // Update information at each step
      current_energyDep += step->GetTotalEnergyDeposit();  // MeV
      current_nielDep += step->GetNonIonizingEnergyDeposit();   //not sure it works?

      // If track leaves volume or is killed, store final step info and save simHit
      if (track.GetNextVolume() != track.GetVolume() || track.GetTrackStatus() >= fStopAndKill) { // if last step

        G4ThreeVector postStepPointPosition = postStepPoint.GetPosition();

        current_posOUT_x = postStepPointPosition.x();
        current_posOUT_y = postStepPointPosition.y();
        current_posOUT_z = postStepPointPosition.z();
        G4ThreeVector localOUTPosition = preStepPoint.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(
                                           postStepPointPosition);
        current_posOUT_v = localOUTPosition.x(); // mm
        current_posOUT_u = localOUTPosition.y();
        current_posOUT_w = localOUTPosition.z();

        //Get the datastore arrays
        StoreArray<MCParticle>  mcParticles;
        StoreArray<PlumeSimHit> simHits;

        if (current_energyDep > CLHEP::eV) {

          if (!simHits.isValid()) simHits.create();
          RelationArray relMCSimHit(mcParticles, simHits);
          PlumeSimHit* hit = simHits.appendNew(
                               current_pdgID,
                               current_sensorID,
                               current_trackID,
                               current_trackVertex_x,
                               current_trackVertex_y,
                               current_trackVertex_z,
                               current_energyDep,
                               current_nielDep,
                               current_posIN_x,
                               current_posIN_y,
                               current_posIN_z,
                               current_posIN_u,
                               current_posIN_v,
                               current_posIN_w,
                               current_posOUT_u,
                               current_posOUT_v,
                               current_posOUT_w,
                               current_momentum_x / CLHEP::GeV,
                               current_momentum_y / CLHEP::GeV,
                               current_momentum_z / CLHEP::GeV,
                               current_thetaAngle / CLHEP::degree,
                               current_phiAngle / CLHEP::degree,
                               current_globalTime
                             );

          //Add Relation between SimHit and MCParticle with a weight of 1. Since
          //the MCParticle index is not yet defined we use the trackID from Geant4
          relMCSimHit.add(current_trackID, hit->getArrayIndex(), 1.0);
        }

        //Reset TrackID
        current_trackID = 0;

      } // end if last


      //Ignore everything below 1eV
      if (current_energyDep < CLHEP::eV) return false;


      return true;
    }

  } //plume namespace
} //Belle2 namespace
