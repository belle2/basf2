/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef B4VHIT_H
#define B4VHIT_H

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include <stdio.h>

namespace Belle2 {

//! The Class for Hits
  /*! This is a base class and all sub-detector hit classes should inherit from it.
      Some pure virtual functions should be implemented in sub-detector hit classes based on your requirements.
      However, the function Save(G4int) are strongly recommended to implement in sub-detector hit classes.
      And you can find an example of CDCHit, located at
      http://b2comp.kek.jp/browse/viewvc.cgi/svn/prototype/trunk/libraries/cdc/simcdc/simcdc/CDCHit.h or CDCHit.cc.

      This function(Save(G4int)) will be automatically called at the end of each event.
  */

  class B4VHit : public G4VHit {

  public:

    //! Constructor
    B4VHit() {}

    //! Destructor
    virtual ~B4VHit() {}

    //! Operator =
    const B4VHit& operator=(const B4VHit &) {
      return *this;
    }

    //! Operator ==
    G4int operator==(const B4VHit &) const
    { return false; }

    //! The method to draw hits
    virtual void Draw() {;}

    //! The method to print some debug information
    virtual void Print() {;}

    //! The method to save hits into an ASCII file
    virtual void Save(FILE*) {;}

    //! The method to save hits into data store
    virtual void Save(G4int) {}

    //! Load hits from an ASCII file
    /*!
        \return True while the load succeds
    */
    virtual G4bool Load(FILE*) {return false;}

  protected:

  };

  typedef G4THitsCollection<B4VHit> B4VHitsCollection;

} // end of namespace Belle2

#endif /* B4VHIT_H */
