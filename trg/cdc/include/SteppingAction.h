//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : SteppingAction.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to control a track in G4
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCSteppingAction_FLAG_
#define TRGCDCSteppingAction_FLAG_

#include "G4UserSteppingAction.hh"
#include "globals.hh"

#ifdef TRGCDC_SHORT_NAMES
#define TCSAction TRGCDCSteppingAction
#endif

namespace Belle2 {

/// A class to control a track in G4
  class TRGCDCSteppingAction : public G4UserSteppingAction {

  public:
    /// Constructor.
    TRGCDCSteppingAction();

    /// Destructor
    virtual ~TRGCDCSteppingAction();

    /// Stepping action to control a step in G4.
    virtual void UserSteppingAction(const G4Step*);
  };

//-----------------------------------------------------------------------------

} // namespace Belle2

#endif /* TRGCDCSteppingAction_FLAG_ */

