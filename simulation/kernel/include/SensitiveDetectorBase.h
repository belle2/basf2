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
#include <mdst/dataobjects/MCParticle.h>
#include <simulation/kernel/UserInfo.h>
#include <G4VSensitiveDetector.hh>
class G4Step;

#include <map>
#include <string>


namespace Belle2 {
  namespace Simulation {

    /** Base class for all Sensitive Detectors to create hits during simulation. */
    class SensitiveDetectorBase : public G4VSensitiveDetector {
    public:

      /** Create a new Sensitive detecor with a given name and belonging to a given subdetector.
       * @param name name of the sensitive detector
       * @param subdetector subdetector the sensitive detector class belongs to
       */
      SensitiveDetectorBase(const std::string& name, Const::EDetector subdetector):
        G4VSensitiveDetector(name), m_subdetector(subdetector) {}

      /** virtual destructor for inheritance */
      virtual ~SensitiveDetectorBase() {}

      /** Return a list of all registered Relations with MCParticles. */
      static const std::map<std::string, RelationArray::EConsolidationAction>& getMCParticleRelations() { return s_mcRelations; }
      /** Enable/Disable all Sensitive Detectors.
       * By default, all sensitive detectors won't create hits to make it
       * possible to use the Geant4 Navigator for non-simulation purposes. Only
       * during simulation the sensitive detectors will be enabled to record
       * hits
       * @param activeStatus bool to indicate wether hits should be recorded
       */
      static void setActive(bool activeStatus) { s_active = activeStatus; }
      /** Register an relation involving MCParticles.
       * All Relations which point from an MCParticle to something have to be
       * registered with addMCParticleRelation() because the index of the
       * MCParticles might change at the end of the event. During simulation,
       * the TrackID should be used as index of the MCParticle
       * @param name Name of the relation to register
       */
      static void registerMCParticleRelation(const std::string& name,
                                             RelationArray::EConsolidationAction ignoreAction = RelationArray::c_negativeWeight);

      /** Overload to make it easer to register MCParticle relations
       * @param relation RelationArray to register
       */
      static void registerMCParticleRelation(const RelationArray& relation,
                                             RelationArray::EConsolidationAction ignoreAction = RelationArray::c_negativeWeight) { registerMCParticleRelation(relation.getName(), ignoreAction); }

    protected:
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
      static std::map<std::string, RelationArray::EConsolidationAction> s_mcRelations;
      /** Static bool which indicates wether recording of hits is enabled */
      static bool s_active;
      /** Subdetector the class belongs to */
      Const::EDetector m_subdetector;
    };

    inline bool SensitiveDetectorBase::ProcessHits(G4Step* aStep, G4TouchableHistory* aROhist)
    {
      if (!s_active) return false;
      bool result = step(aStep, aROhist);
      // Do not include hits from invalid detector (beast,teastbeam, etc.)
      if (result && (m_subdetector != Const::invalidDetector)) TrackInfo::getInfo(*aStep->GetTrack()).addSeenInDetector(m_subdetector);
      return result;
    }

  }  // end namespace Simulation
} // end namespace Belle2

#endif /* SENSITIVEDETECTOR_H_ */
