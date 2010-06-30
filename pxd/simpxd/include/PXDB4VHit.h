/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDG4HIT_H_
#define PXDG4HIT_H_

#include <simkernel/B4VHit.h>

#include <list>

//Geant4 classes
#include "G4THitsCollection.hh"
#include "G4ThreeVector.hh"
#include "G4Allocator.hh"


namespace Belle2 {

  //! A struct to save detailed information about a single step.
  struct PXDStep {

  };


  //! The Class for a B4V PXD Hit.
  /*! Implementation of a geant4 PXD hit.
  */
  class PXDB4VHit : public B4VHit {

  public:

    //! Constructor
    PXDB4VHit(G4ThreeVector posIn, G4ThreeVector posOut, G4ThreeVector momIn, G4int PDGcode,
              G4int trackID, G4double energyDep, G4double stepLength, G4double globalTime,
              G4String volumeName);

    //! Destructor
    ~PXDB4VHit();

    //! Update the PXD hit information.
    void updateHit();

    //! Save pxd geant4 hits to the DataStore
    void Save(G4int iHit);

    //! Operator new
    inline void *operator new(size_t);

    //! Operator delete
    inline void operator delete(void *aPXDB4VHit);

  protected:

  private:

    G4ThreeVector m_posIn;  /*!< Position of pre-step. */
    G4ThreeVector m_posOut; /*!< Position of post-step. */
    G4ThreeVector m_momIn;  /*!< Momentum of pre-step. */

    G4int m_PDGcode;       /*!< The PDG value of particle which created the hit. */
    G4int m_trackID;       /*!< The ID of the track which created the hit. */
    G4double m_energyDep;  /*!< Deposited energy. */
    G4double m_stepLength; /*!< Step length. */
    G4double m_globalTime; /*!< Global time. */

    G4String m_volumeName; /*!< Name of the volume. */

    std::list<PXDStep> m_stepList; /*!< List of the steps that contributed to the hit. */

  };


  //-------------------
  //    Typdefs
  //-------------------
  typedef G4THitsCollection<PXDB4VHit> PXDB4VHitsCollection;

  extern G4Allocator<PXDB4VHit> PXDB4VHitAllocator;

  //Operator new
  inline void* PXDB4VHit::operator new(size_t)
  {
    void *aPXDB4VHit;
    aPXDB4VHit = (void *) PXDB4VHitAllocator.MallocSingle();
    return aPXDB4VHit;
  }

  //Operator delete
  inline void PXDB4VHit::operator delete(void *aPXDB4VHit)
  {
    PXDB4VHitAllocator.FreeSingle((PXDB4VHit*) aPXDB4VHit);
  }

} // end of namespace Belle2

#endif /* PXDG4HIT_H_ */
