//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : SteppingAction.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to control a track in G4
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include "G4Step.hh"
#include "trg/cdc/SteppingAction.h"

using namespace std;

namespace Belle2 {

  TRGCDCSteppingAction::TRGCDCSteppingAction()
  {
  }

  TRGCDCSteppingAction::~TRGCDCSteppingAction()
  {
  }

  void
  TRGCDCSteppingAction::UserSteppingAction(const G4Step* aStep)
  {
    const G4StepPoint& in = * aStep->GetPreStepPoint();
    const G4StepPoint& out = * aStep->GetPostStepPoint();
    const double rIn = in.GetPosition().r();
    const double rOut = out.GetPosition().r();
    const bool curlBack = rIn > rOut;
    if (curlBack)
      aStep->GetTrack()->SetTrackStatus(fKillTrackAndSecondaries);
  };

} // namespace Belle2
