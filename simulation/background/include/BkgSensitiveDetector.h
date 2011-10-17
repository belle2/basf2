/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj and Marko Petric                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BACKSENSITIVEDETECTOR_H
#define BACKSENSITIVEDETECTOR_H

#include <simulation/kernel/SensitiveDetectorBase.h>
#include <simulation/dataobjects/BeamBackHit.h>
#include <TString.h>

namespace Belle2 {

  //! The Class for ARICH Sensitive Detector
  /*! In this class, every variable defined in ARICHSimHit will be calculated,
    and stored in datastore.
  */
  class BkgSensitiveDetector : public Simulation::SensitiveDetectorBase {

  public:

    /**
     * Constructor.
     * @param subDet name of the subdetector (i.e. "IR","PXD","SVD",...)
     * @param iden identifier of subdetector component (optional, 0 by default)
     * @param hitType type of hits that are stored in DataStore (1 only neutrons, 2 only gammas, 0 both - default).
     */
    BkgSensitiveDetector(const char* subDet, int iden = 0, int hitType = 0);

    /**
     * Process each step and calculate variables defined in PXDSimHit.
     * @param aStep Current Geant4 step in the sensitive medium.
     * @result true if a hit was stored, o.w. false.
     */
    bool step(G4Step* aStep, G4TouchableHistory*);


  private:
    int m_subDet;           /* subdetector id number */
    int m_identifier;       /* identifier of subdetector component */
    int m_hitType;        /* hit type of hits that are stored */

  }; // SensitiveDetector class
} // end of namespace Belle2

#endif /* BACKENSITIVEDETECTOR_H */
