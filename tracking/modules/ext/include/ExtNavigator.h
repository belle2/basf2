/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Leo Piilonen                                            *
 * Derived from: G4ErrorPropagationNavigator                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EXTNAVIGATOR_H
#define EXTNAVIGATOR_H

#include <G4Navigator.hh>
#include <G4ThreeVector.hh>

namespace Belle2 {

  /**
   * Perform navigation in the detector geometry and within the "target"
   * surface for error propagation.
   */
  class ExtNavigator : public G4Navigator {
  public:

    //! constructor
    ExtNavigator();

    //! destructor
    ~ExtNavigator();

    //! Check if distance to surface is closer than the proposed step
    //! but only if the extrapolator is mid-propagation
    G4double ComputeStep(const G4ThreeVector& pGlobalPoint,
                         const G4ThreeVector& pDirection,
                         const G4double pCurrentProposedStepLength,
                         G4double& pNewSafety);

    //! Check if distance to surface is smaller than the proposed safety
    //! but only if the extrapolator is mid-propagation
    G4double ComputeSafety(const G4ThreeVector& globalpoint,
                           const G4double pProposedMaxLength = DBL_MAX);

  private:

  };

} // end of namespace Belle2

#endif // EXTNAVIGATOR_H
