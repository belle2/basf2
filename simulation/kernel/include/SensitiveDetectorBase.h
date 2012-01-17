/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Ritter                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SENSITIVEDETECTOR_H_
#define SENSITIVEDETECTOR_H_

#include <framework/datastore/RelationArray.h>
#include <generators/dataobjects/MCParticle.h>
#include <simulation/kernel/UserInfo.h>
#include <G4VSensitiveDetector.hh>
#include <G4Step.hh>

#include <set>
#include <string>


namespace Belle2 {
  namespace Simulation {

    /** Base class for all Sensitive Detectors to create hits during simulation. */
    class SensitiveDetectorBase : public G4VSensitiveDetector {
    public:
      /** Enum to specify all subdetectors which could produce Hits in the simulation.
       * Used to automatically set the SeenIn-flags of the MCParticles
       */
      enum DetectorComponent {
        PXD = MCParticle::c_SeenInPXD,      /**< Pixel detector */
        SVD = MCParticle::c_SeenInSVD,      /**< Strip detector */
        CDC = MCParticle::c_SeenInCDC,      /**< Drift chamber  */
        TOP = MCParticle::c_SeenInTOP,      /**< Time of Propagation */
        ECL = MCParticle::c_LastSeenInECL,  /**< Calorimeter */
        KLM = MCParticle::c_LastSeenInKLM,  /**< KLM */
        Other = 0                           /**< Any other component */
      };

      /** Create a new Sensitive detecor with a given name and belonging to a given subdetector.
       * @param name name of the sensitive detector
       * @param subdetector subdetector the sensitive detector class belongs to
       */
      SensitiveDetectorBase(const std::string& name, DetectorComponent subdetector):
        G4VSensitiveDetector(name), m_subdetector(subdetector) {}

      /** virtual destructor for inheritance */
      virtual ~SensitiveDetectorBase() {}

      /** Return a list of all registered Relations with MCParticles. */
      static const std::set<std::string> getMCParticleRelations() { return m_mcRelations; }
      /** Enable/Disable all Sensitive Detectors.
       * By default, all sensitive detectors won't create hits to make it
       * possible to use the Geant4 Navigator for non-simulation purposes. Only
       * during simulation the sensitive detectors will be enabled to record
       * hits
       * @param active bool to indicate wether hits should be recorded
       */
      static void setActive(bool active) { m_active = active; }
    protected:
      /** Register an relation involving MCParticles.
       * All Relations which point from an MCParticle to something have to be
       * registered with addMCParticleRelation() because the index of the
       * MCParticles might change at the end of the event. During simulation,
       * the TrackID should be used as index of the MCParticle
       * @param name Name of the relation to register
       */
      void registerMCParticleRelation(const std::string& name) { m_mcRelations.insert(name); }
      /** Overload to make it easer to register MCParticle relations
       * @param relation RelationArray to register
       */
      void registerMCParticleRelation(const RelationArray& relation) { m_mcRelations.insert(relation.getName()); }

      /** Process a Geant4 step in any of the sensitive volumes attached to this sensitive detector.
       * This is the main function to be implemented by subclasses. The
       * original ProcessHits is now used to check if recordign of hits is
       * enabled and to set the correct MCParticle flag automatically on
       * creation of a hit
       * @param step Geant4 step which has all the information
       * @param ROhist TouchableHistory of the ReadoutGeometry if any is defined
       * @return true if a hit was recorded, false otherwise
       */
      virtual bool step(G4Step* step, G4TouchableHistory* ROhist) = 0;
    private:
      /** Check if recording hits is enabled and if so call step() and set the
       * correct MCParticle flag.  Called by Geant4 for each step inside the
       * sensitive volumes attached
       */
      virtual bool ProcessHits(G4Step* aStep, G4TouchableHistory* aROhist);
      /** Static set holding all relations which have to be updated at the end of the Event */
      static std::set<std::string> m_mcRelations;
      /** Static bool which indicates wether recording of hits is enabled */
      static bool m_active;
      /** Subdetector the class belongs to */
      DetectorComponent m_subdetector;
    };

    inline bool SensitiveDetectorBase::ProcessHits(G4Step* aStep, G4TouchableHistory* aROhist)
    {
      if (!m_active) return false;
      bool result = step(aStep, aROhist);
      if (result && m_subdetector) TrackInfo::getInfo(*aStep->GetTrack()).addStatus(m_subdetector);
      return result;
    }

  }  // end namespace Simulation
} // end namespace Belle2

#endif /* SENSITIVEDETECTOR_H_ */
