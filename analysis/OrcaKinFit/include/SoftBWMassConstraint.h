/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * Forked from https://github.com/iLCSoft/MarlinKinfit                    *
 *                                                                        *
 * Further information about the fit engine and the user interface        *
 * provided in MarlinKinfit can be found at                               *
 * https://www.desy.de/~blist/kinfit/doc/html/                            *
 * and in the LCNotes LC-TOOL-2009-001 and LC-TOOL-2009-004 available     *
 * from http://www-flc.desy.de/lcnotes/                                   *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifdef MARLIN_USE_ROOT

#ifndef __SOFTBWMASSCONSTRAINT_H
#define __SOFTBWMASSCONSTRAINT_H

#include "analysis/OrcaKinFit/SoftBWParticleConstraint.h"

#include <limits>

namespace Belle2 {
  namespace OrcaKinFit {

    class ParticleFitObject;

//  Class SoftBWMassConstraint:
/// Implements constraint 0 = mass1 - mass2 - m
    /**
     * This class implements different mass constraints:
     * - the invariant mass of several objects should be m
     * - the difference of the invariant masses between two
     *   sets of objects should be m (normally m=0 in this case).
     *
     * Author: Jenny List, Benno List
     * Last update: $Date: 2011/05/03 13:18:28 $
     *          by: $Author: blist $
     *
     */

    class SoftBWMassConstraint : public SoftBWParticleConstraint {
    public:

      /// Constructor
      explicit SoftBWMassConstraint(double gamma_,      ///< The Gamma value
                                    double mass_ = 0.,  ///< The mass difference between object sets 1 and 2
                                    double massmin_ = -std::numeric_limits<double>::infinity(),   ///< lower mass bound
                                    double massmax_ =  std::numeric_limits<double>::infinity()    ///< upper mass bound
                                   );
      /// Virtual destructor
      virtual ~SoftBWMassConstraint();

      /// Returns the value of the constraint function
      virtual double getValue() const override;

      /// Get first order derivatives.
      /// Call this with a predefined array "der" with the necessary number of entries!
      virtual void getDerivatives(int idim,      ///< First dimension of the array
                                  double der[]   ///< Array of derivatives, at least idim x idim
                                 ) const override;

      /// Get the actual invariant mass of the fit objects with a given flag
      virtual double getMass(int flag = 1        ///< The flag
                            );

      /// Sets the target mass of the constraint
      virtual void setMass(double mass_            ///< The new mass
                          );


    protected:
      double mass;      ///< The mass difference between object sets 1 and 2


      /// Second derivatives with respect to the 4-vectors of Fit objects i and j; result false if all derivatives are zero
      virtual bool secondDerivatives(int i,                         ///< number of 1st FitObject
                                     int j,                        ///< number of 2nd FitObject
                                     double* derivatives           ///< The result 4x4 matrix
                                    ) const override;
      /// First derivatives with respect to the 4-vector of Fit objects i; result false if all derivatives are zero
      virtual bool firstDerivatives(int i,                         ///< number of 1st FitObject
                                    double* derivatives           ///< The result 4-vector
                                   ) const override;
    };

  }// end OrcaKinFit namespace
} // end Belle2 namespace


#endif // __SOFTBWMASSCONSTRAINT_H

#endif // MARLIN_USE_ROOT
