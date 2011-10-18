/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Uglov Timofey                                             *
 * Class is based on BkgSensitiveDetector.h by                            *
 * Luka Santelj and Marko Petric                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMBACKSENSITIVEDETECTOR_H
#define EKLMBACKSENSITIVEDETECTOR_H

#include <simulation/kernel/SensitiveDetectorBase.h>
#include <eklm/eklmhit/EKLMBackHit.h>
#include <TString.h>

namespace Belle2 {

  //! The Class for ARICH Sensitive Detector
  /*! In this class, every variable defined in ARICHSimHit will be calculated,
    and stored in datastore.
  */
  class EKLMBkgSensitiveDetector : public Simulation::SensitiveDetectorBase {

  public:

    /**
     * Constructor.
     * @param subDet name of the subdetector (i.e. "IR","PXD","SVD",...)
     * @param iden identifier of subdetector component (optional, 0 by default)
     * @param hitType type of hits that are stored in DataStore (1 only neutrons, 2 only gammas, 0 both - default).
     */
    EKLMBkgSensitiveDetector(const char* subDet, int iden = 0, int hitType = 0);

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

#endif /* EKLMBACKENSITIVEDETECTOR_H */
