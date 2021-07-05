/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to control a track in G4
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

