/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SENSITIVEDETECTOR_H_
#define SENSITIVEDETECTOR_H_

#include <G4VSensitiveDetector.hh>
#include <G4Step.hh>

#include <TObject.h>


namespace Belle2 {

  namespace Simulation {

    /**
     * The base sensitive detector class.
     * It provides support for automatic relation creation and
     * offers additional, convenient methods for the user.
     */
    class SensitiveDetectorBase : public G4VSensitiveDetector {

    public:

      /**
       * Constructor.
       */
      SensitiveDetectorBase(G4String name);

      /**
       * Destructor.
       */
      virtual ~SensitiveDetectorBase();

      /**
       * Adds a new relation between the MCParticle and the hit.
       * Takes as an input a step, which represents the track the hit was created from.
       * Later the track is replaced by the MCParticle which created the track and stored
       * as a relation in the DataStore.
       *
       * @param hit Pointer to the hit which was created by the track.
       * @param step The step representing the track which created the hit.
       * @return True if the relation could be added.
       */
      bool addRelation(TObject* hit, G4Step* step);


    protected:

    };

  }  // end namespace Simulation
} // end namespace Belle2

#endif /* SENSITIVEDETECTOR_H_ */
