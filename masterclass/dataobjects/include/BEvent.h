//+
// File : BEvent.h
// Description : class to contain an event
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 28 - Jan - 2004
//-

#pragma once

#include "TObject.h"
#include "TClonesArray.h"

#include "BParticle.h"

//! The Class for Masterclass simulation parameters
/*! This class provides the data structure to write out the data files used for Belle II Masterclass application.
*/
class BEvent : public TObject {
private:
  int m_evno; /*!< current event number */
  int m_nprt; /*!< number of particles in the event */
  TClonesArray* m_particles; /*!< array of particles */

public:

  //! Default constructor
  BEvent();

  //! Default destructor
  ~BEvent();

  /**
  * Clear the array of particles.
  */

  virtual void Clear(Option_t* /*option*/ = "");
  /**
   * Set the current event number.
   */

  void EventNo(int evtno);

  /**
   * Get the current event number.
   */
  int EventNo();

  /**
     * Add the track to the event.
     * @param px - x component of the particle momentum
     * @param py - y component of the particle momentum
     * @param pz - z component of the particle momentum
     * @param e  - energy of the particle
     * @param charge  - unit charge of the particle
     * @param pid - particle identity - hypothesis with the highest maximum likelihood
     */
  void AddTrack(float px, float py, float pz, float e,
                float charge, SIMPLEPID pid);

  /**
   * Get the number of particles in the event.
   */

  int NParticles();

  /**
   * Returns the array of particles.
   */
  TClonesArray* GetParticleList();

  ClassDef(BEvent, 1)
};


